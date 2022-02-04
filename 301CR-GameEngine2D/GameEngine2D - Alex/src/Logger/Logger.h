#ifndef LOGGER_H
#define LOGGER_h

#include <vector>
#include <string>

enum LogType {
	Log_Info,
	Log_Warning,
	Log_Error
};

struct LogEntry {
	LogType type;
	std::string message;
};

class Logger
{
public:
	static std::vector<LogEntry> messages;
	static void Log(const std::string& message);
	static void Error(const std::string& message);
};

#endif // ! LOGGER_H 