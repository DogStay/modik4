// TrashZoneService.c
//
// Owns every trash pile in the world: places them, hands them out, takes them
// away once worked, and brings them back later somewhere else in their zone.
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
	Object m_Pile;
	int m_ZoneIndex;
	int m_RespawnAtMs;   // 0 while the pile is standing

	void JobsModPileRecord(Object pile, int zoneIndex)
	{
		m_Pile = pile;
		m_ZoneIndex = zoneIndex;
		m_RespawnAtMs = 0;
	}
}

class TrashZoneService
{
	protected static const string PILE_CLASS = "JobsMod_TrashPile";

	// How many times to look for standable ground before giving up on a slot.
	protected static const int PLACEMENT_ATTEMPTS = 12;

	// Piles are kept apart so two never overlap into one unusable heap.
	protected static const float MIN_PILE_SPACING = 6.0;

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
		array<ref JobsModZoneJson> zones = m_Config.GetZones();
		int spawned = 0;

		for (int z = 0; z < zones.Count(); z++)
		{
			JobsModZoneJson zone = zones.Get(z);

			for (int p = 0; p < zone.piles; p++)
			{
				if (SpawnPileInZone(z))
					spawned++;
			}
		}

		JobsLog.Info("SERVER/ZONES: создано куч мусора: " + spawned.ToString()
			+ " в " + zones.Count().ToString() + " зонах.");

		if (spawned == 0 && zones.Count() > 0)
			JobsLog.Error("SERVER/ZONES: ни одной кучи не создано — проверьте координаты зон для текущей карты.");
	}

	protected bool SpawnPileInZone(int zoneIndex)
	{
		JobsModZoneJson zone = m_Config.GetZones().Get(zoneIndex);

		vector position;
		if (!FindPlacement(zone, position))
		{
			JobsLog.Warning("SERVER/ZONES: не найдено место для кучи в зоне '" + zone.name + "'.");
			return false;
		}

		Object pile = GetGame().CreateObjectEx(PILE_CLASS, position, ECE_PLACE_ON_SURFACE);
		if (!pile)
		{
			JobsLog.Error("SERVER/ZONES: не удалось создать '" + PILE_CLASS + "' в зоне '" + zone.name + "'.");
			return false;
		}

		m_Piles.Insert(new JobsModPileRecord(pile, zoneIndex));
		JobsLog.Debug("SERVER/ZONES: куча создана в '" + zone.name + "' на " + position.ToString() + ".");
		return true;
	}

	// Picks a spot inside the zone that is on the terrain surface and clear of
	// the piles already standing. Gives up after a bounded number of tries so a
	// badly placed zone cannot spin here forever.
	protected bool FindPlacement(JobsModZoneJson zone, out vector position)
	{
		for (int attempt = 0; attempt < PLACEMENT_ATTEMPTS; attempt++)
		{
			float angle = Math.RandomFloat(0.0, Math.PI2);
			float distance = Math.RandomFloat(0.0, zone.radius);

			float x = zone.x + Math.Cos(angle) * distance;
			float z = zone.z + Math.Sin(angle) * distance;
			float y = GetGame().SurfaceY(x, z);

			vector candidate = Vector(x, y, z);

			if (IsClearOfOtherPiles(candidate))
			{
				position = candidate;
				return true;
			}
		}

		return false;
	}

	protected bool IsClearOfOtherPiles(vector candidate)
	{
		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (!record || !record.m_Pile)
				continue;

			if (vector.Distance(record.m_Pile.GetPosition(), candidate) < MIN_PILE_SPACING)
				return false;
		}

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

	string GetZoneName(Object pile)
	{
		JobsModPileRecord record = FindRecord(pile);
		if (!record)
			return "";

		return m_Config.GetZones().Get(record.m_ZoneIndex).name;
	}

	// Called once a shift has been accepted and paid. Deletes the pile and books
	// its return, so the same heap cannot be worked twice.
	void ConsumePile(Object pile)
	{
		JobsModPileRecord record = FindRecord(pile);
		if (!record)
			return;

		record.m_RespawnAtMs = GetGame().GetTime() + m_Config.GetPileRespawnSeconds() * 1000;
		record.m_Pile.Delete();
		record.m_Pile = null;

		JobsLog.Debug("SERVER/ZONES: куча отработана, вернётся через "
			+ m_Config.GetPileRespawnSeconds().ToString() + " с.");
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
			if (!record || record.m_Pile)
				continue;

			if (record.m_RespawnAtMs == 0 || now < record.m_RespawnAtMs)
				continue;

			JobsModZoneJson zone = m_Config.GetZones().Get(record.m_ZoneIndex);

			vector position;
			if (!FindPlacement(zone, position))
			{
				// Try again on the next tick rather than dropping the slot: the
				// blocking pile may itself be consumed shortly.
				continue;
			}

			Object pile = GetGame().CreateObjectEx(PILE_CLASS, position, ECE_PLACE_ON_SURFACE);
			if (!pile)
				continue;

			record.m_Pile = pile;
			record.m_RespawnAtMs = 0;
			JobsLog.Debug("SERVER/ZONES: куча восстановлена в '" + zone.name + "'.");
		}
	}

	void DeleteAll()
	{
		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (record && record.m_Pile)
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
			if (m_Piles.Get(i) && m_Piles.Get(i).m_Pile)
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
			if (record && record.m_Pile == pile)
				return record;
		}

		return null;
	}
}
