
#include <opencv2/opencv.hpp>
#include <iostream>
#include  "Logger.h"
#include <unistd.h>
#include <string>  
#include <sstream>
#include <iomanip>
#include "sys/stat.h"
#define THREASHOLD_MEAN 60
#define THREASHOLD 5
#define MICRO_SEC_SEC 1000000.
#define BASE_PATH "./"
#define MAX_CAM 5

using namespace std;
using namespace cv;


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
    fps = atoi(argv[2]);
  }
  logger->Write(Logger::ERRORLOG,"ImageManager", "cannot open video","Logs");
  VideoCapture *videoCaptures[MAX_CAM];
  bool record[MAX_CAM];
  int nbCam = 0;
  for(int i=0; i<MAX_CAM; i++)
  {
   videoCaptures[i] = NULL; 
   VideoCapture *videoCapture;
    videoCapture = new VideoCapture(i);
    record[i] = false;
    if(videoCapture->isOpened())  // check if we succeeded
    {
        logger->Write(Logger::INFO,"ImageManager", "detected camera #"+to_string(i),"Logs");
        videoCaptures[i] = videoCapture; 
        nbCam++;
    }
  }
  
  if(nbCam == 0)
  {
    logger->Write(Logger::ERRORLOG,"ImageManager", "could not detect any camera","Logs");
    exit(-1);
  }
  
  
  float lastMean = THREASHOLD_MEAN - THREASHOLD*2;
  int i = 0;
  for(;;)
    {
      usleep(MICRO_SEC_SEC/fps);
      for(int j=0; j<MAX_CAM; j++)
      {
        if(videoCaptures[j] == NULL )
        {
            continue;
        }
                VideoCapture *videoCapture = videoCaptures[j];
                Mat currentImage;

                bool isValid = videoCapture->read(currentImage); // get a new frame from camera
                if(!isValid)
                {
                  logger->Write(Logger::ERRORLOG,"ImageManager", "invalid image","Logs");
                    continue;
                }
                imshow("frame", currentImage);
                
                float lightMean = mean(mean(currentImage))[0];
                if(lastMean + THREASHOLD < lightMean && lightMean > THREASHOLD_MEAN)
                {
                  logger->Write(Logger::INFO,"ImageManager", "Start recording camera #"+to_string(j),"Logs");
                  record[j] = true;
                }
                if(lastMean - THREASHOLD > lightMean && lightMean < THREASHOLD_MEAN)
                {
                  logger->Write(Logger::INFO,"ImageManager", "Stop recording camera #"+to_string(j),"Logs");
                  record[j] = false;
                }
                
                lastMean = lightMean;
            //      if(lightMean > THREASHOLD_MEAN)
                if(record[j])
                {
                  std::ostringstream ss;
                  std::string path = BASE_PATH + name + "/";
                  mkdir(path.c_str(),0700);
                  ss << path <<"img_" << to_string(j) << "_" << std::setw(5) << std::setfill('0') << i << ".jpg";
                  std::string name =  ss.str();
                  if(i%30 == 0)
                  {
                    logger->Write(Logger::INFO,"ImageManager", "saving " + name + "mean = "+to_string(lightMean),"Logs");
                  }
                  imwrite(name.c_str(), currentImage);
                  i++;
            //       waitKey(0); 
                }
                else
                {
                  nbSkeep++;
                  if(nbSkeep%30 == 0)
                  {
                    logger->Write(Logger::DEBUG,"ImageManager", to_string(nbSkeep)+" images skipped (mean = "+to_string(lightMean)+")","Logs");
                  }
                }
      }

    }
  
  return 0;
}
