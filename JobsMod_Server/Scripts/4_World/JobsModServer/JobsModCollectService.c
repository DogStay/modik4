// JobsModCollectService.c
//
// Fetch work: bring the employer a number of something. Thirty steaks, ten
// planks, whatever the config names.
//
// The mod supplies nothing for this job and spawns nothing. Where the goods
// come from is the player's problem — hunting, looting, trading, a stash they
// already had — and that is the point of it: it is the one job that pays for
// something a player would have been doing anyway.
//
// Counting is by whole items, not by quantity. In DayZ a food item's quantity
// is its weight in grams, so counting quantity would turn "thirty steaks" into
// nine thousand of something, and a job that reads clearly in the config has to
// count the way the person reading it would.
//
// Progress is recomputed from the inventory rather than accumulated, because
// items can leave it again. A player who gathers twenty-nine and eats one is at
// twenty-eight, and the HUD says so; nothing here can be banked by picking a
// thing up and dropping it.
//
// The goods are taken at hand-in and not a moment earlier. Counting them while
// the player walks would let the same steak satisfy two contracts.

class JobsModCollectService
{
	protected ref JobsModConfig m_Config;
	protected JobsModJobService m_Jobs;

	void JobsModCollectService(JobsModConfig config)
	{
		m_Config = config;
	}

	void SetJobService(JobsModJobService jobs)
	{
		m_Jobs = jobs;
	}

	// =====================================================================
	// Counting
	// =====================================================================
	// Everything the player is carrying, including the insides of containers,
	// because a backpack full of meat is still meat the player has.
	int Count(PlayerBase player, JobsModJobJson job)
	{
		if (!player || !player.GetInventory() || !job || !job.collect_classes)
			return 0;

		array<EntityAI> carried = new array<EntityAI>();
		if (!player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried))
			return 0;

		int found = 0;

		for (int i = 0; i < carried.Count(); i++)
		{
			EntityAI item = carried.Get(i);
			if (!item)
				continue;

			if (Accepts(job, item.GetType()))
				found++;
		}

		return found;
	}

	// A job may name several classes so that "any meat" is one contract rather
	// than one per animal.
	protected bool Accepts(JobsModJobJson job, string className)
	{
		for (int i = 0; i < job.collect_classes.Count(); i++)
		{
			if (job.collect_classes.Get(i) == className)
				return true;
		}

		return false;
	}

	// =====================================================================
	// The running job
	// =====================================================================
	// Driven from the job service's slow tick. Every collect assignment is
	// re-counted, and the client is told only when the number actually moved —
	// the HUD is a screenful of widgets and there is no reason to redraw it
	// every few seconds for a player who is walking somewhere.
	void Update(map<string, ref JobsModAssignment> assignments)
	{
		if (!assignments)
			return;

		for (int i = 0; i < assignments.Count(); i++)
		{
			JobsModAssignment assignment = assignments.GetElement(i);
			if (!assignment || assignment.GetType() != JobsModJobType.COLLECT)
				continue;

			JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
			if (!job)
				continue;

			PlayerBase player = FindPlayer(assignments.GetKey(i));
			if (!player || !player.GetIdentity())
				continue;

			int held = Count(player, job);
			if (held == assignment.GetProgress())
				continue;

			assignment.SetProgressTo(held);

			if (m_Jobs)
				m_Jobs.SendState(player, player.GetIdentity());
		}
	}

	// =====================================================================
	// Handing the goods over
	// =====================================================================
	// Called at hand-in, after the player has been checked to be standing at
	// the right NPC. Removes exactly what the job asks for and leaves anything
	// beyond it alone: a player who brought thirty-five steaks keeps five.
	bool TakeGoods(PlayerBase player, JobsModJobJson job)
	{
		if (!player || !player.GetInventory() || !job)
			return false;

		if (Count(player, job) < job.collect_required)
			return false;

		array<EntityAI> carried = new array<EntityAI>();
		if (!player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried))
			return false;

		int taken = 0;

		for (int i = 0; i < carried.Count() && taken < job.collect_required; i++)
		{
			EntityAI item = carried.Get(i);
			if (!item || !Accepts(job, item.GetType()))
				continue;

			GetGame().ObjectDelete(item);
			taken++;
		}

		JobsLog.Debug("SERVER/COLLECT: принято предметов: " + taken.ToString() + " по работе '" + job.id + "'.");
		return taken >= job.collect_required;
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
