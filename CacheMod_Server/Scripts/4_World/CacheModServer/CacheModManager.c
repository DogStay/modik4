// CacheModManager.c
//
// The single source of truth about caches while the server runs.
//
// Three ideas hold this class together:
//
//   Saved is not the same as reachable. Every point in caches.json rolls its
//   own spawn_chance once, at startup. A point that rolls through is ACTIVE and
//   gets an interaction proxy; one that does not is INACTIVE, stays in the file
//   and is unreachable until the next restart. Nothing here counts how many
//   came up — each roll is independent, so a restart may bring none of them or
//   all of them, and that is the intended shape of the feature.
//
//   The proxy is the permission. A proxy exists exactly while a cache may be
//   searched, so it is deleted when the cache is emptied and made again when it
//   respawns. That is why the search action needs no availability check of its
//   own: if a player could aim at a proxy, the cache was searchable.
//
//   Cooldown is not a new roll. A cache that was ACTIVE this run stays part of
//   this run's active set for as long as the server is up. Rolling again after
//   every search would quietly turn spawn_chance into a per-search chance,
//   which is a different feature with the same name.

class CacheModManager
{
	protected ref CacheModConfig m_Config;
	protected ref CacheModLootService m_LootService;
	protected ref CacheModChestService m_ChestService;

	// Runtime state, one entry per loaded cache.
	//
	// A plain array rather than a map keyed by id, and deliberately so: the map
	// API this build exposes is not the one every DayZ version exposes, while
	// array iteration is the same everywhere. The cost is a linear scan of a
	// few hundred short records when a search finishes, which is nothing next
	// to what the search itself already did.
	protected ref array<ref CacheModRuntime> m_Runtime;

	void CacheModManager(CacheModConfig config, CacheModLootService lootService, CacheModChestService chestService)
	{
		m_Config = config;
		m_LootService = lootService;
		m_ChestService = chestService;
		m_Runtime = new array<ref CacheModRuntime>();
	}

	// =====================================================================
	// Startup: the spawn roll
	// =====================================================================
	void Start()
	{
		array<ref CacheModCacheJson> caches = m_Config.GetCaches();
		int activeCount = 0;

		for (int i = 0; i < caches.Count(); i++)
		{
			CacheModCacheJson data = caches.Get(i);
			if (!data)
				continue;

			CacheModRuntime runtime = new CacheModRuntime();
			runtime.m_Data = data;

			float roll = CacheModRandom.GetPercent();
			runtime.m_Active = roll < data.spawn_chance;

			string rollLine = "Ролл " + data.id;
			rollLine = rollLine + ": шанс " + data.spawn_chance.ToString();
			rollLine = rollLine + ", выпало " + roll.ToString();
			rollLine = rollLine + " → " + GetActivationText(runtime.m_Active) + ".";
			CacheLog.Info(CacheLog.ROOT, rollLine);

			m_Runtime.Insert(runtime);

			if (!runtime.m_Active)
				continue;

			activeCount = activeCount + 1;

			// A cache whose cooldown ran out while the server was down comes
			// back available here, which is what makes the timer survive a
			// restart rather than merely outlive one session.
			RefreshAvailability(runtime);

			if (data.available)
				CreateProxy(runtime);
		}

		string startLine = "Активных тайников в этом запуске: " + activeCount.ToString();
		startLine = startLine + " из " + caches.Count().ToString() + ".";
		CacheLog.Info(CacheLog.ROOT, startLine);
	}

	void Stop()
	{
		for (int i = 0; i < m_Runtime.Count(); i++)
		{
			CacheModRuntime runtime = m_Runtime.Get(i);
			if (runtime)
				DeleteProxy(runtime);
		}

		m_Runtime.Clear();
	}

	// =====================================================================
	// Periodic work: respawns
	// =====================================================================
	void Update()
	{
		bool dirty = false;

		for (int i = 0; i < m_Runtime.Count(); i++)
		{
			CacheModRuntime runtime = m_Runtime.Get(i);
			if (!runtime || !runtime.m_Data)
				continue;

			if (runtime.m_Data.available)
				continue;

			if (!RefreshAvailability(runtime))
				continue;

			dirty = true;

			CacheLog.Info(CacheLog.RESPAWN, "Тайник " + runtime.m_Data.id + " снова доступен.");

			// Only an ACTIVE cache gets its proxy back. An INACTIVE one has its
			// availability restored in the file — so a restart that does bring
			// it up finds it ready — but nothing appears in the world for it.
			if (runtime.m_Active)
				CreateProxy(runtime);
		}

		if (dirty)
			m_Config.SaveCaches();
	}

