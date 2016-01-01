// ffmpeg -framerate 1 -pattern_type glob -i '*.jpg' -c:v libx264 out.mp4

#include <signal.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include  "Logger.h"
#include <unistd.h>
#include <string>  
#include <sstream>
#include <iomanip>
#include "sys/stat.h"
#include <boost/filesystem.hpp>
#define THREASHOLD_MEAN 40
#define THREASHOLD 5
#define MICRO_SEC_SEC 1000000.
#define BASE_PATH "./"
#define MAX_CAM 5

using namespace std;
using namespace cv;

struct stream {
  VideoCapture * videoCapture;
  bool record;
  long int frame_id;
  std::ofstream *metadata;
  float lastMean;
  long int nbSkeep;
};
int nbCams = 0;
struct stream streams[MAX_CAM];

void signalHandler( int signum )
{
  switch(signum)
  {
    //  kill -10 `cat /var/lock/IRecorderDaemon`
    case SIGUSR1:
    {
        
        break;
    }
    case SIGPIPE:
    {
        break;
    }
    case SIGWINCH:
    {
     break; 
    }
    case SIGSEGV:
    {
        exit(signum);
        break;
    }
    default:
    {
      {
        std::cout << "Ending with signal" << signum << std::endl;
          for(int i=0; i<nbCams; i++)
          {
            streams[i].videoCapture->release();
            streams[i].metadata->close();
            delete streams[i].videoCapture;
            delete streams[i].metadata;
          }
        exit(signum);  
        break;
      }
    }
  }
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<boost::filesystem::path> getImageList(boost::filesystem::path _path, string _ext)
{
        std::vector<boost::filesystem::path> list;
        if(!boost::filesystem::exists(_path) || !boost::filesystem::is_directory(_path)) return list;

        boost::filesystem::recursive_directory_iterator it(_path);
        boost::filesystem::recursive_directory_iterator endit;

        while(it != endit)
        {
            if(boost::filesystem::is_regular_file(*it) && it->path().extension() == _ext)
            {
              list.push_back(it->path());
            }
            ++it;

        }
        std::sort(list.begin(), list.end());

        return list;
}

bool postProcess()
{
      std::ifstream f("cam0.csv", std::ios::in);
      boost::filesystem::path path = "/home/mehdi/Mehdi/perso/newYearMusique";
      std::vector<boost::filesystem::path> list = getImageList(path, ".jpg");
      for (auto it = list.begin(); it != list.end(); ++it)
      {
          Mat img = imread((*it).string().c_str());
          char metadata[255];
          f.getline(metadata, 255);
          
          std::vector<std::string> metadatas = split(metadata, ',');
          cout << metadatas[1] << endl;
          std::string save = (*it).string() + "_";

            putText(img, metadatas[1].c_str(), Point(10,40), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,255), 1);
          imwrite(save, img);

      }
      f.close();
}

void help()
{
    std::cout << "this help" << std::endl;
}

void rotate(cv::Mat& src, double angle, cv::Mat& dst)
{
    int len = std::max(src.cols, src.rows);
    cv::Point2f pt(len/2., len/2.);
    cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);

    cv::warpAffine(src, dst, r, cv::Size(len, len));
}


