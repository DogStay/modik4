// JobsModJobService.c
//
// Owns every job a player is holding: who has one, how far along it is, and
// what it pays.
//
// =========================================================================
// TRUST MODEL
// =========================================================================
// Untrusted: everything inside ParamsReadContext. A client may send any npc id,
// any job id, any assignment id, at any rate.
//
// Trusted: PlayerIdentity, which the engine attaches rather than the client, and
// the world itself — where the player stands and where the NPC stands.
//
// A client cannot finish a job by asking. There is no "job done" message:
// progress is only ever added by the server, from a pile it watched being sorted
// or a box it watched being delivered. The client's only say in the matter is
// which job to start and when to walk back for the money — and both are checked
// against the NPC's real position.
// =========================================================================

class JobsModJobService
{
	protected static const string REWARD_CLASS = "JobsMod_Money";

	protected ref JobsModConfig m_Config;
	protected JobsModNpcService m_Npcs;
	protected JobsModLoaderService m_Loader;

	// One assignment per player, keyed by identity id.
	protected ref map<string, ref JobsModAssignment> m_Assignments;
	// "<playerId>|<jobId>" -> server time the cooldown runs out at.
	protected ref map<string, int> m_CooldownUntilMs;

	protected int m_NextAssignmentId;

	void JobsModJobService(JobsModConfig config, JobsModNpcService npcs, JobsModLoaderService loader)
	{
		m_Config = config;
		m_Npcs = npcs;
		m_Loader = loader;
		m_Assignments = new map<string, ref JobsModAssignment>();
		m_CooldownUntilMs = new map<string, int>();
		m_NextAssignmentId = Math.RandomInt(1000, 100000);
	}

	// =====================================================================
	// Queries used by the other services
	// =====================================================================
	JobsModAssignment GetAssignment(string playerId)
	{
		JobsModAssignment assignment;
		if (m_Assignments.Find(playerId, assignment))
			return assignment;

		return null;
	}

	// Every open assignment, for services that have to sweep them all. Handed
	// out as plain references: the map keeps ownership, so a caller cannot keep
	// an assignment alive past the point this service dropped it.
	void CollectAssignments(out array<JobsModAssignment> result)
	{
		for (int i = 0; i < m_Assignments.Count(); i++)
			result.Insert(m_Assignments.GetElement(i));
	}

	// =====================================================================
	// Talking to an NPC
	// =====================================================================
	// Raised from the talk action's server half. The object is what the player
	// actually interacted with, so the npc id is resolved from the world here
	// and never taken from the client.
	void HandleTalk(PlayerBase player, Object npcObject)
	{
		if (!GetGame().IsServer() || !player || !npcObject)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		string npcId = m_Npcs.GetNpcIdByObject(npcObject);
		if (npcId == "")
		{
			Reject(player, identity, JobsModRejectReason.UNKNOWN_NPC);
			return;
		}

		if (!IsPlayerReady(player))
		{
			Reject(player, identity, JobsModRejectReason.PLAYER_NOT_READY);
			return;
		}

		if (!m_Npcs.IsPlayerAtNpc(player, npcId))
		{
			Reject(player, identity, JobsModRejectReason.TOO_FAR);
			return;
		}

		SendMenu(player, identity, npcId);
	}