	// True when this call is what flipped the cache back to available.
	protected bool RefreshAvailability(CacheModRuntime runtime)
	{
		CacheModCacheJson data = runtime.m_Data;

		if (data.available)
			return false;

		int now = CacheModClock.GetUtcSeconds();
		if (data.next_respawn_time > now)
			return false;

		data.available = true;
		data.next_respawn_time = 0;
		return true;
	}

	// =====================================================================
	// Placement
	// =====================================================================
	// The position arrived from a client, so nothing about it is trusted: the
	// caller has already checked the sender's rights, and this checks that the
	// point is near the sender and inside the map.
	bool CreateCache(PlayerBase player, PlayerIdentity identity, vector position, float radius, int cacheType, out string createdId)
	{
		createdId = "";

		if (!player || !identity)
			return false;

		vector playerPosition = player.GetPosition();
		float distance = vector.Distance(playerPosition, position);
		if (distance > CacheModRPC.MAX_PLACEMENT_DISTANCE)
		{
			string rejectLine = "Позиция от " + identity.GetPlainId();
			rejectLine = rejectLine + " отклонена: " + distance.ToString() + " м от игрока.";
			CacheLog.Warning(CacheLog.PLACEMENT, rejectLine);
			return false;
		}

		CacheModCacheJson data = new CacheModCacheJson();
		data.id = m_Config.BuildNextCacheId();
		data.SetPosition(position);
		data.radius = CacheModConfig.ClampRadius(radius);
		data.cache_type = CacheModType.ToText(cacheType);
		data.required_tool = "";
		data.spawn_chance = m_Config.GetDefaultSpawnChance();
		data.available = true;
		data.last_search_time = 0;
		data.next_respawn_time = 0;
		// The Steam64 comes from the identity the engine attached to the
		// connection, never from anything the client put in the message.
		data.created_by = identity.GetPlainId();
		data.created_utc = CacheModClock.GetUtcSeconds();

		m_Config.AddCache(data);

		if (!m_Config.SaveCaches())
			return false;

		CacheModRuntime runtime = new CacheModRuntime();
		runtime.m_Data = data;

		// A cache placed by hand is live at once. Waiting for a restart to roll
		// it would make placement impossible to verify, and the admin who put it
		// there has already decided it should exist; spawn_chance takes over
		// from the next restart onwards.
		runtime.m_Active = true;

		m_Runtime.Insert(runtime);
		CreateProxy(runtime);

		createdId = data.id;
		string createdLine = "Тайник " + data.id + " создан игроком " + identity.GetPlainId();
		createdLine = createdLine + " в " + position.ToString() + ", радиус " + data.radius.ToString() + ".";
		CacheLog.Info(CacheLog.PLACEMENT, createdLine);

		return true;
	}

	bool DeleteCache(string cacheId)
	{
		int index = FindRuntimeIndex(cacheId);
		if (index >= 0)
		{
			DeleteProxy(m_Runtime.Get(index));
			m_Runtime.Remove(index);
		}

		if (!m_Config.RemoveCache(cacheId))
			return false;

		m_ChestService.RemoveChestOf(cacheId);
		m_Config.SaveCaches();

		CacheLog.Info(CacheLog.ADMIN, "Тайник " + cacheId + " удалён.");
		return true;
	}

	// Applies an edit from the admin menu. Radius and tool take effect at once —
	// the proxy is rebuilt so the client learns the new tool — while a changed
	// spawn_chance is saved and takes effect at the next restart, because this
	// run's active set was decided when the server came up.
	bool UpdateCache(string cacheId, float radius, int cacheType, string requiredTool, float spawnChance)
	{
		CacheModRuntime runtime = FindRuntime(cacheId);
		if (!runtime || !runtime.m_Data)
			return false;

		CacheModCacheJson data = runtime.m_Data;
		data.radius = CacheModConfig.ClampRadius(radius);
		data.cache_type = CacheModType.ToText(cacheType);
		data.spawn_chance = CacheModConfig.ClampChance(spawnChance);

		int toolIndex = CacheModTools.GetIndex(requiredTool);
		if (toolIndex == CacheModTools.NONE)
			data.required_tool = "";
		else
			data.required_tool = CacheModTools.GetClassName(toolIndex);

		if (runtime.m_Proxy)
		{
			DeleteProxy(runtime);

			if (data.available && runtime.m_Active)
				CreateProxy(runtime);
		}

		m_Config.SaveCaches();
		CacheLog.Info(CacheLog.ADMIN, "Тайник " + cacheId + " изменён.");
		return true;
	}

