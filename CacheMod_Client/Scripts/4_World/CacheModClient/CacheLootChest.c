// CacheLootChest.c
//
// What a finished search leaves behind: a real, visible container holding the
// loot the server generated for it.
//
// It has nothing to do with the proxy. The proxy is the hidden point that made
// the search possible and is gone by the time this exists; the chest is an
// ordinary container that happens to be temporary. Its lifetime is enforced by
// the server's chest service, not by the entity itself, so a chest whose owning
// cache was deleted mid-life still disappears on schedule.

class CacheLootChest extends Container_Base
{
	// The cache this chest came out of, so the server can clear the reference
	// when the chest goes away. Server-side only, like the proxy's id.
	protected string m_CacheId;

	void CacheModSetCacheId(string cacheId)
	{
		if (!GetGame().IsServer())
			return;

		m_CacheId = cacheId;
	}

	string CacheModGetCacheId()
	{
		return m_CacheId;
	}

	// A chest is spawned for one search and removed on a timer. It is created
	// with the non-persistent spawn flags and given no storage category in the
	// config, so a restart cannot leave one standing with no service owning it
	// and no timer left to clear it.
}
