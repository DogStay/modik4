// JobsModGuardService.c
//
// Guard duty: hand over a kit, then pay for time spent standing on a post.
//
// This is the only job where the work is measured in seconds, and that changes
// what the server has to be careful about. Everything else advances because the
// player did something the server witnessed — sorted a pile, put a box down,
// handed a parcel over. Time passes whether anyone is looking, so the clock has
// to be driven from the server's own tick and read from the player's real
// position, never from anything the client says.
//
// The clock only runs while the player is inside the post. Stepping out stops
// it and stepping back in resumes it: the shift is not cancelled, because a
// guard walking a few metres to look at something is doing the job, and losing
// twenty minutes for it would make the work unplayable. What it does not do is
// advance, so the post cannot be left and the pay still collected.

class JobsModGuardService
{
	protected ref JobsModConfig m_Config;
	protected JobsModJobService m_Jobs;

	// The server tick is coarse, so progress is credited in whole seconds and
	// the remainder of each tick is kept rather than thrown away.
	protected int m_LastTickMs;

	void JobsModGuardService(JobsModConfig config)
	{
		m_Config = config;
		m_LastTickMs = 0;
	}

	void SetJobService(JobsModJobService jobs)
	{
		m_Jobs = jobs;
	}

	// =====================================================================
	// Starting a shift
	// =====================================================================
	// The kit goes into the assignment's own list, which every path that ends a
	// job already deletes. A shift that ends by hand-in, abandon, disconnect or
	// timeout therefore takes the equipment back without this service being
	// asked, and a player cannot farm kit by taking the contract repeatedly.
	//
	// A piece that does not fit is not fatal on its own — the player may simply
	// be carrying a lot — but a kit that fits nowhere at all means the job would
	// start without the equipment it is defined by, so that is refused.
	bool IssueKit(PlayerBase player, JobsModAssignment assignment, JobsModJobJson job)
	{
		if (!player || !player.GetInventory() || !assignment || !job)
			return false;

		if (!job.equipment || job.equipment.Count() == 0)
			return true;

		int issued = 0;

		for (int i = 0; i < job.equipment.Count(); i++)
		{
			string className = job.equipment.Get(i);
			if (className == "")
				continue;

			EntityAI piece = player.GetInventory().CreateInInventory(className);
			if (!piece)
			{
				JobsLog.Warning("SERVER/GUARD: '" + className + "' не выдан игроку — нет места или неверный класс.");
				continue;
			}

			assignment.TrackCargo(piece);
			issued++;
		}

		if (issued == 0)
		{
			JobsLog.Warning("SERVER/GUARD: ни один предмет снаряжения не выдан для работы '" + job.id + "'.");
			return false;
		}

		JobsLog.Debug("SERVER/GUARD: выдано предметов снаряжения: " + issued.ToString() + " для задания " + assignment.GetId().ToString() + ".");
		return true;
	}

	// =====================================================================
	// The shift itself
	// =====================================================================
	// Called from the job service's slow tick. Every guard assignment is
	// checked against where its player actually stands.
	void Update(map<string, ref JobsModAssignment> assignments)
	{
		if (!assignments)
			return;

		int now = GetGame().GetTime();

		// First call: take the current time as the baseline rather than
		// crediting the whole uptime to whoever is on duty.
		if (m_LastTickMs == 0)
		{
			m_LastTickMs = now;
			return;
		}

		int elapsed = (now - m_LastTickMs) / 1000;
		if (elapsed < 1)
			return;

		m_LastTickMs = m_LastTickMs + elapsed * 1000;

		for (int i = 0; i < assignments.Count(); i++)
		{
			JobsModAssignment assignment = assignments.GetElement(i);
			if (!assignment || assignment.GetType() != JobsModJobType.GUARD || !assignment.IsActive())
				continue;

			TickOne(assignments.GetKey(i), assignment, elapsed);
		}
	}

	protected void TickOne(string playerId, JobsModAssignment assignment, int elapsed)
	{
		JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
		if (!job)
			return;

		JobsModGuardPostJson post = m_Config.GetGuardPost(job.guard_post_id);
		if (!post)
			return;

		PlayerBase player = FindPlayer(playerId);
		if (!player || !player.GetIdentity())
			return;

		if (!IsOnPost(player, post))
			return;

		bool finished = assignment.AddProgressBy(elapsed);

		// The HUD is redrawn from the job state, so it is sent on the same tick
		// the seconds are credited. Without it the countdown would only move
		// when something else happened to send state.
		if (m_Jobs)
			m_Jobs.SendState(player, player.GetIdentity());

		if (!finished)
			return;

		JobsLog.Info("SERVER/GUARD: смена отстояна; задание " + assignment.GetId().ToString() + ".");

		if (m_Jobs)
		{
			JobsModNpcJson npc = m_Config.GetNpc(assignment.GetHandInNpcId());
			string npcName = assignment.GetHandInNpcId();
			if (npc)
				npcName = npc.name;

			m_Jobs.SendMessage(player, player.GetIdentity(), "СМЕНА ОКОНЧЕНА", "Пост можно покинуть. Вернитесь за оплатой: " + npcName + ".");
		}
	}

	// Compared in 2D. A post is a place on the ground, and a guard standing on
	// a step or a floor above is still at their post.
	bool IsOnPost(PlayerBase player, JobsModGuardPostJson post)
	{
		if (!player || !post)
			return false;

		vector centre = post.GetPosition();
		vector here = player.GetPosition();

		float dx = here[0] - centre[0];
		float dz = here[2] - centre[2];

		return (dx * dx + dz * dz) <= (post.radius * post.radius);
	}

	protected PlayerBase FindPlayer(string playerId)
	{
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);

		for (int i = 0; i < players.Count(); i++)
		{
			PlayerBase player = PlayerBase.Cast(players.Get(i));
			if (player && player.GetIdentity() && player.GetIdentity().GetId() == playerId)
				return player;
		}

		return null;
	}
}