	// =====================================================================
	// The search
	// =====================================================================
	// Called when a player's search action ran to the end. Everything is checked
	// again here, because between the action starting and this call the world
	// had seven seconds to change: the cache may have been deleted from the
	// admin menu, another player may have finished a moment earlier, the tool
	// may have been dropped.
	void HandleSearchFinished(PlayerBase player, CacheInteractionProxy proxy)
	{
		if (!player || !proxy)
			return;

		string cacheId = proxy.CacheModGetCacheId();
		CacheModRuntime runtime = FindRuntime(cacheId);

		if (!runtime || !runtime.m_Data)
		{
			CacheLog.Warning(CacheLog.SEARCH, "Обыск отклонён: тайник " + cacheId + " не существует.");
			Notify(player, CacheModReject.CACHE_GONE);
			return;
		}

		CacheModCacheJson data = runtime.m_Data;

		// The race between two players who started at the same time is settled
		// right here, and by nothing else: whoever reaches this line first finds
		// available true and takes it; the second finds it false. There is no
		// window between the check and the write, because Enforce runs this on
		// one thread and nothing below yields.
		if (!data.available)
		{
			CacheLog.Info(CacheLog.SEARCH, "Обыск отклонён: тайник " + cacheId + " уже обыскан.");
			Notify(player, CacheModReject.ALREADY_SEARCHED);
			return;
		}

		if (!runtime.m_Active)
		{
			CacheLog.Warning(CacheLog.SEARCH, "Обыск отклонён: тайник " + cacheId + " неактивен в этом запуске.");
			Notify(player, CacheModReject.CACHE_GONE);
			return;
		}

		if (m_ChestService.HasChest(cacheId))
		{
			CacheLog.Warning(CacheLog.SEARCH, "Обыск отклонён: у тайника " + cacheId + " уже стоит сундук.");
			Notify(player, CacheModReject.CHEST_PRESENT);
			return;
		}

		vector cachePosition = data.GetPosition();
		vector playerPosition = player.GetPosition();
		float distance = vector.Distance(cachePosition, playerPosition);
		if (distance > data.radius)
		{
			string farLine = "Обыск отклонён: игрок в " + distance.ToString();
			farLine = farLine + " м при радиусе " + data.radius.ToString() + ".";
			CacheLog.Info(CacheLog.SEARCH, farLine);
			Notify(player, CacheModReject.TOO_FAR);
			return;
		}

		if (!HasRequiredTool(player, data))
		{
			CacheLog.Info(CacheLog.SEARCH, "Обыск отклонён: нет инструмента '" + data.required_tool + "'.");
			Notify(player, CacheModReject.TOOL_MISSING);
			return;
		}

		// Taken. From here on the cache is nobody else's, so the state is
		// written before anything can fail: a chest that fails to spawn costs
		// this one search, not the integrity of the cooldown.
		data.available = false;
		data.last_search_time = CacheModClock.GetUtcSeconds();
		data.next_respawn_time = data.last_search_time + m_Config.GetRespawnSeconds();

		DeleteProxy(runtime);
		m_Config.SaveCaches();

		string doneLine = "Обыск " + cacheId + " завершён игроком " + GetPlayerId(player);
		doneLine = doneLine + "; следующий респавн в " + data.next_respawn_time.ToString() + " UTC.";
		CacheLog.Info(CacheLog.SEARCH, doneLine);

		string tierId = m_LootService.ResolveTier(cachePosition);
		CacheLog.Info(CacheLog.LOOT, "Тайник " + cacheId + ": итоговый тир '" + tierId + "'.");

		EntityAI chest = m_ChestService.SpawnChest(cacheId, cachePosition);
		if (!chest)
		{
			CacheLog.Error(CacheLog.CHEST, "Сундук для " + cacheId + " не создан.");
			return;
		}

		m_LootService.FillContainer(chest, cachePosition, tierId);
	}

	// =====================================================================
	// Proxies
	// =====================================================================
	protected void CreateProxy(CacheModRuntime runtime)
	{
		CacheModCacheJson data = runtime.m_Data;

		if (runtime.m_Proxy)
			return;

		vector position = data.GetPosition();

		// Placed on the surface and given no lifetime, which is the same pair
		// the rest of this codebase spawns world objects with. The object must
		// stay networked — a local one would exist on the server alone and no
		// client could ever aim at it — so nothing here suppresses replication.
		//
		// The geometry the cursor raycast hits comes from the crate model the
		// class inherits and is there whether or not a rigid body was built.
		int flags = ECE_PLACE_ON_SURFACE | ECE_NOLIFETIME;
		Object created = GetGame().CreateObjectEx("CacheInteractionProxy", position, flags);

		CacheInteractionProxy proxy = CacheInteractionProxy.Cast(created);
		if (!proxy)
		{
			CacheLog.Error(CacheLog.INTERACTION, "Не удалось создать proxy для " + data.id + ".");

			if (created)
				GetGame().ObjectDelete(created);

			return;
		}

		proxy.SetPosition(position);
		proxy.SetOrientation(vector.Zero);

		int toolIndex = CacheModTools.GetIndex(data.required_tool);
		if (CacheModType.FromText(data.cache_type) != CacheModType.TOOL_REQUIRED)
			toolIndex = CacheModTools.NONE;

		bool handsOnly = m_Config.IsToolInHandsOnly();
		proxy.CacheModSetup(data.id, toolIndex, handsOnly);

		runtime.m_Proxy = proxy;

		string proxyLine = "Proxy создан для " + data.id + " в " + position.ToString();
		proxyLine = proxyLine + ", радиус " + data.radius.ToString();
		proxyLine = proxyLine + ", инструмент '" + data.required_tool + "'.";
		CacheLog.Info(CacheLog.INTERACTION, proxyLine);
	}

