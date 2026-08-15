// CacheInteractionProxy.c
//
// The invisible thing a player aims at.
//
// A cache is a line in a JSON file; the DayZ action system needs an entity. The
// proxy is that entity and nothing else: it holds no loot, cannot be picked up,
// damaged, moved or stored, and carries no authority — the manager on the
// server owns the state, the proxy only says which cache it belongs to.
//
// Invisibility is done with SetInvisible() rather than with an empty model,
// because the cursor raycast that finds action targets uses the object's
// geometry, which stays where it is when the visual is hidden. An entity with
// no model has nothing to hit and would never become an ActionTarget.
//
// One proxy exists per ACTIVE, available cache. It is deleted the moment the
// cache goes on cooldown and created again when it respawns, which is why the
// search action needs no "is it still there" question of its own: if the player
// can aim at a proxy, the cache behind it was available when it was made.

class CacheInteractionProxy extends ItemBase
{
	// Which cache this proxy stands for. Server-side only — the client is never
	// told, because knowing the id would let a client enumerate caches it has
	// not found. The action sends the proxy itself back to the server, and the
	// server looks the id up from its own map.
	protected string m_CacheId;

	// What the player must be holding, as an index into CacheModTools. This one
	// *is* synchronised: the action has to decide whether to appear at all,
	// before any server round trip, and the alternative — showing the action to
	// everyone and refusing it seven seconds later — turns a hidden-cache mod
	// into a guessing game.
	protected int m_RequiredToolIndex;

	// Whether the tool has to be in the player's hands rather than merely
	// somewhere in their inventory. Also a server-set, synchronised setting.
	protected bool m_ToolInHandsOnly;

	void CacheInteractionProxy()
	{
		RegisterNetSyncVariableInt("m_RequiredToolIndex", 0, 64);
		RegisterNetSyncVariableBool("m_ToolInHandsOnly");
	}

	// =====================================================================
	// Server-side setup
	// =====================================================================
	void CacheModSetup(string cacheId, int requiredToolIndex, bool toolInHandsOnly)
	{
		if (!GetGame().IsServer())
			return;

		m_CacheId = cacheId;
		m_RequiredToolIndex = requiredToolIndex;
		m_ToolInHandsOnly = toolInHandsOnly;
		SetSynchDirty();
	}

	string CacheModGetCacheId()
	{
		return m_CacheId;
	}

	int CacheModGetRequiredToolIndex()
	{
		return m_RequiredToolIndex;
	}

	bool CacheModIsToolInHandsOnly()
	{
		return m_ToolInHandsOnly;
	}

	// =====================================================================
	// Being invisible
	// =====================================================================
	override void EEInit()
	{
		super.EEInit();

		// Applied on both sides. On the server it stops the object showing up
		// in anything that walks visible entities; on the client it is what
		// actually hides it from the player.
		SetInvisible(true);
	}

	// Synchronisation is also the moment a client first learns about the proxy,
	// so the hide is repeated here: an entity that streams in after EEInit has
	// already run on that client would otherwise appear as a crate.
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		SetInvisible(true);
	}

	// =====================================================================
	// Refusing everything a normal item allows
	// =====================================================================
	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		return false;
	}

	override bool IsTakeable()
	{
		return false;
	}

	override bool CanBeDamaged()
	{
		return false;
	}

	override bool IsHealthVisible()
	{
		return false;
	}

	// Nothing about a proxy survives a restart on purpose: the manager rebuilds
	// the whole set from caches.json after rolling spawn chances, and a proxy
	// restored from storage would belong to a cache that may not be active this
	// run — with no way for the manager to tell it apart from one it made.
	//
	// Two things enforce that together: the manager creates proxies with the
	// non-persistent spawn flags, and the config gives the class no storage
	// category, so the hive has nothing to write even if a flag were wrong.
	override bool CanBeCombined(EntityAI other_item, bool reservation_check = true, bool stack_max_limit = false)
	{
		return false;
	}
}
