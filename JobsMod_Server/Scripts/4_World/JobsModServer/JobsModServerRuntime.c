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
	protected static ref SortingSessionService s_SessionService;
	protected static bool s_Started;

	static void Start()
	{
		if (!GetGame().IsServer())
			return;

		// A second Start would build a second service, leaving the first one's
		// sessions unreachable but still counted against players.
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

		s_SessionService = new SortingSessionService();
		JobsModTrashActionBridge.GetOnSortRequested().Insert(OnSortRequested);

		s_Started = true;
		JobsLog.Info("SERVER: JobsMod запущен. Build: " + JobsModBuildInfo.BUILD
			+ "; protocol=" + JobsModRPC.PROTOCOL_VERSION.ToString() + ".");
	}

	static void Stop()
	{
		if (!s_Started)
			return;

		JobsModTrashActionBridge.GetOnSortRequested().Remove(OnSortRequested);
		s_SessionService = null;
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

	// Subscribed to the client PBO's bridge, so the action never names a server
	// type. Static because ScriptInvoker outlives no instance here.
	protected static void OnSortRequested(PlayerBase player, Object pile)
	{
		if (!s_SessionService)
			return;

		s_SessionService.HandleSortRequested(player, pile);
	}
}
