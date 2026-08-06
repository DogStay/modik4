// JobsLog.c
//
// Every mod message goes through here so all of them share one prefix and can
// be found in the RPT with a single search. Debug output is off by default and
// is switched on by the server config, because per-frame debug lines on a full
// server cost more than they explain.

class JobsLog
{
	static bool s_DebugEnabled = false;

	protected static const string PREFIX = "[JobsMod] ";

	static void Error(string message)
	{
		Print(PREFIX + "ERROR: " + message);
	}

	static void Warning(string message)
	{
		Print(PREFIX + "WARN:  " + message);
	}

	static void Info(string message)
	{
		Print(PREFIX + "INFO:  " + message);
	}

	static void Debug(string message)
	{
		if (s_DebugEnabled)
			Print(PREFIX + "DEBUG: " + message);
	}
}
