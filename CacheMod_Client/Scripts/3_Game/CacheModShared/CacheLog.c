// CacheLog.c
//
// Every message the mod prints goes through here. One prefix per subsystem, so
// a single search in the RPT ("[CacheMod/") shows the whole run, and a narrower
// one ("[CacheMod/Search]") shows just the part being debugged.
//
// Debug output is off until the server config turns it on: the search loop and
// the proxy sweep run often enough that per-tick lines cost more than they
// explain on a populated server.

class CacheLog
{
	static bool s_DebugEnabled = false;

	static const string ROOT = "[CACHE_MOD]";
	static const string PLACEMENT = "[CacheMod/Placement]";
	static const string SEARCH = "[CacheMod/Search]";
	static const string INTERACTION = "[CacheMod/Interaction]";
	static const string LOOT = "[CacheMod/Loot]";
	static const string ZONE = "[CacheMod/Zone]";
	static const string CHEST = "[CacheMod/Chest]";
	static const string RESPAWN = "[CacheMod/Respawn]";
	static const string ADMIN = "[CacheMod/Admin]";

	static void Error(string category, string message)
	{
		Print(category + " ERROR: " + message);
	}

	static void Warning(string category, string message)
	{
		Print(category + " WARN:  " + message);
	}

	static void Info(string category, string message)
	{
		Print(category + " INFO:  " + message);
	}

	static void Debug(string category, string message)
	{
		if (s_DebugEnabled)
			Print(category + " DEBUG: " + message);
	}
}
