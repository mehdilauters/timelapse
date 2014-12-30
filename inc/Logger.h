/*
 * Logger.h
 *
 *  Created on: 7 janv. 2012
 *      Author: rox
 */
//#pragma once
#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <cstring>
#include <stdlib.h>
#include <stdlib.h>
#include <vector>



class LoggerObserver;




class Logger
{

public:

	/**
	 * Levels for Debugging
	 */
  typedef  enum Priority
    {
        DEBUG =0,
        CONFIG,
        INFO,
        WARNING,
        ERRORLOG,
        E_NBR_PRIORITY
    } PRIORITY;

    static const std::string PRIORITY_NAMES[];

    /**
      *Constructor of the Logger class
      *
      * @param da : DatabaseAccess object
      * @param minPriority : Priority object -  represents the minimal Debug level
      * @return
      */


    /**
     *
     * @param pole : tModule object for obtaining the name of the pole
     * @param minPriority : Priority object -  represents the minimal Debug level
     */
    Logger(Priority minPriority = Logger::DEBUG);
    /**
     * Destructor of the Logger class
     *
     * @return
     */
    ~Logger();

    /**
     * Writes the message in the Log file and on the screen;
     * If there is a connection toward the DataBase, the message will be written as well in the DataBase
     *
     * @param priority : Priority object -  represents the minimal Debug level
     * @param message : String object - the message sent
     * @param marker : String object - checkpoint in the code
     */
    void Write(Priority priority, std::string moduleName, std::string message, std::string marker);
	
	void WriteFile(Priority priority, std::string moduleName, std::string marker, int dataSize, char *_data);

	void addObserver(LoggerObserver *_observer);
	

    /** TimeStamp for the message to be written in the Log file
    *
    * @return
    */
    std::string TimeStampMessage();

    /**
     * TimeStamp for the Log file
     *
     * @return
     */
    std::string TimeStampFile();

    /**
     * convert priority string to enum
     * @param _da :  string _priority
     * @return enum
     */
    PRIORITY stringPriorityToEnum(std::string _priority );


    void setReportingLevel(Priority _reportingLevel);
	


private:

    /**
     * Private members
     */
    bool        m_active;
    std::ofstream    m_fileStream;
    Priority    m_minPriority;
   std::string _reportingLevel;

    std::string	m_poleName;
    std::string  m_marker;
	
	std::vector<LoggerObserver *> m_observers;
	


};



Logger::PRIORITY stringPriorityToEnum(std::string _priority );

std::string priorityEnumToString( Logger::PRIORITY  _priority );


class LoggerObserver
{
    public:
        virtual ~LoggerObserver() {}
		virtual void log(Logger::Priority priority, std::string moduleName, std::string message, std::string marker) = 0;
};

#endif /* LOGGER_H_ */
