// MissionServerJobsMod.c
//
// Drives the lifecycle of the server services and runs their periodic work.
//
// Nothing is owned here — JobsModServerRuntime holds the single reference. This
// class only says when to start, when to tick and when to stop, which keeps the
// mission free of any knowledge about what the services actually are.

modded class MissionServer
{
	// Freight is carried at walking pace, so a two second sweep of the drop-off
	// yards is as responsive as anyone can tell.
	protected static const float LOADER_INTERVAL_SECONDS = 2.0;

	// Expiries, pile respawns and missing NPCs are all minute-scale concerns and
	// share the slower timer.
	protected static const float UPKEEP_INTERVAL_SECONDS = 15.0;

	// How long after a player joins their job state is sent. Sending it in the
	// connect callback itself is too early: the client is still building the
	// world and has nowhere to put it yet.
	protected static const int WELCOME_DELAY_MS = 5000;

	protected float m_JobsModLoaderTimer;
	protected float m_JobsModUpkeepTimer;

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

		m_JobsModLoaderTimer = m_JobsModLoaderTimer + timeslice;
		if (m_JobsModLoaderTimer >= LOADER_INTERVAL_SECONDS)
		{
			m_JobsModLoaderTimer = 0.0;
			JobsModServerRuntime.GetLoaderService().Update();
		}

		m_JobsModUpkeepTimer = m_JobsModUpkeepTimer + timeslice;
		if (m_JobsModUpkeepTimer < UPKEEP_INTERVAL_SECONDS)
			return;

		m_JobsModUpkeepTimer = 0.0;

		JobsModServerRuntime.GetSessionService().PruneExpired();
		JobsModServerRuntime.GetJobService().Update();
		JobsModServerRuntime.GetZoneService().Update();
		JobsModServerRuntime.GetNpcService().Update();

		// Parcels are reconciled on the slow timer because nothing about a
		// courier job is time-critical: the delivery happens through a menu,
		// not by walking into a circle, so a stray parcel costs nothing for the
		// few seconds it survives.
		JobsModServerRuntime.GetMessengerService().Update();

		// Checked on the slow timer with everything else: an admin who has just
		// saved a file can wait the same fifteen seconds the expiry sweep does.
		JobsModServerRuntime.CheckReloadRequest();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!JobsModServerRuntime.IsStarted() || !player || !identity)
			return;

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(
			JobsModSendWelcome, WELCOME_DELAY_MS, false, player);
	}

	// A disconnecting player's session and job are dropped at once rather than
	// waiting to expire, so reconnecting does not run into their own stale state
	// and their freight does not stay standing in the world.
	override void InvokeOnDisconnect(PlayerBase player)
	{
		if (JobsModServerRuntime.IsStarted() && player && player.GetIdentity())
		{
			string playerId = player.GetIdentity().GetId();
			JobsModServerRuntime.GetSessionService().DropPlayer(playerId);
			JobsModServerRuntime.GetJobService().DropPlayer(playerId);
		}

		super.InvokeOnDisconnect(player);
	}

	// Deferred from InvokeOnConnect. The player may have dropped in between, so
	// everything is re-checked rather than assumed.
	void JobsModSendWelcome(PlayerBase player)
	{
		if (!JobsModServerRuntime.IsStarted() || !player || !player.GetIdentity())
			return;

		// Tells the HUD there is no job yet, which is what clears it after a
		// reconnect. Which survivors are employers needs no message: that rides
		// on the entities themselves.
		JobsModServerRuntime.GetJobService().SendState(player, player.GetIdentity());
	}
}
