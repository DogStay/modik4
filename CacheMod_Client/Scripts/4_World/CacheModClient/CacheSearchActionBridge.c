// CacheSearchActionBridge.c
//
// The seam between the two gameplay PBOs.
//
// The action lives in the client PBO because every player needs it. What
// answers it — the cache manager, the loot tables, the chest — lives in the
// server PBO, which clients never download, so the action may not name a single
// server type or clients would fail to compile. A ScriptInvoker inverts the
// dependency: the action announces "this player finished searching this proxy",
// and the server mod subscribes to it during bootstrap.
//
// On a server without CacheMod_Server the invoker has no subscribers and the
// request is dropped, which is the right outcome for a client that joined a
// server not running the server half.

class CacheSearchActionBridge
{
	protected static ref ScriptInvoker s_OnSearchFinished;

	static ScriptInvoker GetOnSearchFinished()
	{
		if (!s_OnSearchFinished)
			s_OnSearchFinished = new ScriptInvoker();

		return s_OnSearchFinished;
	}

	// Raised from the server half of ActionSearchCache only. Guarded here as
	// well as at the call site, so a future caller cannot raise it on a client
	// where no subscriber could legitimately act on it.
	static void ReportSearchFinished(PlayerBase player, CacheInteractionProxy proxy)
	{
		if (!GetGame().IsServer())
			return;

		if (!player || !proxy)
			return;

		GetOnSearchFinished().Invoke(player, proxy);
	}
}

// The same inversion for placement. The admin's book raises this on the server
// after the position has been validated; the server mod turns it into a cache.
class CachePlacementBridge
{
	protected static ref ScriptInvoker s_OnPlacementRequested;

	static ScriptInvoker GetOnPlacementRequested()
	{
		if (!s_OnPlacementRequested)
			s_OnPlacementRequested = new ScriptInvoker();

		return s_OnPlacementRequested;
	}

	// player, position, radius, cache type. Everything else about the new cache
	// comes from the server's own defaults — the client has no say in it.
	static void RequestPlacement(PlayerBase player, vector position, float radius, int cacheType)
	{
		if (!GetGame().IsServer())
			return;

		if (!player)
			return;

		GetOnPlacementRequested().Invoke(player, position, radius, cacheType);
	}
}
