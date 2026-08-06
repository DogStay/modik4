// MissionServerJobsMod.c
//
// Drives the lifecycle of the server services and prunes dead sessions.
//
// Nothing is owned here — JobsModServerRuntime holds the single reference. This
// class only says when to start, when to tick and when to stop, which keeps the
// mission free of any knowledge about what the services actually are.

modded class MissionServer
{
	// Sessions and pile respawns are both minute-scale concerns, so the sweep
	// runs on a slow timer rather than every frame.
	protected static const float PRUNE_INTERVAL_SECONDS = 15.0;

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

		TrashZoneService zones = JobsModServerRuntime.GetZoneService();
		if (zones)
			zones.Update();
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