int main(int argc, char *argv[])
{
  int nbSkeep = 0;
  Logger * logger = new Logger(Logger::DEBUG);
  int fps = 2;
  string name = "";
  bool date = false;
  float rotateAngle = 0;
  char c;
  while ((c = getopt(argc, argv, "f:n:dr:h")) != -1)
  {
    switch (c)
    {
      case 'f':
        fps = std::atoi(optarg);
        break;
      case 'r':
        rotateAngle = std::atof(optarg);
        break;
      case 'n':
        name = optarg;
        break;
      case 'd':
          date = true;
          break;
      case 'h':
        help();
        return 0;
        break;
      default:
        logger->Write(Logger::WARNING,"CONFIG", "Unknown option", "Logs");
        help();
        break;
    }
  }
  
//   return postProcess();
  
  
  bool record[MAX_CAM];
  std::ofstream *metadatas[MAX_CAM];

  for(int i=0; i<MAX_CAM; i++)
  {
   VideoCapture *videoCapture;
    videoCapture = new VideoCapture(i);
    record[i] = false;
    if(videoCapture->isOpened())  // check if we succeeded
    {
        logger->Write(Logger::INFO,"ImageManager", "detected camera #"+to_string(i),"Logs");
        nbCams++;
        streams[i].videoCapture = videoCapture;
        streams[i].lastMean = THREASHOLD_MEAN - THREASHOLD*2;
        streams[i].frame_id = 0;
        streams[i].nbSkeep = 0;
        std::string path = BASE_PATH + name + "/cam" + to_string(i) + ".csv";
        streams[i].metadata = new std::ofstream(path, std::ios::trunc);
        if(!*streams[i].metadata)
        {
          logger->Write(Logger::ERRORLOG,"ImageManager", "Could not open " + path,"Logs");
        }
    }
  }
  
  if(nbCams == 0)
  {
    logger->Write(Logger::ERRORLOG,"ImageManager", "could not detect any camera","Logs");
    exit(-1);
  }
  
//   for(int i = 0; i < 128; i++)
  {
    signal(SIGINT, signalHandler);
  }


  for(;;)
    {
//       usleep((MICRO_SEC_SEC/fps)/nbCams);
      sleep(fps);
      for(int j=0; j<nbCams; j++)
      {
                VideoCapture *videoCapture = streams[j].videoCapture;
                Mat currentImage;

                bool isValid = videoCapture->read(currentImage); // get a new frame from camera
                if(!isValid)
                {
                  logger->Write(Logger::ERRORLOG,"ImageManager", "invalid image "+to_string(j),"Logs");
                    continue;
                }
                
                if(rotateAngle != 0)
                {
                   rotate(currentImage, rotateAngle, currentImage); 
                }
                if(date)
                {
                    std::string dateStr = "";
                    time_t rawtime;
                    struct tm * timeinfo;
                    time (&rawtime);
                    timeinfo = localtime (&rawtime);
                    dateStr = std::to_string(timeinfo->tm_mday) + "/" + std::to_string(timeinfo->tm_mon+1) + "/" + std::to_string(timeinfo->tm_year + 1900) + " " + std::to_string(timeinfo->tm_hour) + ":" + std::to_string(timeinfo->tm_min);
                    putText(currentImage, dateStr, Point(10,40), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,255), 1);
                }
                imshow("frame", currentImage);
                
                float lightMean = mean(mean(currentImage))[0];
                if(streams[j].lastMean + THREASHOLD < lightMean && lightMean > THREASHOLD_MEAN && !streams[j].record)
                {
                  logger->Write(Logger::INFO,"ImageManager", "Start recording camera #"+to_string(j),"Logs");
                  streams[j].record = true;
                }
                if(streams[j].lastMean - THREASHOLD > lightMean && lightMean < THREASHOLD_MEAN && streams[j].record)
                {
                  logger->Write(Logger::INFO,"ImageManager", "Stop recording camera #"+to_string(j),"Logs");
                  streams[j].record = false;
                }
                
                streams[j].lastMean = lightMean;
            //      if(lightMean > THREASHOLD_MEAN)
                if(streams[j].record)
                {
                  std::ostringstream ss;
                  std::string path = BASE_PATH + name + "/";
                  mkdir(path.c_str(),0700);
                  ss << path <<"img_" << to_string(j) << "_" << std::setw(5) << std::setfill('0') << streams[j].frame_id << ".jpg";
                  std::string name =  ss.str();
                  if(streams[j].frame_id%30 == 0)
                  {
                    logger->Write(Logger::INFO,"ImageManager", "saving " + name + " mean = "+to_string(lightMean),"Logs");
                  }
                  imwrite(name.c_str(), currentImage);
                  if (streams[j].metadata)
                  {
                     std::time_t result = std::time(nullptr);
                    (*streams[j].metadata) << streams[j].frame_id << "," << std::asctime(std::localtime(&result)) ;
                  }
                  streams[j].frame_id++;
                  waitKey(0); 
                }
                else
                {
                  streams[j].nbSkeep++;
                  if(streams[j].nbSkeep%30 == 0)
                  {
                    logger->Write(Logger::DEBUG,"ImageManager", to_string(streams[j].nbSkeep)+" images skipped (mean = "+to_string(lightMean)+")","Logs");
                  }
                }
      }

    }
  
  return 0;
}
