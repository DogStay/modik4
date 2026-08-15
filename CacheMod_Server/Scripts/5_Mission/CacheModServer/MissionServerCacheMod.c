// MissionServerCacheMod.c
//
// Drives the lifecycle of the server side and runs its periodic work.
//
// Nothing is owned here — CacheModServerRuntime holds the single reference.
// This class only says when to start, when to tick and when to stop.

modded class MissionServer
{
	// Respawns and chest expiry are both minute-scale concerns: a cache that
	// comes back ten seconds late is indistinguishable from one that came back
	// on time, and a sweep every ten seconds on a server with hundreds of
	// points is work nobody can perceive.
	protected static const float CACHEMOD_UPKEEP_SECONDS = 10.0;

	// How long after a player joins their settings are sent. Sending it from
	// the connect callback itself is too early: the client is still building
	// the world and has nowhere to put it.
	protected static const int CACHEMOD_WELCOME_DELAY_MS = 5000;

	protected float m_CacheModUpkeepTimer;

	override void OnInit()
	{
		super.OnInit();

		CacheModAuthority.Load();
		CacheModServerRuntime.Start();
	}

	override void OnMissionFinish()
	{
		CacheModServerRuntime.Stop();
		super.OnMissionFinish();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!CacheModServerRuntime.IsStarted())
			return;

		m_CacheModUpkeepTimer = m_CacheModUpkeepTimer + timeslice;
		if (m_CacheModUpkeepTimer < CACHEMOD_UPKEEP_SECONDS)
			return;

		m_CacheModUpkeepTimer = 0.0;

		CacheModServerRuntime.GetManager().Update();
		CacheModServerRuntime.GetChestService().Update();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!CacheModServerRuntime.IsStarted() || !player || !identity)
			return;

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(
			CacheModSendWelcome, CACHEMOD_WELCOME_DELAY_MS, false, player);
	}

	// Deferred from InvokeOnConnect, so the player may have dropped in between:
	// everything is re-checked rather than assumed.
	void CacheModSendWelcome(PlayerBase player)
	{
		if (!CacheModServerRuntime.IsStarted() || !player || !player.GetIdentity())
			return;

		CacheModServerRuntime.SendSettings(player);
	}
}
