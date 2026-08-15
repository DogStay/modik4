// CacheModChestService.c
//
// Owns the visible chests: one per completed search, each with a deadline.
//
// The chest is the only thing about a cache a player ever sees. It is a real
// container with real cargo, and it is temporary — a server that left them
// standing would, after a week, be a server with a chest at every cache point
// and no reason for anyone to search again.
//
// The service, not the entity, holds the deadline. That way a chest whose cache
// was deleted from the admin menu still disappears on schedule, and a restart
// (which takes every non-persistent chest with it) leaves nothing behind.

class CacheModChestService
{
	protected ref CacheModConfig m_Config;
	protected ref array<ref CacheModChestRecord> m_Chests;

	void CacheModChestService(CacheModConfig config)
	{
		m_Config = config;
		m_Chests = new array<ref CacheModChestRecord>();
	}

	bool HasChest(string cacheId)
	{
		return FindRecord(cacheId) != null;
	}

	EntityAI SpawnChest(string cacheId, vector cachePosition)
	{
		string chestClass = m_Config.GetChestClass();

		vector position = cachePosition;
		position[1] = position[1] + m_Config.GetChestHeightOffset();

		// Placed on the surface under the cache position, and networked like any
		// other container: the whole point of the chest is that players see it.
		// It stays out of persistence the same way the proxy does — no
		// types.xml entry — and the record below is what removes it.
		int flags = ECE_PLACE_ON_SURFACE;
		Object created = GetGame().CreateObjectEx(chestClass, position, flags);

		EntityAI chest = EntityAI.Cast(created);
		if (!chest)
		{
			CacheLog.Error(CacheLog.CHEST, "Класс сундука '" + chestClass + "' не создан.");

			if (created)
				GetGame().ObjectDelete(created);

			return null;
		}

		CacheLootChest typed = CacheLootChest.Cast(chest);
		if (typed)
			typed.CacheModSetCacheId(cacheId);

		CacheModChestRecord record = new CacheModChestRecord();
		record.m_CacheId = cacheId;
		record.m_Chest = chest;
		record.m_ExpiresUtc = CacheModClock.GetUtcSeconds() + m_Config.GetChestLifetimeSeconds();
		m_Chests.Insert(record);

		string spawnLine = "Сундук для " + cacheId + " создан в " + position.ToString();
		spawnLine = spawnLine + ", исчезнет в " + record.m_ExpiresUtc.ToString() + " UTC.";
		CacheLog.Info(CacheLog.CHEST, spawnLine);

		return chest;
	}

	// Runs on the server's slow tick. Two ways a chest ends: its time ran out,
	// or it was emptied and the config says an empty chest is done.
	void Update()
	{
		int now = CacheModClock.GetUtcSeconds();
		bool despawnWhenEmpty = m_Config.IsChestDespawnWhenEmpty();

		for (int i = m_Chests.Count() - 1; i >= 0; i--)
		{
			CacheModChestRecord record = m_Chests.Get(i);

			// The entity may already be gone — destroyed, or removed by another
			// mod. The record goes with it rather than being counted as a chest
			// that is still standing.
			if (!record || !record.m_Chest)
			{
				m_Chests.Remove(i);
				continue;
			}

			if (now >= record.m_ExpiresUtc)
			{
				CacheLog.Info(CacheLog.CHEST, "Сундук " + record.m_CacheId + " удалён по истечении времени.");
				Remove(i);
				continue;
			}

			if (!despawnWhenEmpty)
				continue;

			if (!IsEmpty(record.m_Chest))
				continue;

			CacheLog.Info(CacheLog.CHEST, "Сундук " + record.m_CacheId + " опустошён и удалён досрочно.");
			Remove(i);
		}
	}

	void RemoveChestOf(string cacheId)
	{
		for (int i = m_Chests.Count() - 1; i >= 0; i--)
		{
			CacheModChestRecord record = m_Chests.Get(i);
			if (record && record.m_CacheId == cacheId)
				Remove(i);
		}
	}

	void RemoveAll()
	{
		for (int i = m_Chests.Count() - 1; i >= 0; i--)
			Remove(i);
	}

	protected void Remove(int index)
	{
		CacheModChestRecord record = m_Chests.Get(index);

		if (record && record.m_Chest)
			GetGame().ObjectDelete(record.m_Chest);

		m_Chests.Remove(index);
	}

	protected CacheModChestRecord FindRecord(string cacheId)
	{
		for (int i = 0; i < m_Chests.Count(); i++)
		{
			CacheModChestRecord record = m_Chests.Get(i);
			if (record && record.m_CacheId == cacheId && record.m_Chest)
				return record;
		}

		return null;
	}

	// Empty means empty of everything, cargo and attachments alike: a chest with
	// a rifle attached but nothing in its grid is not finished with.
	protected bool IsEmpty(EntityAI chest)
	{
		array<EntityAI> contents = new array<EntityAI>();
		chest.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, contents);

		for (int i = 0; i < contents.Count(); i++)
		{
			EntityAI item = contents.Get(i);

			if (!item)
				continue;

			// EnumerateInventory reports the container itself first.
			if (item == chest)
				continue;

			return false;
		}

		return true;
	}
}

class CacheModChestRecord
{
	string m_CacheId;
	EntityAI m_Chest;
	int m_ExpiresUtc;
}