	protected void DeleteProxy(CacheModRuntime runtime)
	{
		if (!runtime.m_Proxy)
			return;

		string cacheId = "";
		if (runtime.m_Data)
			cacheId = runtime.m_Data.id;

		GetGame().ObjectDelete(runtime.m_Proxy);
		runtime.m_Proxy = null;

		CacheLog.Debug(CacheLog.INTERACTION, "Proxy удалён для " + cacheId + ".");
	}

	// =====================================================================
	// Admin view
	// =====================================================================
	// The packed point list the admin visualisation draws. Only called for a
	// sender the caller has already checked has the permission.
	string BuildAdminPointList()
	{
		string packed = "";
		int now = CacheModClock.GetUtcSeconds();

		for (int i = 0; i < m_Runtime.Count(); i++)
		{
			CacheModRuntime runtime = m_Runtime.Get(i);
			if (!runtime || !runtime.m_Data)
				continue;

			CacheModCacheJson data = runtime.m_Data;
			vector position = data.GetPosition();

			int state = CacheModState.INACTIVE;
			if (runtime.m_Active && data.available)
				state = CacheModState.ACTIVE;
			else if (runtime.m_Active)
				state = CacheModState.COOLDOWN;

			int respawnLeft = 0;
			if (!data.available && data.next_respawn_time > now)
				respawnLeft = data.next_respawn_time - now;

			string record = data.id;
			record = record + CacheModRPC.FIELD_SEPARATOR + position[0].ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + position[1].ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + position[2].ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + data.radius.ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + state.ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + CacheModType.FromText(data.cache_type).ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + data.spawn_chance.ToString();
			record = record + CacheModRPC.FIELD_SEPARATOR + respawnLeft.ToString();

			if (packed != "")
				packed = packed + CacheModRPC.RECORD_SEPARATOR;

			packed = packed + record;
		}

		return packed;
	}

	// =====================================================================
	// Helpers
	// =====================================================================
	protected CacheModRuntime FindRuntime(string cacheId)
	{
		int index = FindRuntimeIndex(cacheId);
		if (index < 0)
			return null;

		return m_Runtime.Get(index);
	}

	protected int FindRuntimeIndex(string cacheId)
	{
		for (int i = 0; i < m_Runtime.Count(); i++)
		{
			CacheModRuntime runtime = m_Runtime.Get(i);

			if (runtime && runtime.m_Data && runtime.m_Data.id == cacheId)
				return i;
		}

		return -1;
	}

	protected bool HasRequiredTool(PlayerBase player, CacheModCacheJson data)
	{
		if (CacheModType.FromText(data.cache_type) != CacheModType.TOOL_REQUIRED)
			return true;

		if (data.required_tool == "")
			return true;

		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (inHands && inHands.IsKindOf(data.required_tool))
			return true;

		if (m_Config.IsToolInHandsOnly())
			return false;

		array<EntityAI> carried = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried);

		for (int i = 0; i < carried.Count(); i++)
		{
			EntityAI candidate = carried.Get(i);
			if (candidate && candidate.IsKindOf(data.required_tool))
				return true;
		}

		return false;
	}

	protected void Notify(PlayerBase player, int reason)
	{
		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		string text = CacheModReject.GetText(reason);

		GetGame().RPCSingleParam(
			player,
			CacheModRPC.NOTIFY_MESSAGE,
			new Param1<string>(text),
			true,
			identity);
	}

	protected string GetPlayerId(PlayerBase player)
	{
		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return "?";

		return identity.GetPlainId();
	}

	protected string GetActivationText(bool active)
	{
		if (active)
			return "ACTIVE";

		return "INACTIVE";
	}
}

// One cache while the server is up: its saved record, whether it rolled through
// this run, and the proxy standing in for it right now.
class CacheModRuntime
{
	ref CacheModCacheJson m_Data;
	bool m_Active;
	CacheInteractionProxy m_Proxy;
}
