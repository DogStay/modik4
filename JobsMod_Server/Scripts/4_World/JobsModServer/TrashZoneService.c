// TrashZoneService.c
//
// One active physical trash object per zone. Config may contain many candidate
// points, but only one of them is selected on a server start. The object itself
// is a random vanilla model from JobsModTrashCatalog and is marked as
// non-takeable by ItemBaseJobsModTrash.c.

class JobsModPileRecord
{
	ref JobsModPilePointJson m_Point;
	Object m_Pile;
	int m_RespawnAtMs;

	void JobsModPileRecord(JobsModPilePointJson point)
	{
		m_Point = point;
		m_Pile = null;
		m_RespawnAtMs = 0;
	}
}

class TrashZoneService
{
	protected static const string LEGACY_PILE_CLASS = "JobsMod_TrashPile";
	protected static const float STALE_CLEAN_RADIUS = 2.0;

	protected ref JobsModConfig m_Config;
	protected ref array<ref JobsModPileRecord> m_Piles;

	void TrashZoneService(JobsModConfig config)
	{
		m_Config = config;
		m_Piles = new array<ref JobsModPileRecord>();
	}

	void SpawnAll()
	{
		array<ref JobsModPilePointJson> points = m_Config.GetPilePoints();
		array<string> zoneIds = new array<string>();

		for (int i = 0; i < points.Count(); i++)
		{
			JobsModPileRecord record = new JobsModPileRecord(points.Get(i));
			m_Piles.Insert(record);
			CleanupStaleAtPoint(record.m_Point);

			string zoneId = record.m_Point.zone_id;
			if (zoneIds.Find(zoneId) == -1)
				zoneIds.Insert(zoneId);
		}

		int spawned = 0;
		for (i = 0; i < zoneIds.Count(); i++)
		{
			JobsModPileRecord selected = PickRandomRecord(zoneIds.Get(i));
			if (selected && Place(selected))
				spawned++;
		}

		JobsLog.Info("SERVER/ZONES: создано активных предметов мусора: " + spawned.ToString() + "; правило: один предмет на зону.");

		if (spawned == 0 && m_Piles.Count() > 0)
			JobsLog.Error("SERVER/ZONES: ни одного предмета мусора не создано — проверьте координаты и классы каталога.");
	}

	protected JobsModPileRecord PickRandomRecord(string zoneId)
	{
		array<ref JobsModPileRecord> candidates = new array<ref JobsModPileRecord>();

		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (record.m_Point.zone_id == zoneId)
				candidates.Insert(record);
		}

		if (candidates.Count() == 0)
			return null;

		int index = Math.RandomInt(0, candidates.Count());
		return candidates.Get(index);
	}

	protected bool Place(JobsModPileRecord record)
	{
		JobsModPilePointJson point = record.m_Point;
		vector position = ResolvePointPosition(point);

		array<ref JobsModTrashItem> catalog = JobsModTrashCatalog.GetItems();
		if (!catalog || catalog.Count() == 0)
		{
			JobsLog.Error("SERVER/ZONES: каталог мусора пуст.");
			return false;
		}

		int randomIndex = Math.RandomInt(0, catalog.Count());
		JobsModTrashItem definition = catalog.Get(randomIndex);
		string className = definition.GetWorldClassName();

		Object created = GetGame().CreateObjectEx(className, position, ECE_PLACE_ON_SURFACE);
		ItemBase trash = ItemBase.Cast(created);
		if (!trash)
		{
			if (created)
				created.Delete();

			JobsLog.Error("SERVER/ZONES: не удалось создать предмет мусора '" + className + "' в точке '" + point.id + "'.");
			return false;
		}

		trash.JobsModSetWorldTrash(true);
		record.m_Pile = trash;
		record.m_RespawnAtMs = 0;

		JobsLog.Info("SERVER/ZONES: активный мусор '" + definition.GetDisplayName() + "' (" + className + ") создан в точке '" + point.id + "' на " + position.ToString() + ".");
		return true;
	}

	protected vector ResolvePointPosition(JobsModPilePointJson point)
	{
		vector position = point.GetPosition();
		if (position[1] <= 0)
			position[1] = GetGame().SurfaceY(position[0], position[2]);

		return position;
	}

	// object sitting exactly on a configured work point. This prevents persistent
	// world storage from stacking another object on every server restart.
	protected void CleanupStaleAtPoint(JobsModPilePointJson point)
	{
		vector position = ResolvePointPosition(point);
		array<Object> objects = new array<Object>();
		array<CargoBase> proxyCargo = new array<CargoBase>();
		GetGame().GetObjectsAtPosition(position, STALE_CLEAN_RADIUS, objects, proxyCargo);

		int removed = 0;
		for (int i = 0; i < objects.Count(); i++)
		{
			Object object = objects.Get(i);
			if (!object)
				continue;

			string type = object.GetType();
			if (type == LEGACY_PILE_CLASS)
			{
				object.Delete();
				removed++;
				continue;
			}

			// The configured work point is reserved for JobsMod. Delete one of our
			// catalog models there even when a restart restored the vanilla item
			// without the transient network flag. This prevents stacking forever.
			if (JobsModTrashCatalog.IsWorldClassName(type))
			{
				object.Delete();
				removed++;
			}
		}

		if (removed > 0)
			JobsLog.Info("SERVER/ZONES: удалено старых JobsMod-объектов в точке '" + point.id + "': " + removed.ToString() + ".");
	}

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

	void CollectStandingPiles(string zoneId, out array<vector> positions)
	{
		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (!record.m_Pile)
				continue;

			if (record.m_Point.zone_id != zoneId)
				continue;

			positions.Insert(record.m_Pile.GetPosition());
		}
	}

	void ConsumePile(Object pile)
	{
		JobsModPileRecord record = FindRecord(pile);
		if (!record)
			return;

		record.m_RespawnAtMs = GetGame().GetTime() + m_Config.GetPileRespawnSeconds() * 1000;
		record.m_Pile.Delete();
		record.m_Pile = null;

		JobsLog.Debug("SERVER/ZONES: предмет мусора в точке '" + record.m_Point.id + "' отсортирован; новый случайный предмет появится через " + m_Config.GetPileRespawnSeconds().ToString() + " с.");
	}

	void Update()
	{
		int now = GetGame().GetTime();

		for (int i = 0; i < m_Piles.Count(); i++)
		{
			JobsModPileRecord record = m_Piles.Get(i);
			if (record.m_Pile)
				continue;

			if (record.m_RespawnAtMs == 0)
				continue;

			if (now < record.m_RespawnAtMs)
				continue;

			if (Place(record))
				JobsLog.Debug("SERVER/ZONES: новый случайный предмет мусора восстановлен в точке '" + record.m_Point.id + "'.");
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
		JobsLog.Info("SERVER/ZONES: все активные предметы мусора удалены.");
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
