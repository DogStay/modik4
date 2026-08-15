// CacheModRPC.c
//
// The wire contract between client and server, plus the few tables both sides
// must agree on. Everything here compiles in 3_Game, so it may not name
// PlayerBase, ItemBase or anything else the World module introduces.

class CacheModRPC
{
	// Chosen high enough to stay clear of vanilla ids and of JobsMod's window.
	static const int BASE = 24700;

	// Both OnRPC handlers use this window to ignore traffic that is not ours.
	static const int ID_RANGE = 100;

	// --- Client -> Server ---
	// An administrator confirmed a hologram position and wants a cache written
	// there. Payload: Param2<vector, float> position + radius.
	//
	// Starting a search has no message of its own: ActionSearchCache already
	// runs its completion half on the server, so the request travels through
	// the engine's own action pipeline instead of a hand-rolled RPC.
	static const int REQUEST_CREATE_CACHE = BASE + 1;
	// Admin visualisation asked for the current picture of every cache.
	static const int REQUEST_ADMIN_POINTS = BASE + 2;

	// --- Server -> Client ---
	// Free-text notification shown to the player (search refused, cache placed).
	static const int NOTIFY_MESSAGE = BASE + 50;
	// The admin sphere payload: one packed string, see CacheModAdminPoint.
	static const int NOTIFY_ADMIN_POINTS = BASE + 51;
	// The few tuning values a client has to know to build its own action
	// components. Payload: Param1<float> search duration in seconds.
	static const int NOTIFY_SETTINGS = BASE + 52;

	// Separator inside packed multi-value payloads. Safe because no cache id
	// may contain one — the config enforces that when it validates ids.
	static const string FIELD_SEPARATOR = ";";
	static const string RECORD_SEPARATOR = "|";

	// Hard ceiling on how far a placement request may land from the admin who
	// sent it. The client picks the spot with a hologram, which the engine
	// already keeps within arm's reach; this only bounds a forged request.
	static const float MAX_PLACEMENT_DISTANCE = 30.0;

	// Bounds for the single radius a cache has. Anything smaller cannot be
	// aimed at reliably, anything larger stops feeling like a hidden spot.
	static const float MIN_CACHE_RADIUS = 0.5;
	static const float MAX_CACHE_RADIUS = 15.0;
}

// What a cache demands of the player who found it.
class CacheModType
{
	static const int BASIC = 0;
	static const int TOOL_REQUIRED = 1;

	static const string TEXT_BASIC = "BASIC";
	static const string TEXT_TOOL_REQUIRED = "TOOL_REQUIRED";

	static int FromText(string text)
	{
		if (text == TEXT_TOOL_REQUIRED)
			return TOOL_REQUIRED;

		return BASIC;
	}

	static string ToText(int type)
	{
		if (type == TOOL_REQUIRED)
			return TEXT_TOOL_REQUIRED;

		return TEXT_BASIC;
	}
}

// Runtime state of one cache, as far as anyone outside the manager needs it.
class CacheModState
{
	// Rolled its spawn chance this restart and is waiting to be found.
	static const int ACTIVE = 0;
	// Saved, but its spawn chance did not roll this restart. No proxy exists,
	// and an ordinary player has no way to reach it until the next restart.
	static const int INACTIVE = 1;
	// Found and emptied; waiting out its respawn timer.
	static const int COOLDOWN = 2;
}

// The tools a TOOL_REQUIRED cache may ask for.
//
// Caches carry a tool as an index into this table rather than as a classname,
// because the index is what the interaction proxy synchronises to the client:
// the action has to know whether to appear before the server is consulted, and
// entity synchronisation carries numbers, not strings.
//
// Appending to the end is safe. Reordering or removing an entry changes what
// every saved cache means and must be treated as a breaking change.
class CacheModTools
{
	static const int NONE = 0;

	protected static ref array<string> s_Classes;

	protected static void Build()
	{
		if (s_Classes)
			return;

		s_Classes = new array<string>();
		// Index 0 is the empty slot: a BASIC cache needs nothing.
		s_Classes.Insert("");
		s_Classes.Insert("Shovel");
		s_Classes.Insert("FieldShovel");
		s_Classes.Insert("Crowbar");
		s_Classes.Insert("Screwdriver");
		s_Classes.Insert("Pickaxe");
		s_Classes.Insert("Hatchet");
		s_Classes.Insert("Pliers");
		s_Classes.Insert("LockPick");
	}

	static int GetCount()
	{
		Build();
		return s_Classes.Count();
	}

	static string GetClassName(int index)
	{
		Build();

		if (index < 0)
			return "";

		if (index >= s_Classes.Count())
			return "";

		return s_Classes.Get(index);
	}

	// Returns NONE for an unknown name, which is how a mistyped required_tool
	// degrades into "no tool needed" instead of into an unsearchable cache.
	static int GetIndex(string className)
	{
		Build();

		if (className == "")
			return NONE;

		string wanted = className;
		wanted.ToLower();

		for (int i = 0; i < s_Classes.Count(); i++)
		{
			string candidate = s_Classes.Get(i);
			candidate.ToLower();

			if (candidate == wanted)
				return i;
		}

		return NONE;
	}
}

// Why a search or a placement was refused. The number travels; the client turns
// it into text, so the wording can change without touching the protocol.
class CacheModReject
{
	static const int UNKNOWN = 0;
	static const int NOT_ADMIN = 1;
	static const int CACHE_GONE = 2;
	static const int ALREADY_SEARCHED = 3;
	static const int TOO_FAR = 4;
	static const int TOOL_MISSING = 5;
	static const int CHEST_PRESENT = 6;
	static const int SAVE_FAILED = 7;

	static string GetText(int reason)
	{
		switch (reason)
		{
			case NOT_ADMIN:
				return "У вас нет прав на размещение тайников.";
			case CACHE_GONE:
				return "Тайник больше не существует.";
			case ALREADY_SEARCHED:
				return "Здесь уже обыскали.";
			case TOO_FAR:
				return "Вы отошли от тайника.";
			case TOOL_MISSING:
				return "Нужен подходящий инструмент.";
			case CHEST_PRESENT:
				return "Содержимое тайника уже вскрыто.";
			case SAVE_FAILED:
				return "Не удалось сохранить тайник на сервере.";
		}

		return "Запрос отклонён.";
	}
}
