#include <signal.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include  "Logger.h"
#include <unistd.h>
#include <string>  
#include <sstream>
#include <iomanip>
#include "sys/stat.h"
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

int main(int argc, char *argv[])
{
  int nbSkeep = 0;
  Logger * logger = new Logger(Logger::DEBUG);
  int fps = 0;
  string name = "";
  if(argc != 3)
  {
    logger->Write(Logger::ERRORLOG,"ImageManager", "not enought argv","Logs");
    logger->Write(Logger::INFO,"ImageManager", string(argv[0]) + " name fps","Logs");
    exit(0);
  }
  else
  {
    name = string(argv[1]);
    fps = atof(argv[2]);
  }
  
  
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
      usleep((MICRO_SEC_SEC/fps)/nbCams);
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
            //       waitKey(0); 
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
