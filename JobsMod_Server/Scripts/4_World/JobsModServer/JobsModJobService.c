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


	protected ref JobsModConfig m_Config;
	protected JobsModNpcService m_Npcs;
	protected JobsModLoaderService m_Loader;
	protected JobsModMessengerService m_Messenger;
	protected TrashZoneService m_Zones;
	protected JobsModGuardService m_Guard;
	protected JobsModCollectService m_Collect;

	// One assignment per player, keyed by identity id.
	protected ref map<string, ref JobsModAssignment> m_Assignments;
	// "<playerId>|<jobId>" -> server time the cooldown runs out at.
	protected ref map<string, int> m_CooldownUntilMs;

	protected int m_NextAssignmentId;

	void JobsModJobService(JobsModConfig config, JobsModNpcService npcs, JobsModLoaderService loader,
		JobsModMessengerService messenger, TrashZoneService zones, JobsModGuardService guard,
		JobsModCollectService collect)
	{
		m_Guard = guard;
		m_Collect = collect;
		m_Config = config;
		m_Npcs = npcs;
		m_Loader = loader;
		m_Messenger = messenger;
		m_Zones = zones;
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

		// The hand-in button belongs to whoever signs the job off — the employer
		// for most jobs, the addressee for a courier's — so it is only offered
		// here when this is that person and the work is actually done.
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

			if (held.GetHandInNpcId() == npcId && IsHandInAllowed(held))
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
		Param2<string, string> request = new Param2<string, string>("", "");
		if (!ctx.Read(request))
		{
			JobsLog.Warning("SERVER/RPC: не удалось прочитать REQUEST_JOB_ACCEPT от '" + identity.GetName() + "'.");
			return;
		}

		string npcId = request.param1;
		string jobId = request.param2;
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

		// Same reasoning for the parcel, with one difference that matters: it
		// can fail simply because the player is carrying too much. That is not
		// a broken config, so the refusal has to say what it actually is —
		// otherwise the player is told the job does not exist while standing in
		// front of the person offering it.
		if (assignment.GetType() == JobsModJobType.MESSENGER)
		{
			if (!m_Messenger.GiveParcel(player, assignment, job))
			{
				Reject(player, identity, JobsModRejectReason.NO_INVENTORY_SPACE);
				return;
			}
		}

		// The kit is what the guard contract is: refusing the job when none of
		// it fits is better than sending someone to stand a shift unequipped.
		if (assignment.GetType() == JobsModJobType.GUARD)
		{
			if (!m_Guard.IssueKit(player, assignment, job))
			{
				Reject(player, identity, JobsModRejectReason.NO_INVENTORY_SPACE);
				return;
			}
		}

		m_Assignments.Set(playerId, assignment);

		SendState(player, identity);
		SendMessage(player, identity, "РАБОТА ПРИНЯТА", job.name + ". " + job.description);

		JobsLog.Info("SERVER/JOBS: работа '" + jobId + "' выдана игроку '" + identity.GetName() + "' от NPC '" + npcId + "'; id=" + assignment.GetId().ToString() + ".");
	}

	// =====================================================================
	// Handing a job in
	// =====================================================================
	void HandleComplete(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		Param2<string, int> request = new Param2<string, int>("", 0);
		if (!ctx.Read(request))
		{
			JobsLog.Warning("SERVER/RPC: не удалось прочитать REQUEST_JOB_COMPLETE от '" + identity.GetName() + "'.");
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
		if (assignment.GetId() != request.param2)
		{
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_JOB);
			return;
		}

		if (!IsHandInAllowed(assignment))
		{
			Reject(player, identity, JobsModRejectReason.JOB_NOT_FINISHED);
			return;
		}

		if (assignment.GetHandInNpcId() != request.param1)
		{
			Reject(player, identity, JobsModRejectReason.WRONG_NPC);
			return;
		}

		if (!m_Npcs.IsPlayerAtNpc(player, assignment.GetHandInNpcId()))
		{
			Reject(player, identity, JobsModRejectReason.TOO_FAR);
			return;
		}

		// The parcel is the delivery, so it is checked here and not earlier:
		// the player may have been carrying it a second ago and lost it to a
		// mod, a container or a death that has not been swept up yet. Paying
		// for a delivery that did not arrive is the one thing this job must not
		// do.
		if (assignment.GetType() == JobsModJobType.MESSENGER && !m_Messenger.HasParcel(player, assignment))
		{
			Reject(player, identity, JobsModRejectReason.PARCEL_MISSING);
			return;
		}

		JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
		if (!job)
		{
			// The job was edited out of the config while it was being worked.
			// Closing it out is the only honest option; refusing would leave the
			// player stuck holding an assignment nothing can finish.
			JobsLog.Warning("SERVER/JOBS: работа '" + assignment.GetJobId() + "' исчезла из конфига, задание закрыто без оплаты.");
			EndAssignment(player, identity, assignment, "");
			return;
		}

		// The goods are taken here and nowhere earlier. Counting them while the
		// player walked would let the same thirty steaks satisfy two contracts,
		// and taking them early would rob anyone who then walked away.
		if (assignment.GetType() == JobsModJobType.COLLECT && !m_Collect.TakeGoods(player, job))
		{
			Reject(player, identity, JobsModRejectReason.JOB_NOT_FINISHED);
			return;
		}

		// Read out before ending it: the map holds the only strong reference to
		// the assignment, so it is gone by the time the log line is built.
		string issuedBy = assignment.GetNpcId();
		string signedBy = assignment.GetHandInNpcId();

		GrantReward(player, identity, job);
		StartCooldown(playerId, job);
		EndAssignment(player, identity, assignment, "");

		// Both ends are logged because a courier job has two of them, and which
		// pair of NPCs a delivery actually ran between is the first thing worth
		// knowing when a route looks wrong.
		JobsLog.Info("SERVER/JOBS: работа '" + job.id + "' принята у '" + identity.GetName() + "'; выдал '" + issuedBy + "', принял '" + signedBy + "', выплачено " + job.reward.ToString() + ".");
	}

	void HandleAbandon(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		Param1<int> request = new Param1<int>(0);
		if (!ctx.Read(request))
			return;

		JobsModAssignment assignment = GetAssignment(identity.GetId());
		if (!assignment || assignment.GetId() != request.param1)
			return;

		// Read out before ending it. The map holds the only strong reference, so
		// dropping it there destroys the assignment and leaves this local
		// pointing at nothing.
		string jobId = assignment.GetJobId();

		EndAssignment(player, identity, assignment, "Работа брошена.");
		JobsLog.Info("SERVER/JOBS: '" + identity.GetName() + "' отказался от работы '" + jobId + "'.");
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

		JobsModNpcJson npc = m_Config.GetNpc(assignment.GetHandInNpcId());
		string npcName = assignment.GetHandInNpcId();
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
	//
	// The caller's reference to the assignment is dead once this returns: the
	// map held the only strong one. Anything still needed from it has to be read
	// out first.
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

	// Whether the job can be signed off right now.
	//
	// For work that is counted — piles, boxes — the count has to be complete,
	// and the count is what turns the assignment finished. A courier is not
	// counted: arriving with the parcel is the whole job, so the hand-in is the
	// delivery rather than a receipt for one, and the checks that matter
	// (the right person, the parcel still in hand) are made by the caller.
	protected bool IsHandInAllowed(JobsModAssignment assignment)
	{
		if (assignment.GetType() == JobsModJobType.MESSENGER)
			return assignment.IsActive();

		return assignment.IsFinished();
	}

	// Ends an assignment that cannot be completed any more, with no pay and no
	// cooldown: nothing was earned, and nothing was farmed either, so making
	// the player wait before trying again would only punish them for it.
	//
	// The assignment is destroyed by this call — see EndAssignment.
	void FailAssignment(PlayerBase player, JobsModAssignment assignment, string note)
	{
		if (!assignment)
			return;

		PlayerIdentity identity = null;
		if (player)
			identity = player.GetIdentity();

		EndAssignment(player, identity, assignment, note);
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

	// Dying ends the job, whatever kind it was.
	//
	// It has to, for the courier: the parcel would otherwise be lying in a body
	// for anyone to loot, and the job it belonged to would still be open for a
	// player who is about to respawn on the coast without it. The other jobs
	// are ended for the same reason a disconnect ends them — a corpse is not
	// going to finish carrying the boxes.
	//
	// The player object still exists at this point and is told, so the HUD
	// clears instead of following them to the respawn screen.
	void HandlePlayerDeath(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		JobsModAssignment assignment = GetAssignment(player.GetIdentity().GetId());
		if (!assignment)
			return;

		string jobId = assignment.GetJobId();

		EndAssignment(player, player.GetIdentity(), assignment, "Вы погибли. Работа закрыта.");
		JobsLog.Info("SERVER/JOBS: работа '" + jobId + "' закрыта смертью игрока '" + player.GetIdentity().GetName() + "'.");
	}

	// Drops jobs nobody is coming back to, so neither the map nor the world
	// grows without bound over a long uptime.
	void Update()
	{
		// Before the expiry sweep: a shift that just finished must not be taken
		// off the player in the same tick for running out of time.
		if (m_Guard)
			m_Guard.Update(m_Assignments);

		if (m_Collect)
			m_Collect.Update(m_Assignments);

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

	// The job stores a class list; the player needs a name.
	protected string GetCollectLabel(JobsModAssignment assignment)
	{
		JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
		if (job && job.collect_label != "")
			return job.collect_label;

		return assignment.GetJobId();
	}

	// Seconds are what the job counts in, but "1200" tells a player nothing.
	// Kept to integer arithmetic: a float through ToString() prints six decimals.
	protected string FormatRemaining(JobsModAssignment assignment)
	{
		int left = assignment.GetRequired() - assignment.GetProgress();
		if (left < 0)
			left = 0;

		if (left >= 60)
		{
			int minutes = left / 60;
			return "осталось " + minutes.ToString() + " мин";
		}

		return "осталось " + left.ToString() + " с";
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

		string rewardClass = m_Config.GetRewardClass();
		EntityAI reward = player.GetInventory().CreateInInventory(rewardClass);

		// A full inventory must not swallow the pay: fall back to the ground at
		// the player's feet rather than silently dropping the reward.
		if (!reward)
		{
			Object spawned = GetGame().CreateObjectEx(rewardClass, player.GetPosition(), ECE_PLACE_ON_SURFACE);
			reward = EntityAI.Cast(spawned);
		}

		if (!reward)
		{
			JobsLog.Error("SERVER/JOBS: награду '" + rewardClass + "' выдать не удалось — проверьте reward_class в settings.json.");
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
		int type = JobsModJobType.UNKNOWN;
		int progress = 0;
		int required = 0;
		int assignmentId = 0;
		string jobName = "";
		string zoneName = "";
		string npcName = "";
		string hint = "";
		string cargoClass = "";

		array<ref Param> markers = new array<ref Param>();

		if (assignment)
		{
			status = assignment.GetStatus();
			type = assignment.GetType();
			progress = assignment.GetProgress();
			required = assignment.GetRequired();
			assignmentId = assignment.GetId();
			zoneName = m_Config.GetZoneName(assignment.GetZoneId());

			JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
			if (job)
			{
				jobName = job.name;
				cargoClass = job.cargo_class;
			}

			// The person the player has to walk to, which for a courier is the
			// addressee and not the employer. Every line below reads better for
			// it: there is never a moment in any job where the name of somebody
			// the player is not going to see is the useful one.
			JobsModNpcJson npc = m_Config.GetNpc(assignment.GetHandInNpcId());
			if (npc)
				npcName = npc.name;

			if (assignment.GetType() == JobsModJobType.MESSENGER)
				hint = "Доставьте пакет: " + npcName + " (" + zoneName + ").";
			else if (assignment.IsFinished())
				hint = "Вернитесь к нанимателю: " + npcName + ".";
			else if (assignment.GetType() == JobsModJobType.LOADING)
				hint = "Отнесите ящики в зону разгрузки.";
			else if (assignment.GetType() == JobsModJobType.GUARD)
				hint = "Оставайтесь на посту: " + FormatRemaining(assignment) + ".";
			else if (assignment.GetType() == JobsModJobType.COLLECT)
				hint = "Соберите: " + GetCollectLabel(assignment) + ".";
			else
				hint = "Разберите мусор в зоне: " + zoneName + ".";

			CollectMarkers(assignment, npcName, markers);
		}

		array<ref Param> message = new array<ref Param>();
		message.Insert(new Param4<int, int, int, int>(status, progress, required, assignmentId));
		message.Insert(new Param4<string, string, string, string>(jobName, zoneName, npcName, hint));
		message.Insert(new Param3<string, int, int>(cargoClass, type, markers.Count()));

		for (int i = 0; i < markers.Count(); i++)
			message.Insert(markers.Get(i));

		GetGame().RPC(player, JobsModRPC.NOTIFY_JOB_STATE, message, true, identity);
	}

	// Where the player has to go, sent with every state update because the
	// client has no way to know any of it: piles, yards and employers all live
	// in a config only the server ever reads.
	//
	// Everything relevant is sent at once and the client picks. That is what
	// keeps the marker correct between messages — sorting one pile does not
	// require a fresh list to point at the next one, and picking up a box does
	// not require a round trip to start pointing at the drop-off.
	protected void CollectMarkers(JobsModAssignment assignment, string npcName, out array<ref Param> markers)
	{
		// Once the work is done the only thing left is the walk back, so the
		// employer replaces the work points rather than joining them.
		if (assignment.IsFinished())
		{
			PlayerBase npcEntity = m_Npcs.GetNpcEntity(assignment.GetHandInNpcId());
			if (npcEntity)
				markers.Insert(new Param3<int, string, vector>(JobsModMarkerKind.EMPLOYER, npcName, npcEntity.GetPosition()));

			return;
		}

		// A guard has one place to be for the whole shift, and it is a configured
		// circle rather than a person, so the centre is what the marker gets.
		if (assignment.GetType() == JobsModJobType.GUARD)
		{
			JobsModJobJson guardJob = m_Config.GetJob(assignment.GetJobId());
			if (guardJob)
			{
				JobsModGuardPostJson post = m_Config.GetGuardPost(guardJob.guard_post_id);
				if (post)
				{
					vector centre = post.GetPosition();
					markers.Insert(new Param3<int, string, vector>(
						JobsModMarkerKind.DESTINATION, post.name, Vector(centre[0], 0, centre[2])));
				}
			}
		}

		// A courier has one place to be from the moment the job starts, and the
		// addressee's real position is sent rather than a configured point: an
		// NPC that had to be respawned is not necessarily standing where the
		// config says any more, and the marker has to point at the person.
		if (assignment.GetType() == JobsModJobType.MESSENGER)
		{
			PlayerBase recipient = m_Npcs.GetNpcEntity(assignment.GetHandInNpcId());
			if (recipient)
				markers.Insert(new Param3<int, string, vector>(JobsModMarkerKind.DESTINATION, npcName, recipient.GetPosition()));

			return;
		}

		if (assignment.GetType() == JobsModJobType.LOADING)
		{
			JobsModLoaderAreaJson area = m_Config.GetLoaderArea(assignment.GetLoaderAreaId());
			if (!area)
				return;

			// Height is dropped on purpose: the config carries a circle on the
			// map, and the client looks the ground up under it. Sending the
			// height an admin happened to paste would float the marker at
			// whatever altitude they were standing at.
			vector source = area.GetSource();
			vector destination = area.GetDestination();

			markers.Insert(new Param3<int, string, vector>(
				JobsModMarkerKind.SOURCE, "Погрузка", Vector(source[0], 0, source[2])));
			markers.Insert(new Param3<int, string, vector>(
				JobsModMarkerKind.DESTINATION, "Разгрузка", Vector(destination[0], 0, destination[2])));
			return;
		}

		array<vector> piles = new array<vector>();
		m_Zones.CollectStandingPiles(assignment.GetZoneId(), piles);

		for (int i = 0; i < piles.Count(); i++)
			markers.Insert(new Param3<int, string, vector>(JobsModMarkerKind.TARGET, "Мусор", piles.Get(i)));
	}

	// Public because the guard service announces the end of a shift, which is
	// the one moment a job finishes without the player having done anything the
	// job service could have witnessed.
	void SendMessage(PlayerBase player, PlayerIdentity identity, string title, string text)
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

	// Public because the loader and the courier both have to go from an
	// assignment back to the player working it, and one lookup is better than
	// three copies of it.
	PlayerBase FindPlayerById(string playerId)
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