	protected void SendMenu(PlayerBase player, PlayerIdentity identity, string npcId)
	{
		JobsModNpcJson npc = m_Config.GetNpc(npcId);
		if (!npc)
		{
			Reject(player, identity, JobsModRejectReason.UNKNOWN_NPC);
			return;
		}

		string playerId = identity.GetId();
		JobsModAssignment held = GetAssignment(playerId);

		array<ref Param> offers = new array<ref Param>();

		for (int i = 0; i < npc.jobs.Count(); i++)
		{
			JobsModJobJson job = m_Config.GetJob(npc.jobs.Get(i));
			if (!job)
				continue;

			int cooldown = GetCooldownRemaining(playerId, job.id);
			int availability = JobsModOfferState.AVAILABLE;

			if (held && held.GetJobId() == job.id)
				availability = JobsModOfferState.HELD;
			else if (held)
				availability = JobsModOfferState.BLOCKED_BY_OTHER;
			else if (cooldown > 0)
				availability = JobsModOfferState.ON_COOLDOWN;

			int required = job.piles_required;
			if (JobsModJobType.FromText(job.type) == JobsModJobType.LOADING)
				required = job.cargos_required;

			offers.Insert(new Param4<string, string, string, string>(
				job.id, job.name, job.description, m_Config.GetZoneName(job.zone_id)));
			offers.Insert(new Param4<int, int, int, int>(
				job.reward, required, cooldown, availability));
		}

		// The hand-in button belongs to the NPC that issued the job, so it is
		// only offered here when this is that NPC and the work is done.
		int handInAvailable = 0;
		int assignmentId = 0;
		string heldJobName = "";
		int heldStatus = JobsModJobStatus.NONE;

		if (held)
		{
			assignmentId = held.GetId();
			heldStatus = held.GetStatus();

			JobsModJobJson heldJob = m_Config.GetJob(held.GetJobId());
			if (heldJob)
				heldJobName = heldJob.name;

			if (held.IsFinished() && held.GetNpcId() == npcId)
				handInAvailable = 1;
		}

		array<ref Param> message = new array<ref Param>();
		message.Insert(new Param4<string, string, string, int>(
			npc.id, npc.name, npc.description, offers.Count() / 2));
		message.Insert(new Param4<int, int, string, int>(
			handInAvailable, assignmentId, heldJobName, heldStatus));

		for (int o = 0; o < offers.Count(); o++)
			message.Insert(offers.Get(o));

		GetGame().RPC(player, JobsModRPC.NOTIFY_JOB_MENU, message, true, identity);
		JobsLog.Debug("SERVER/JOBS: меню NPC '" + npcId + "' отправлено '" + identity.GetName() + "'.");
	}

	// =====================================================================
	// Taking a job
	// =====================================================================
	void HandleAccept(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		Param3<int, string, string> request = new Param3<int, string, string>(0, "", "");
		if (!ctx.Read(request))
		{
			JobsLog.Warning("SERVER/RPC: не удалось прочитать REQUEST_JOB_ACCEPT от '" + identity.GetName() + "'.");
			return;
		}

		if (request.param1 != JobsModRPC.PROTOCOL_VERSION)
		{
			Reject(player, identity, JobsModRejectReason.PROTOCOL_MISMATCH);
			return;
		}

		string npcId = request.param2;
		string jobId = request.param3;
		string playerId = identity.GetId();

		if (!IsPlayerReady(player))
		{
			Reject(player, identity, JobsModRejectReason.PLAYER_NOT_READY);
			return;
		}

		JobsModNpcJson npc = m_Config.GetNpc(npcId);
		if (!npc)
		{
			Reject(player, identity, JobsModRejectReason.UNKNOWN_NPC);
			return;
		}

		// The id came from the client, so being at the NPC is verified against
		// where that NPC actually stands, not against what the client claims.
		if (!m_Npcs.IsPlayerAtNpc(player, npcId))
		{
			Reject(player, identity, JobsModRejectReason.TOO_FAR);
			return;
		}

		JobsModJobJson job = m_Config.GetJob(jobId);
		if (!job)
		{
			Reject(player, identity, JobsModRejectReason.UNKNOWN_JOB);
			return;
		}

		// Being able to name a job is not the same as this employer offering it.
		if (npc.jobs.Find(jobId) < 0)
		{
			Reject(player, identity, JobsModRejectReason.NPC_DOES_NOT_OFFER);
			return;
		}

		if (GetAssignment(playerId))
		{
			Reject(player, identity, JobsModRejectReason.JOB_ALREADY_HELD);
			return;
		}

		if (GetCooldownRemaining(playerId, jobId) > 0)
		{
			Reject(player, identity, JobsModRejectReason.ON_COOLDOWN);
			return;
		}

		m_NextAssignmentId++;
		JobsModAssignment assignment = new JobsModAssignment(m_NextAssignmentId, playerId, npcId, job);

		// Freight has to exist before the player is told the job started,
		// otherwise they walk to an empty yard and the job looks broken.
		if (assignment.GetType() == JobsModJobType.LOADING)
		{
			if (!m_Loader.SpawnFor(assignment, job))
			{
				Reject(player, identity, JobsModRejectReason.UNKNOWN_JOB);
				JobsLog.Error("SERVER/JOBS: груз для работы '" + jobId + "' не создан, работа не выдана.");
				return;
			}
		}

		m_Assignments.Set(playerId, assignment);

		SendState(player, identity);
		SendMessage(player, identity, "РАБОТА ПРИНЯТА", job.name + ". " + job.description);

		JobsLog.Info("SERVER/JOBS: работа '" + jobId + "' выдана игроку '" + identity.GetName()
			+ "' от NPC '" + npcId + "'; id=" + assignment.GetId().ToString() + ".");
	}

