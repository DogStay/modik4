// MissionServerJobsMod.c
//
// Drives the lifecycle of the server services and prunes dead sessions.
//
// Nothing is owned here — JobsModServerRuntime holds the single reference. This
// class only says when to start, when to tick and when to stop, which keeps the
// mission free of any knowledge about what the services actually are.

modded class MissionServer
{
	// Expired sessions are cheap to hold and pointless to scan every frame, so
	// the sweep runs on a slow timer instead.
	protected static const float PRUNE_INTERVAL_SECONDS = 30.0;

	protected float m_JobsModPruneTimer;

	override void OnInit()
	{
		super.OnInit();
		JobsModServerRuntime.Start();
	}

	override void OnMissionFinish()
	{
		JobsModServerRuntime.Stop();
		super.OnMissionFinish();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!JobsModServerRuntime.IsStarted())
			return;

		m_JobsModPruneTimer = m_JobsModPruneTimer + timeslice;
		if (m_JobsModPruneTimer < PRUNE_INTERVAL_SECONDS)
			return;

		m_JobsModPruneTimer = 0.0;

		SortingSessionService service = JobsModServerRuntime.GetSessionService();
		if (service)
			service.PruneExpired();
	}

	// A disconnecting player's session is dropped at once rather than waiting to
	// expire, so reconnecting does not run into their own stale ALREADY_BUSY.
	override void InvokeOnDisconnect(PlayerBase player)
	{
		if (player && player.GetIdentity())
		{
			SortingSessionService service = JobsModServerRuntime.GetSessionService();
			if (service)
				service.DropPlayer(player.GetIdentity().GetId());
		}

		super.InvokeOnDisconnect(player);
	}
}
