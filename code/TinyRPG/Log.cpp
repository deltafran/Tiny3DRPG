#include "stdafx.h"
#include "Log.h"
#include "Timer.h"

struct LogState
{
	std::ofstream file;
	std::string fileName;
	bool isFile = false;
};

static LogState logState;

namespace Log
{
	bool Open(const std::string& Filename)
	{
		logState.file.open(Filename.c_str(), std::ios::trunc);
		if (logState.file.fail())
			return false;

		logState.file << "Pixie Engine = debug log file:\n";
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
        else if (LogState.Context != LOGCONTEXT_NONE)
        {
            /* Extend the message string */
            stringc FinalMessage, TimePart;

            if (LogState.TimeFormat != LOGTIME_DISABLE && (Flags & LOG_TIME))
                TimePart = getFormatedTime();
            if (!(Flags & LOG_NOTAB))
                FinalMessage += LogState.Tab;

            FinalMessage += Message;

            if (!(Flags & LOG_NONEWLINE))
                FinalMessage += "\n";

            /* Print the message to the console output */
            if (LogState.Context & LOGCONTEXT_CONSOLE)
            {
                std::cout << TimePart.str();

#if defined(SP_PLATFORM_WINDOWS)
                static HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

                if (Flags & LOG_ERROR)
                    SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
                else if (Flags & LOG_WARNING)
                    SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                else if (Flags & LOG_DEBUG)
                    SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif

                std::cout << FinalMessage.str();

#if defined(SP_PLATFORM_WINDOWS)
                SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
            }

            /* Print the message to the log file */
            if (LogState.IsFile && (LogState.Context & LOGCONTEXT_FILE))
            {
                pause(false);
                LogState.File << (TimePart + FinalMessage).str();
                pause(true);
            }

            if (LogState.MsgCallback)
                LogState.MsgCallback(FinalMessage, Flags);
        }
	}
} // napespace Log