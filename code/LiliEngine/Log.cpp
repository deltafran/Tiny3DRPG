#include "stdafx.h"
#include "Log.h"
#include "Timer.h"
#include "StringUtils.h"

struct LogState
{
	std::ofstream file;
	std::string fileName;
	bool isFile = false;
	bool IsPaused = false;
};

static LogState logState;

void pause(bool isPaused)
{
	if (logState.isFile && logState.IsPaused != isPaused)
	{
		logState.IsPaused = isPaused;

		if (isPaused)
			logState.file.close();
		else
			logState.file.open(logState.fileName.c_str(), std::ios::app);
	}
}

std::string getFormatedTime()
{
	std::string Time;

	Time = (
		Str::Number(Timer::GetTime(TIME_HOUR), 2) + ":" +
		Str::Number(Timer::GetTime(TIME_MINUTE), 2) + ":" +
		Str::Number(Timer::GetTime(TIME_SECOND), 2)
		);

	return "[" + Time + "] ";
}

namespace Log
{
	bool Open(const std::string& Filename)
	{
		logState.file.open(Filename.c_str(), std::ios::trunc);
		if (logState.file.fail())
			return false;

		logState.file << "Lili Engine = debug log file:\n";
		logState.file << ("(generated at " + Timer::GetTime() + ")\n").c_str();
		logState.file << "==================================\n\n";

		logState.isFile = true;
		logState.fileName = Filename;

		return true;
	}
	void Close()
	{
		if (!logState.isFile)
			return;

		logState.file << "\n==================================\n";
		logState.file << "Log END.";

		logState.file.close();

		logState.isFile = false;
		logState.fileName = "";
	}
	void Error(const std::string& message, int Flags)
	{
		Message("Error: " + message + "!", Flags | LOG_ERROR);
	}
	void Warning(const std::string& message, int Flags)
	{
		Message("Warning: " + message + "!", Flags | LOG_WARNING);
	}
	void Debug(const std::string& ProcName, const std::string& message, int Flags)
	{
		if (ProcName.size())
			Message("Debug [ " + ProcName + " ]: " + message + "!", Flags | LOG_DEBUG);
		else
			Message("Debug: " + message + "!", Flags | LOG_DEBUG);
	}
	void Message(const std::string& message, int Flags)
	{
		if (Flags & LOG_MSGBOX)
		{
			/* Open Win32 message box */
			if (Flags & LOG_ERROR)
				MessageBoxA(0, message.c_str(), "Error", MB_OK | MB_ICONERROR);
			else if ((Flags & LOG_WARNING) || (Flags & LOG_DEBUG))
				MessageBoxA(0, message.c_str(), "Warning", MB_OK | MB_ICONWARNING);
			else
				MessageBoxA(0, message.c_str(), "Information", MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			/* Extend the message string */
			std::string FinalMessage, TimePart;

			TimePart = getFormatedTime();

			FinalMessage += message;

			if (!(Flags & LOG_NONEWLINE))
				FinalMessage += "\n";

			/* Print the message to the console output */
			{
				std::cout << TimePart;

				static HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

				if (Flags & LOG_ERROR)
					SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
				else if (Flags & LOG_WARNING)
					SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				else if (Flags & LOG_DEBUG)
					SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

				std::cout << FinalMessage;

				SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}

			/* Print the message to the log file */
			if (logState.isFile)
			{
				pause(false);
				logState.file << (TimePart + FinalMessage);
				pause(true);
			}
		}
	}
} // napespace Log