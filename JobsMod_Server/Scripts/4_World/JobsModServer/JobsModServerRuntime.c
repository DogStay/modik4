// JobsModServerRuntime.c
//
// Owns the server services and publishes them to code the engine constructs
// for us.
//
// The mission bootstrap lives in 5_Mission, but PlayerBase.OnRPC — the entry
// point for every client message — has to be modded in 4_World, because
// PlayerBase does not exist yet while 5_Mission compiles. This class bridges
// that gap: it holds the single owning reference, and the bootstrap only drives
// its lifecycle.

class JobsModServerRuntime
{
	protected static ref JobsModConfig s_Config;
	protected static ref JobsModNpcService s_NpcService;
	protected static ref JobsModLoaderService s_LoaderService;
	protected static ref JobsModMessengerService s_MessengerService;
	protected static ref JobsModJobService s_JobService;
	protected static ref TrashZoneService s_ZoneService;
	protected static ref SortingSessionService s_SessionService;
	protected static bool s_Started;

	static void Start()
	{
		if (!GetGame().IsServer())
			return;

		// A second Start would build a second set of services, leaving the first
		// one's assignments unreachable but still counted against players.
		if (s_Started)
		{
			JobsLog.Warning("SERVER: повторный запуск отклонён — службы уже работают.");
			return;
		}

		string catalogError;
		if (!JobsModTrashCatalog.Validate(catalogError))
		{
			JobsLog.Error("SERVER: каталог мусора некорректен (" + catalogError + "). Запуск прерван.");
			return;
		}

		// Config first: every service is built from it and there is nothing
		// sensible for any of them to do without it.
		s_Config = new JobsModConfig();
		if (!s_Config.Load())
		{
			JobsLog.Error("SERVER: конфигурация не загружена. Запуск прерван.");
			s_Config = null;
			return;
		}

		JobsLog.s_DebugEnabled = s_Config.IsDebugLogging();

		s_NpcService = new JobsModNpcService(s_Config);
		s_NpcService.SpawnAll();

		s_ZoneService = new TrashZoneService(s_Config);
		s_ZoneService.SpawnAll();

		// The loader, the courier and the job service all need each other: the
		// job service asks for freight and parcels when a job starts, and both
		// report back into it. The two are built first without the reference
		// and given it once the job service exists, so none of the three has to
		// be half-constructed.
		s_LoaderService = new JobsModLoaderService(s_Config);
		s_MessengerService = new JobsModMessengerService(s_Config);
		s_JobService = new JobsModJobService(s_Config, s_NpcService, s_LoaderService, s_MessengerService, s_ZoneService);
		s_LoaderService.SetJobService(s_JobService);
		s_MessengerService.SetJobService(s_JobService);

		s_SessionService = new SortingSessionService(s_Config, s_ZoneService, s_JobService);

		JobsModTrashActionBridge.GetOnSortRequested().Insert(OnSortRequested);
		JobsModNpcActionBridge.GetOnTalkRequested().Insert(OnTalkRequested);

		s_Started = true;
		JobsLog.Info("SERVER: JobsMod запущен. Build: " + JobsModBuildInfo.BUILD +
			"; protocol=" + JobsModRPC.PROTOCOL_VERSION.ToString() + ".");
	}

	static void Stop()
	{
		if (!s_Started)
			return;

		JobsModTrashActionBridge.GetOnSortRequested().Remove(OnSortRequested);
		JobsModNpcActionBridge.GetOnTalkRequested().Remove(OnTalkRequested);

		if (s_ZoneService)
			s_ZoneService.DeleteAll();

		if (s_NpcService)
			s_NpcService.DeleteAll();

		s_SessionService = null;
		s_JobService = null;
		s_MessengerService = null;
		s_LoaderService = null;
		s_ZoneService = null;
		s_NpcService = null;
		s_Config = null;
		s_Started = false;

		JobsLog.Info("SERVER: JobsMod остановлен.");
	}

	static bool IsStarted()
	{
		return s_Started;
	}

	static SortingSessionService GetSessionService()
	{
		return s_SessionService;
	}

	static TrashZoneService GetZoneService()
	{
		return s_ZoneService;
	}

	static JobsModNpcService GetNpcService()
	{
		return s_NpcService;
	}

	static JobsModJobService GetJobService()
	{
		return s_JobService;
	}

	static JobsModLoaderService GetLoaderService()
	{
		return s_LoaderService;
	}

	static JobsModMessengerService GetMessengerService()
	{
		return s_MessengerService;
	}

	// Subscribed to the client PBO's bridges, so neither action ever names a
	// server type. Static because a ScriptInvoker outlives no instance here.
	protected static void OnSortRequested(PlayerBase player, Object pile)
	{
		if (!s_SessionService)
			return;

		s_SessionService.HandleSortRequested(player, pile);
	}

	protected static void OnTalkRequested(PlayerBase player, Object npc)
	{
		if (!s_JobService)
			return;

		s_JobService.HandleTalk(player, npc);
	}
}
