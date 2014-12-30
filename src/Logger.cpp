// #include "boost/date_time/local_time/local_time.hpp"
#include "Logger.h"
using namespace std;


const string Logger::PRIORITY_NAMES[] = {
		 "DEBUG",
		"CONFIG",
		"INFO",
		"WARNING",
		"ERRORLOG"
};

Logger::PRIORITY  stringPriorityToEnum( string _priority ){

	Logger::PRIORITY result = Logger::DEBUG;
	if( _priority == Logger::PRIORITY_NAMES[ Logger::DEBUG ] ){
		result = Logger::DEBUG;
	}
	if( _priority == Logger::PRIORITY_NAMES[ Logger::CONFIG ] ){
		result = Logger::CONFIG;
	}
	if( _priority == Logger::PRIORITY_NAMES[ Logger::INFO ] ){
		result = Logger::INFO;
	}
	if( _priority == Logger::PRIORITY_NAMES[ Logger::WARNING ] ){
		result = Logger::WARNING;
	}
	if( _priority == Logger::PRIORITY_NAMES[ Logger::ERRORLOG ] ){
		result = Logger::ERRORLOG;
	}

	return result;
}


std::string priorityEnumToString( Logger::PRIORITY _priority ){

    string result = Logger::PRIORITY_NAMES[ Logger::DEBUG ];

	if( _priority == Logger::DEBUG ){
		result = Logger::PRIORITY_NAMES[ Logger::DEBUG ];
	}
	if( _priority == Logger::CONFIG ){
		result = Logger::PRIORITY_NAMES[ Logger::CONFIG ];
	}
	if( _priority ==  Logger::INFO ){
		result = Logger::PRIORITY_NAMES[ Logger::INFO ];
	}
	if( _priority == Logger::WARNING  ){
		result = Logger::PRIORITY_NAMES[ Logger::WARNING ];
	}
	if( _priority == Logger::ERRORLOG ){
		result = Logger::PRIORITY_NAMES[ Logger::ERRORLOG ];
	}

	return result;
}

/**
 *
 * Function implementations
 *
 */


Logger::Logger(Priority minPriority)
{
	this->m_active = false;

	this->m_active = true;
	this->m_minPriority = minPriority;

	 string logFile = "_test_";
     string extension = ".log";

	 if (logFile != "")
	    {
	    	string fileName = this->m_poleName + logFile + TimeStampFile() + extension;
	    	this->m_fileStream.open(fileName.c_str());
	    }
}


Logger:: ~Logger()
{
	m_active = false;
	    if (m_fileStream.is_open())
	    {
	        m_fileStream.close();
	    }
}




void Logger::setReportingLevel(Priority _reportingLevel)
{
	this->m_minPriority=_reportingLevel;
}

string Logger::TimeStampMessage()
{
	  time_t tim=time(NULL);
	  tm *now=localtime(&tim);
	  #warning no timestamp
	  char buffer [40];
	  buffer[0] = '\0';
//        sprintf(buffer, "%d_%02d_%02dT%02d:%02d:%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

	  return string(buffer);
}

string Logger::TimeStampFile()
{
	  time_t tim=time(NULL);
	  tm *now=localtime(&tim);

	  char buffer [40];

	   sprintf(buffer, "%d_%02d_%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday);

	  return string(buffer);
}


void Logger::Write(Priority priority, std::string moduleName, string message, string marker)
{

	//write all logs (with all priorities) in the Log File
    if (m_active)
    {
        // identify current output stream
        ostream& stream
            = m_fileStream.is_open() ?m_fileStream : std::cout;

		//ecrire dans le fichier
        stream  << PRIORITY_NAMES[priority]
                << ": "
                << TimeStampMessage()
                << " : "
                << marker
                <<" - "
                << message
                << endl;

        //ecrire sur l'écran
        if (m_active && priority >= m_minPriority)
        {
			#ifndef WIN32
        		std::cout << "\e[" << (31 + priority % 7) << "m";
			#endif
            std::cout   	<< moduleName << " => " <<PRIORITY_NAMES[priority]
        	                          << ": "
        	                          << TimeStampMessage()
        	                          << " : "
        	                          << marker
        	                          <<" - "
        	                          << message;
			#ifndef WIN32
        		std::cout<< "\e[0m"<<endl;
			#endif

            for(std::vector<LoggerObserver *>::iterator it = this->m_observers.begin(); it != this->m_observers.end(); ++it) {
                (*it)->log(priority, moduleName, message, marker);
            }
        }
    }
}



void Logger::WriteFile(Priority priority, std::string moduleName, std::string marker, int dataSize, char *_data)
{
    if (m_active && priority >= m_minPriority)
    {
        this->Write(priority, moduleName, "saveFile", marker);
        std::stringstream stream;
        stream  << PRIORITY_NAMES[priority]
                << "_"
                << TimeStampMessage()
                << "_"
                << moduleName
                <<"_"
                << marker;

        ofstream myfile;
        myfile.open (stream.str().c_str());
        myfile.write(_data, dataSize);
        myfile.close();
    }

}


void Logger::addObserver(LoggerObserver *_observer)
{
    this->m_observers.push_back(_observer);
}


