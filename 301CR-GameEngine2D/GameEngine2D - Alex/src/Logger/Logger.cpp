#include "Logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

std::vector<LogEntry> Logger::messages;

std::string CurrentTimetoString()
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string output(30, '\0');

	struct tm newtime;
	localtime_s(&newtime, &now);
	std::strftime(&output[0], output.size(), "%d-%b-%Y %H:%M:%S", &newtime);
	return output;
}

void Logger::Log(const std::string& message)
{
	LogEntry logEntry;
	logEntry.type = Log_Info;
	logEntry.message = "LOG: [" + CurrentTimetoString() + "]: " + message;

	std::cout << logEntry.message << std::endl;
	messages.push_back(logEntry);
}

void Logger::Error(const std::string& message)
{
	LogEntry logEntry;
	logEntry.type = Log_Error;
	logEntry.message = "ERROR: [" + CurrentTimetoString() + "]: " + message;

	messages.push_back(logEntry);
	std::cerr << logEntry.message << std::endl;
}
