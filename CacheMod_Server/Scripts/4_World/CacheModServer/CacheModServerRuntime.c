// CacheModServerRuntime.c
//
// Owns the server services and publishes them to the code the engine builds for
// us.
//
// The mission bootstrap lives in 5_Mission, but PlayerBase.OnRPC — the entry
// point for every client message — has to be modded in 4_World, because
// PlayerBase does not exist yet while 5_Mission compiles. This class bridges
// that gap: it holds the single owning reference and the bootstrap only drives
// its lifecycle.
//
// It is also where the admin plugin, which lives in a fourth PBO, reaches the
// manager. That plugin may name these types because it declares CacheMod_Server
// as a required addon; nothing here names the plugin in return.

class CacheModServerRuntime
{
	protected static ref CacheModConfig s_Config;
	protected static ref CacheModLootService s_LootService;
	protected static ref CacheModChestService s_ChestService;
	protected static ref CacheModManager s_Manager;
	protected static bool s_Started;

	static void Start()
	{
		if (!GetGame().IsServer())
			return;

		// A second Start would build a second manager, leaving the first one's
		// proxies standing with nothing owning them.
		if (s_Started)
		{
			CacheLog.Warning(CacheLog.ROOT, "Повторный запуск отклонён — мод уже работает.");
			return;
		}

		s_Config = new CacheModConfig();
		if (!s_Config.Load())
		{
			CacheLog.Error(CacheLog.ROOT, "Конфигурация не загружена. Запуск прерван.");
			s_Config = null;
			return;
		}

		s_LootService = new CacheModLootService(s_Config);
		s_ChestService = new CacheModChestService(s_Config);
		s_Manager = new CacheModManager(s_Config, s_LootService, s_ChestService);

		CacheSearchActionBridge.GetOnSearchFinished().Insert(OnSearchFinished);
		CachePlacementBridge.GetOnPlacementRequested().Insert(OnPlacementRequested);

		s_Manager.Start();

		s_Started = true;
		CacheLog.Info(CacheLog.ROOT, "Cache Mod запущен.");
	}

	static void Stop()
	{
		if (!s_Started)
			return;

		CacheSearchActionBridge.GetOnSearchFinished().Remove(OnSearchFinished);
		CachePlacementBridge.GetOnPlacementRequested().Remove(OnPlacementRequested);

		if (s_ChestService)
			s_ChestService.RemoveAll();

		if (s_Manager)
			s_Manager.Stop();

		s_Manager = null;
		s_ChestService = null;
		s_LootService = null;
		s_Config = null;
		s_Started = false;

		CacheLog.Info(CacheLog.ROOT, "Cache Mod остановлен.");
	}

	static bool IsStarted()
	{
		return s_Started;
	}

	static CacheModManager GetManager()
	{
		return s_Manager;
	}

	static CacheModConfig GetConfig()
	{
		return s_Config;
	}

	static CacheModChestService GetChestService()
	{
		return s_ChestService;
	}

	// Applies edited zones, tiers and settings without a restart. Caches are
	// deliberately not re-read: their runtime state — which ones rolled active,
	// which are counting down — exists only in memory, and rebuilding them from
	// disk would reset every timer on the server.
	static bool ReloadStaticData()
	{
		if (!s_Started || !s_Config)
		{
			CacheLog.Error(CacheLog.ADMIN, "Перезагрузка невозможна — мод не запущен.");
			return false;
		}

		return s_Config.ReloadStaticData();
	}

	// =====================================================================
	// Bridge subscribers
	// =====================================================================
	protected static void OnSearchFinished(PlayerBase player, CacheInteractionProxy proxy)
	{
		if (!s_Manager)
			return;

		s_Manager.HandleSearchFinished(player, proxy);
	}

	protected static void OnPlacementRequested(PlayerBase player, vector position, float radius, int cacheType)
	{
		if (!s_Manager)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		string createdId;
		if (s_Manager.CreateCache(player, identity, position, radius, cacheType, createdId))
		{
			SendMessage(player, "Тайник " + createdId + " создан.");
			return;
		}

		SendMessage(player, CacheModReject.GetText(CacheModReject.SAVE_FAILED));
	}

	// =====================================================================
	// Talking to one client
	// =====================================================================
	static void SendMessage(PlayerBase player, string text)
	{
		if (!player)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		GetGame().RPCSingleParam(
			player,
			CacheModRPC.NOTIFY_MESSAGE,
			new Param1<string>(text),
			true,
			identity);
	}

	// The client builds its own progress bar, so it has to be told how long a
	// search takes. Sent once, after the player has finished connecting.
	static void SendSettings(PlayerBase player)
	{
		if (!s_Started || !player)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		GetGame().RPCSingleParam(
			player,
			CacheModRPC.NOTIFY_SETTINGS,
			new Param1<float>(s_Config.GetSearchDurationSeconds()),
			true,
			identity);
	}

	static void SendAdminPoints(PlayerBase player)
	{
		if (!s_Started || !player)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		string packed = s_Manager.BuildAdminPointList();

		GetGame().RPCSingleParam(
			player,
			CacheModRPC.NOTIFY_ADMIN_POINTS,
			new Param1<string>(packed),
			true,
			identity);
	}
}
