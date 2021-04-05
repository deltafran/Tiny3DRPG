#pragma once

namespace Log
{
	enum ELogMessageFlags
	{
		LOG_MSGBOX = 0x0001, // Use a message box.
		LOG_TIME = 0x0002, // Print time (e.g. "[18:30:12] ...").
		LOG_WARNING = 0x0004, // Yellow color like a warning message.
		LOG_ERROR = 0x0008, // Red color like an error message.
		LOG_DEBUG = 0x0010, // Pink color like a debug message.
		LOG_NONEWLINE = 0x0020, // No new line will be printed.
		LOG_NOTAB = 0x0040, // No tab string is printed.
	};

	bool Open(const std::string& Filename);
	void Close();

	void Error(const std::string& Message, int Flags = LOG_TIME);
	void Warning(const std::string& Message, int Flags = LOG_TIME);
	void Debug(const std::string& ProcName, const std::string& Message = "Invalid arguments", int Flags = LOG_TIME);
	void Message(const std::string& Message, int Flags = LOG_TIME);
} // napespace Log