	// =====================================================================
	// Handing a job in
	// =====================================================================
	void HandleComplete(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		Param3<int, string, int> request = new Param3<int, string, int>(0, "", 0);
		if (!ctx.Read(request))
		{
			JobsLog.Warning("SERVER/RPC: не удалось прочитать REQUEST_JOB_COMPLETE от '" + identity.GetName() + "'.");
			return;
		}

		if (request.param1 != JobsModRPC.PROTOCOL_VERSION)
		{
			Reject(player, identity, JobsModRejectReason.PROTOCOL_MISMATCH);
			return;
		}

		string playerId = identity.GetId();
		JobsModAssignment assignment = GetAssignment(playerId);

		if (!assignment)
		{
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_JOB);
			return;
		}

		// A stale menu can send an id the player no longer holds. Paying on it
		// would pay for a job that was already closed.
		if (assignment.GetId() != request.param3)
		{
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_JOB);
			return;
		}

		if (!assignment.IsFinished())
		{
			Reject(player, identity, JobsModRejectReason.JOB_NOT_FINISHED);
			return;
		}

		if (assignment.GetNpcId() != request.param2)
		{
			Reject(player, identity, JobsModRejectReason.WRONG_NPC);
			return;
		}

		if (!m_Npcs.IsPlayerAtNpc(player, assignment.GetNpcId()))
		{
			Reject(player, identity, JobsModRejectReason.TOO_FAR);
			return;
		}

		JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
		if (!job)
		{
			// The job was edited out of the config while it was being worked.
			// Closing it out is the only honest option; refusing would leave the
			// player stuck holding an assignment nothing can finish.
			JobsLog.Warning("SERVER/JOBS: работа '" + assignment.GetJobId()
				+ "' исчезла из конфига, задание закрыто без оплаты.");
			EndAssignment(player, identity, assignment, "");
			return;
		}

		GrantReward(player, identity, job);
		StartCooldown(playerId, job);
		EndAssignment(player, identity, assignment, "");

		JobsLog.Info("SERVER/JOBS: работа '" + job.id + "' принята у '" + identity.GetName()
			+ "'; выплачено " + job.reward.ToString() + ".");
	}

	void HandleAbandon(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		Param2<int, int> request = new Param2<int, int>(0, 0);
		if (!ctx.Read(request) || request.param1 != JobsModRPC.PROTOCOL_VERSION)
			return;

		JobsModAssignment assignment = GetAssignment(identity.GetId());
		if (!assignment || assignment.GetId() != request.param2)
			return;

		EndAssignment(player, identity, assignment, "Работа брошена.");
		JobsLog.Info("SERVER/JOBS: '" + identity.GetName() + "' отказался от работы '" + assignment.GetJobId() + "'.");
	}

	// =====================================================================
	// Progress, reported by the services that actually watched the work
	// =====================================================================
	// Called after a pile has been sorted and accepted. Returns true when that
	// pile was the last one the job needed.
	bool ReportSortedPile(PlayerBase player, PlayerIdentity identity, JobsModAssignment assignment)
	{
		bool finished = assignment.AddProgress();
		AnnounceProgress(player, identity, assignment, finished);
		return finished;
	}

	// Called after a box has been left in the drop-off area.
	bool ReportDeliveredCargo(PlayerBase player, PlayerIdentity identity, JobsModAssignment assignment)
	{
		bool finished = assignment.AddProgress();

		if (finished)
		{
			// Nothing is gained by leaving the rest of the freight standing once
			// the count is reached, and a yard full of leftovers looks like the
			// job never ended.
			assignment.DeleteAllCargo();
		}

		AnnounceProgress(player, identity, assignment, finished);
		return finished;
	}

	protected void AnnounceProgress(PlayerBase player, PlayerIdentity identity, JobsModAssignment assignment, bool finished)
	{
		SendState(player, identity);

		if (!finished)
			return;

		JobsModNpcJson npc = m_Config.GetNpc(assignment.GetNpcId());
		string npcName = assignment.GetNpcId();
		if (npc)
			npcName = npc.name;

		SendMessage(player, identity, "РАБОТА ВЫПОЛНЕНА", "Вернитесь к нанимателю: " + npcName + ".");
	}

	// =====================================================================
	// Lifecycle
	// =====================================================================
	// Ends the assignment and cleans up everything it put in the world. Used by
	// hand-in, abandon, disconnect and timeout alike, so no path can forget the
	// freight.
	protected void EndAssignment(PlayerBase player, PlayerIdentity identity, JobsModAssignment assignment, string note)
	{
		assignment.DeleteAllCargo();
		m_Assignments.Remove(assignment.GetPlayerId());

		if (player && identity)
		{
			SendState(player, identity);

			if (note != "")
				SendMessage(player, identity, "РАБОТА ЗАКРЫТА", note);
		}
	}

	// A disconnecting player's job is dropped at once. Keeping it would leave
	// their freight standing in the world for anyone to find, and reconnecting
	// would run into their own stale assignment.
	void DropPlayer(string playerId)
	{
		JobsModAssignment assignment = GetAssignment(playerId);
		if (!assignment)
			return;

		assignment.DeleteAllCargo();
		m_Assignments.Remove(playerId);
		JobsLog.Debug("SERVER/JOBS: задание игрока " + playerId + " снято при выходе.");
	}

	// Drops jobs nobody is coming back to, so neither the map nor the world
	// grows without bound over a long uptime.
	void Update()
	{
		int timeout = m_Config.GetAssignmentTimeoutSeconds();
		array<string> stale = new array<string>();

		int i;
		for (i = 0; i < m_Assignments.Count(); i++)
		{
			JobsModAssignment assignment = m_Assignments.GetElement(i);
			if (assignment && assignment.IsExpired(timeout))
				stale.Insert(m_Assignments.GetKey(i));
		}

		for (i = 0; i < stale.Count(); i++)
		{
			string playerId = stale.Get(i);
			JobsModAssignment expired = GetAssignment(playerId);

			PlayerBase player = FindPlayerById(playerId);
			PlayerIdentity identity = null;
			if (player)
				identity = player.GetIdentity();

			EndAssignment(player, identity, expired, "Срок выполнения истёк.");
			JobsLog.Info("SERVER/JOBS: просроченное задание снято; игрок=" + playerId + ".");
		}
	}

	int GetActiveCount()
	{
		return m_Assignments.Count();
	}

	// =====================================================================
	// Reward and cooldown
	// =====================================================================
	protected void GrantReward(PlayerBase player, PlayerIdentity identity, JobsModJobJson job)
	{
		if (job.reward <= 0)
			return;

		EntityAI reward = player.GetInventory().CreateInInventory(REWARD_CLASS);

		// A full inventory must not swallow the pay: fall back to the ground at
		// the player's feet rather than silently dropping the reward.
		if (!reward)
		{
			Object spawned = GetGame().CreateObjectEx(REWARD_CLASS, player.GetPosition(), ECE_PLACE_ON_SURFACE);
			reward = EntityAI.Cast(spawned);
		}

		if (!reward)
		{
			JobsLog.Error("SERVER/JOBS: награду '" + REWARD_CLASS + "' выдать не удалось.");
			return;
		}

		ItemBase rewardItem = ItemBase.Cast(reward);
		if (rewardItem)
			rewardItem.SetQuantity(job.reward);

		SendMessage(player, identity, "РАСЧЁТ ПОЛУЧЕН",
			job.name + ". Начислено: " + job.reward.ToString() + ".");
	}

	protected void StartCooldown(string playerId, JobsModJobJson job)
	{
		if (job.cooldown_seconds <= 0)
			return;

		m_CooldownUntilMs.Set(CooldownKey(playerId, job.id), GetGame().GetTime() + job.cooldown_seconds * 1000);
	}

	// Seconds still owed before this player may take this job again.
	int GetCooldownRemaining(string playerId, string jobId)
	{
		int until;
		if (!m_CooldownUntilMs.Find(CooldownKey(playerId, jobId), until))
			return 0;

		int remaining = (until - GetGame().GetTime()) / 1000;
		if (remaining <= 0)
			return 0;

		return remaining;
	}

	protected string CooldownKey(string playerId, string jobId)
	{
		return playerId + "|" + jobId;
	}

	// =====================================================================
	// Outgoing
	// =====================================================================
	// The whole HUD state in one message. Sending everything every time keeps
	// the client from having to reconstruct anything, and means a lost message
	// is repaired by the next one instead of leaving the HUD wrong forever.
	void SendState(PlayerBase player, PlayerIdentity identity)
	{
		if (!player || !identity)
			return;

		JobsModAssignment assignment = GetAssignment(identity.GetId());

		int status = JobsModJobStatus.NONE;
		int progress = 0;
		int required = 0;
		int assignmentId = 0;
		string jobName = "";
		string zoneName = "";
		string npcName = "";
		string hint = "";

		if (assignment)
		{
			status = assignment.GetStatus();
			progress = assignment.GetProgress();
			required = assignment.GetRequired();
			assignmentId = assignment.GetId();
			zoneName = m_Config.GetZoneName(assignment.GetZoneId());

			JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
			if (job)
				jobName = job.name;

			JobsModNpcJson npc = m_Config.GetNpc(assignment.GetNpcId());
			if (npc)
				npcName = npc.name;

			if (assignment.IsFinished())
				hint = "Вернитесь к нанимателю: " + npcName + ".";
			else if (assignment.GetType() == JobsModJobType.LOADING)
				hint = "Отнесите ящики в зону разгрузки.";
			else
				hint = "Разберите мусор в зоне: " + zoneName + ".";
		}

		array<ref Param> message = new array<ref Param>();
		message.Insert(new Param4<int, int, int, int>(status, progress, required, assignmentId));
		message.Insert(new Param4<string, string, string, string>(jobName, zoneName, npcName, hint));

		GetGame().RPC(player, JobsModRPC.NOTIFY_JOB_STATE, message, true, identity);
	}

	protected void SendMessage(PlayerBase player, PlayerIdentity identity, string title, string text)
	{
		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_JOB_MESSAGE,
			new Param2<string, string>(title, text),
			true,
			identity);
	}

	protected void Reject(PlayerBase player, PlayerIdentity identity, int reason)
	{
		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_REJECTED,
			new Param1<int>(reason),
			true,
			identity);

		JobsLog.Debug("SERVER/JOBS: отказ игроку '" + identity.GetName() + "'; причина=" + reason.ToString() + ".");
	}

	// =====================================================================
	// Helpers
	// =====================================================================
	protected bool IsPlayerReady(PlayerBase player)
	{
		return player.IsAlive() && !player.IsUnconscious() && !player.IsRestrained();
	}

	protected PlayerBase FindPlayerById(string playerId)
	{
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);

		for (int i = 0; i < players.Count(); i++)
		{
			PlayerBase player = PlayerBase.Cast(players.Get(i));
			if (!player || !player.GetIdentity())
				continue;

			if (player.GetIdentity().GetId() == playerId)
				return player;
		}

		return null;
	}
}
