// CacheModLootService.c
//
// Decides what a finished search is worth, and puts it in the chest.
//
// The tier is never stored on the cache. It is resolved from the cache's
// position at the moment the search completes, every time, which is what makes
// loot zones editable at runtime: draw a military zone around an existing point
// and it starts paying out military loot on the next search; delete the zone
// and it goes back to the default tier. Nothing about the cache itself changes,
// because the cache never knew.

class CacheModLootService
{
	protected ref CacheModConfig m_Config;

	void CacheModLootService(CacheModConfig config)
	{
		m_Config = config;
	}

	// =====================================================================
	// Which zone, which tier
	// =====================================================================
	// Highest priority wins where zones overlap. On equal priority the smaller
	// zone wins, because a small zone inside a large one is the more specific
	// statement — the airfield's hangar row is not simply "the airfield".
	CacheModZoneJson FindZone(vector position)
	{
		array<ref CacheModZoneJson> zones = m_Config.GetZones();
		CacheModZoneJson best = null;

		for (int i = 0; i < zones.Count(); i++)
		{
			CacheModZoneJson zone = zones.Get(i);
			if (!zone)
				continue;

			vector center = zone.GetCenter();

			// Distance is measured flat. A zone is a circle on the map, and a
			// cache in a basement or on a roof is still inside it — comparing
			// heights would exclude exactly the places worth hiding things in.
			float dx = position[0] - center[0];
			float dz = position[2] - center[2];
			float distanceSquared = dx * dx + dz * dz;
			float radiusSquared = zone.radius * zone.radius;

			if (distanceSquared > radiusSquared)
				continue;

			if (!best)
			{
				best = zone;
				continue;
			}

			if (zone.priority > best.priority)
			{
				best = zone;
				continue;
			}

			if (zone.priority < best.priority)
				continue;

			if (zone.radius < best.radius)
				best = zone;
		}

		return best;
	}

	string ResolveTier(vector position)
	{
		CacheModZoneJson zone = FindZone(position);

		if (!zone)
		{
			string fallback = m_Config.GetDefaultTier();
			CacheLog.Info(CacheLog.ZONE, "Точка " + position.ToString() + " вне зон → тир по умолчанию '" + fallback + "'.");
			return fallback;
		}

		CacheLog.Info(CacheLog.ZONE, "Точка " + position.ToString() + " в зоне '" + zone.id + "' → тир '" + zone.tier + "'.");
		return zone.tier;
	}

	// =====================================================================
	// Generation
	// =====================================================================
	void FillContainer(EntityAI container, vector position, string tierId)
	{
		if (!container)
			return;

		CacheModTierJson tier = m_Config.GetTier(tierId);
		if (!tier)
		{
			CacheLog.Error(CacheLog.LOOT, "Тир '" + tierId + "' не найден — сундук останется пустым.");
			return;
		}

		if (tier.items.Count() == 0)
		{
			CacheLog.Warning(CacheLog.LOOT, "Тир '" + tierId + "' не содержит предметов.");
			return;
		}

		int minItems = tier.min_items;
		int maxItems = tier.max_items;

		// A zone may override the count without owning a tier of its own, which
		// is how the same military tier can be generous at the airfield and thin
		// at a roadside checkpoint.
		CacheModZoneJson zone = FindZone(position);
		if (zone && zone.max_items > 0)
		{
			minItems = zone.min_items;
			maxItems = zone.max_items;
		}

		if (minItems < 1)
			minItems = 1;

		if (maxItems < minItems)
			maxItems = minItems;

		int drawCount = Math.RandomIntInclusive(minItems, maxItems);
		int spawned = 0;

		for (int i = 0; i < drawCount; i++)
		{
			CacheModLootItemJson entry = DrawWeighted(tier);
			if (!entry)
				continue;

			int count = Math.RandomIntInclusive(entry.min, entry.max);

			for (int c = 0; c < count; c++)
			{
				EntityAI created = SpawnOne(container, entry);
				if (created)
					spawned = spawned + 1;
			}
		}

		string summary = "Сгенерировано предметов: " + spawned.ToString();
		summary = summary + " (розыгрышей " + drawCount.ToString() + ", тир '" + tierId + "').";
		CacheLog.Info(CacheLog.LOOT, summary);
	}

	// Weight is relative, not a percentage: an entry of 20 against one of 10 is
	// twice as likely whatever else is in the list, so an admin can add an item
	// without rebalancing every other number in the tier.
	protected CacheModLootItemJson DrawWeighted(CacheModTierJson tier)
	{
		float total = 0.0;

		for (int i = 0; i < tier.items.Count(); i++)
		{
			CacheModLootItemJson item = tier.items.Get(i);
			if (item)
				total = total + item.weight;
		}

		if (total <= 0.0)
			return null;

		float roll = Math.RandomFloat(0.0, total);
		float running = 0.0;

		for (int j = 0; j < tier.items.Count(); j++)
		{
			CacheModLootItemJson candidate = tier.items.Get(j);
			if (!candidate)
				continue;

			running = running + candidate.weight;

			if (roll <= running)
				return candidate;
		}

		return tier.items.Get(tier.items.Count() - 1);
	}

	protected EntityAI SpawnOne(EntityAI container, CacheModLootItemJson entry)
	{
		EntityAI created = container.GetInventory().CreateInInventory(entry.classname);

		if (!created)
		{
			// Almost always one of two things: a classname from a mod the
			// server does not load, or a chest with no room left. Both are the
			// admin's to fix, and both are worth naming.
			CacheLog.Warning(CacheLog.LOOT, "Предмет '" + entry.classname + "' не создан (нет класса или нет места).");
			return null;
		}

		RollAttachments(created, entry);
		return created;
	}

	protected void RollAttachments(EntityAI parent, CacheModLootItemJson entry)
	{
		if (!entry.attachments)
			return;

		for (int i = 0; i < entry.attachments.Count(); i++)
		{
			CacheModAttachmentJson attachment = entry.attachments.Get(i);
			if (!attachment)
				continue;

			float roll = Math.RandomFloat(0.0, 100.0);
			if (roll >= attachment.chance)
				continue;

			for (int q = 0; q < attachment.quantity; q++)
			{
				EntityAI created = parent.GetInventory().CreateAttachment(attachment.classname);

				if (!created)
				{
					// A second magazine has nowhere to go on a rifle with one
					// magazine slot, which is normal rather than an error; it is
					// logged at debug so a tier with a real typo can still be
					// found by turning debug on.
					CacheLog.Debug(CacheLog.LOOT, "Аттачмент '" + attachment.classname + "' не установлен на '" + entry.classname + "'.");
					break;
				}
			}
		}
	}
}
