// TrashZoneService.c
//
// Owns every trash pile in the world: places them, hands them out, takes them
// away once worked, and brings them back later.
//
// Each pile stands on a spot the admin wrote down, not on a spot rolled inside
// a radius. A rolled spot eventually lands inside a wall, under a floor or on a
// roof, and the pile that lands there is simply unusable — with nothing in the
// log to say so, because from the server's side the spawn succeeded. An exact
// point costs the admin one visit with the coordinates on screen and removes
// the whole failure mode.
//
// Piles exist only because this service made them. A pile the service does not
// know about is refused, so an object spawned by other means cannot be used to
// mint rewards.
//
// A worked pile is removed rather than left standing on a cooldown. If it
// stayed, players would keep walking up to a pile that silently refuses them,
// and the interaction would look broken. Removing it makes the world state and
// the rules say the same thing.

class JobsModPileRecord
{
	ref JobsModPilePointJson m_Point;
	Object m_Pile;
	int m_RespawnAtMs;   // 0 while the pile is standing

	void JobsModPileRecord(JobsModPilePointJson point)
	{
		m_Point = point;
		m_Pile = null;
		m_RespawnAtMs = 0;
	}
}

class TrashZoneService
{
	protected static const string PILE_CLASS = "JobsMod_TrashPile";

	protected ref JobsModConfig m_Config;
	protected ref array<ref JobsModPileRecord> m_Piles;

	void TrashZoneService(JobsModConfig config)
	{
		m_Config = config;
		m_Piles = new array<ref JobsModPileRecord>();
	}

	// =====================================================================
	// Spawning
	// =====================================================================
	void SpawnAll()
	{
		array<ref JobsModPilePointJson> points = m_Config.GetPilePoints();
		int spawned = 0;

		for (int i = 0; i < points.Count(); i++)
		{
			JobsModPileRecord record = new JobsModPileRecord(points.Get(i));
			m_Piles.Insert(record);

			if (Place(record))
				spawned++;
		}

		JobsLog.Info("SERVER/ZONES: создано куч мусора: " + spawned.ToString() + " из " + m_Piles.Count().ToString() + " точек.");

		if (spawned == 0 && m_Piles.Count() > 0)
			JobsLog.Error("SERVER/ZONES: ни одной кучи не создано — проверьте координаты точек для текущей карты.");
	}

	protected bool Place(JobsModPileRecord record)
	{
		JobsModPilePointJson point = record.m_Point;
		vector position = point.GetPosition();

		// A height at or below zero means the config carries no height, which is
		// the common case: the admin reads x and z off the map and leaves y at
		// zero. Snapping to the terrain is the right reading of that, and
		// ECE_PLACE_ON_SURFACE settles the pile onto whatever is actually there.
		if (position[1] <= 0)
			position[1] = GetGame().SurfaceY(position[0], position[2]);

		Object pile = GetGame().CreateObjectEx(PILE_CLASS, position, ECE_PLACE_ON_SURFACE);
		if (!pile)
		{
			JobsLog.Error("SERVER/ZONES: не удалось создать '" + PILE_CLASS + "' в точке '" + point.id + "'.");
			return false;
		}

		record.m_Pile = pile;
		record.m_RespawnAtMs = 0;

		JobsLog.Debug("SERVER/ZONES: куча '" + point.id + "' создана на " + position.ToString() + ".");
		return true;
	}

	// =====================================================================
	// Lookup and consumption
	// =====================================================================
	// True only for a standing pile this service placed.
	bool IsManagedPile(Object pile)
	{
		return FindRecord(pile) != null;
	}

	string GetZoneId(Object pile)
	{
		JobsModPileRecord record = FindRecord(pile);
		if (!record)
			return "";

		return record.m_Point.zone_id;
	}

	string GetZoneName(Object pile)
	{
		JobsModPileRecord record = FindRecord(pile);
		if (!record)
			return "";

		return m_Config.GetZoneName(record.m_Point.zone_id);
	}

	// Where the piles of one zone are standing right now. A pile that has been
	// worked and is waiting to respawn is not in the list, so a marker built
	// from it can never point at an empty spot.
	void CollectStandingPiles(string zoneId, out array<vector> positions)
	{
		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (!record.m_Pile || record.m_Point.zone_id != zoneId)
				continue;

			positions.Insert(record.m_Pile.GetPosition());
		}
	}

	// Called once a pile has been sorted and accepted. Deletes it and books its
	// return, so the same heap cannot be worked twice.
	void ConsumePile(Object pile)
	{
		JobsModPileRecord record = FindRecord(pile);
		if (!record)
			return;

		record.m_RespawnAtMs = GetGame().GetTime() + m_Config.GetPileRespawnSeconds() * 1000;
		record.m_Pile.Delete();
		record.m_Pile = null;

		JobsLog.Debug("SERVER/ZONES: куча '" + record.m_Point.id + "' отработана, вернётся через " + m_Config.GetPileRespawnSeconds().ToString() + " с.");
	}

	// =====================================================================
	// Upkeep
	// =====================================================================
	void Update()
	{
		int now = GetGame().GetTime();

		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (record.m_Pile)
				continue;

			if (record.m_RespawnAtMs == 0 || now < record.m_RespawnAtMs)
				continue;

			if (Place(record))
				JobsLog.Debug("SERVER/ZONES: куча '" + record.m_Point.id + "' восстановлена.");
		}
	}

	void DeleteAll()
	{
		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (record.m_Pile)
				record.m_Pile.Delete();
		}

		m_Piles.Clear();
		JobsLog.Info("SERVER/ZONES: все кучи мусора удалены.");
	}

	int GetStandingCount()
	{
		int count = 0;

		for (int i = 0; i < m_Piles.Count(); i++)
		{
			if (m_Piles.Get(i).m_Pile)
				count++;
		}

		return count;
	}

	protected JobsModPileRecord FindRecord(Object pile)
	{
		if (!pile)
			return null;

		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (record.m_Pile && record.m_Pile == pile)
				return record;
		}

		return null;
	}
}
