// JobsModMessengerService.c
//
// The courier job: one sealed parcel, handed over at one end and taken at the
// other by somebody else.
//
// What makes this job different from the other two is that the thing being
// worked on lives in a player's inventory rather than in the world. That is
// also the only hard problem in it. A trash pile cannot follow a player through
// a disconnect; a parcel can, and a parcel that survives one is a duplicate
// waiting to be carried into the next job.
//
// So the parcel is pinned down from two sides:
//
//   * JobsMod_Parcel refuses to be dropped, taken into hands, moved between
//     containers or picked up off the ground. That is what the player runs
//     into, and it is enforced on the client and the server alike because the
//     inventory checks it in both places.
//
//   * This service sweeps, every upkeep tick, and settles the difference
//     between what the assignments say and what players are actually carrying.
//     A parcel with no job behind it is deleted; a job whose parcel has gone is
//     closed. Nothing here trusts the first layer to have held.
//
// The second sweep is what makes the awkward cases fall out instead of needing
// their own code. A player who died with a parcel, one who logged out and back
// in with it, one who was handed it by a mod that ignores inventory rules —
// they all end up in the same place: carrying a parcel their assignment does
// not know about, which is deleted on the next tick.

class JobsModMessengerService
{
	protected ref JobsModConfig m_Config;
	protected JobsModJobService m_Jobs;

	void JobsModMessengerService(JobsModConfig config)
	{
		m_Config = config;
	}

	// Wired after construction for the same reason the loader is: the job
	// service is built from this one and cannot exist yet.
	void SetJobService(JobsModJobService jobs)
	{
		m_Jobs = jobs;
	}

	// =====================================================================
	// Handing the parcel over
	// =====================================================================
	// Puts the parcel in the player's inventory and gives it to the assignment
	// to own. Returns false when there was nowhere to put it, which is a real
	// and common outcome — a full inventory — and not an error.
	//
	// There is deliberately no fallback to the ground. The reward falls back
	// that way because pay that vanishes is worse than pay in the dirt, but a
	// parcel on the ground is the exact situation this whole job is built to
	// prevent: refusing the job is the honest answer.
	bool GiveParcel(PlayerBase player, JobsModAssignment assignment, JobsModJobJson job)
	{
		if (!player || !player.GetInventory() || !assignment || !job)
			return false;

		EntityAI parcel = player.GetInventory().CreateInInventory(job.package_class);
		if (!parcel)
		{
			JobsLog.Debug("SERVER/COURIER: пакет '" + job.package_class + "' не помещён в инвентарь — нет места или неверный класс.");
			return false;
		}

		// Into the assignment's own list, which is what every path that ends an
		// assignment already deletes. The parcel therefore cannot outlive the
		// job through hand-in, abandon, disconnect or timeout without this
		// service being involved at all.
		assignment.TrackCargo(parcel);
		return true;
	}

	// Whether this assignment's parcel is still where it belongs. Asked before
	// paying for a delivery: the parcel is the delivery, and a player standing
	// at the recipient without it has not made one.
	bool HasParcel(PlayerBase player, JobsModAssignment assignment)
	{
		if (!player || !assignment)
			return false;

		return IsCarriedBy(assignment.GetParcel(), player);
	}

	// The parcel's root has to be this player. Being inside a worn backpack is
	// still on the player; being inside the same backpack after it was dropped
	// is not, and that is the one way a parcel can leave without the inventory
	// rules ever being asked.
	protected bool IsCarriedBy(EntityAI parcel, PlayerBase player)
	{
		if (!parcel || !player)
			return false;

		// Cast rather than compare the Man the engine hands back directly:
		// comparing across the two types is exactly the sort of thing that
		// compiles differently depending on the toolchain.
		PlayerBase root = PlayerBase.Cast(parcel.GetHierarchyRootPlayer());
		return root == player;
	}

	// =====================================================================
	// Upkeep
	// =====================================================================
	// Runs on the slow timer. Nothing here needs to be quick: the delivery
	// itself happens through a menu, not through proximity, so a parcel that is
	// wrong for a few seconds costs nothing.
	void Update()
	{
		if (!m_Jobs)
			return;

		CloseJobsWithoutParcel();
		DeleteOrphanParcels();
	}

	// A courier who no longer has the parcel cannot finish, and leaving the
	// assignment open would leave them walking to a recipient who will refuse
	// them. Closing it says so at once and frees them to take work again.
	protected void CloseJobsWithoutParcel()
	{
		array<JobsModAssignment> assignments = new array<JobsModAssignment>();
		m_Jobs.CollectAssignments(assignments);

		for (int i = 0; i < assignments.Count(); i++)
		{
			JobsModAssignment assignment = assignments.Get(i);

			if (assignment.GetType() != JobsModJobType.MESSENGER || !assignment.IsActive())
				continue;

			// Offline players are not judged. Disconnecting already ends the
			// assignment, so anyone still holding one here is mid-reconnect and
			// their inventory is not a thing to read yet.
			PlayerBase player = m_Jobs.FindPlayerById(assignment.GetPlayerId());
			if (!player)
				continue;

			if (HasParcel(player, assignment))
				continue;

			JobsLog.Info("SERVER/COURIER: пакет задания " + assignment.GetId().ToString() + " потерян, работа закрыта.");

			// The assignment is destroyed by this call and must not be touched
			// again; the loop moves on to the next index and never does.
			m_Jobs.FailAssignment(player, assignment, "Пакет утерян. Работа закрыта.");
		}
	}

	// Deletes every parcel that no current job accounts for.
	//
	// This is the half that does not care how the parcel got there. A player
	// carrying one without an active courier job should not be carrying one at
	// all, whatever happened — the job ended, they reconnected onto an old
	// character, they took it off a body before the rules could stop them.
	//
	// It recognises JobsMod_Parcel and what inherits from it, which is what the
	// mod hands out. A server that points package_class at some other item gets
	// the assignment's own cleanup and nothing more, because there would be no
	// way to tell that item apart from the same item a player owns legitimately.
	protected void DeleteOrphanParcels()
	{
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);

		for (int i = 0; i < players.Count(); i++)
		{
			// A player mid-respawn can be in the list before their inventory
			// exists, and reading it there is a null access rather than an
			// empty answer.
			PlayerBase player = PlayerBase.Cast(players.Get(i));
			if (!player || !player.GetIdentity() || !player.GetInventory())
				continue;

			// Not named 'owned': that is an Enforce keyword and the declaration
			// will not parse.
			EntityAI ownedParcel = FindOwnedParcel(player);

			array<EntityAI> carried = new array<EntityAI>();
			if (!player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried))
				continue;

			for (int c = 0; c < carried.Count(); c++)
			{
				EntityAI item = carried.Get(c);

				if (!item || item == ownedParcel)
					continue;

				if (!item.IsInherited(JobsMod_Parcel))
					continue;

				GetGame().ObjectDelete(item);
				JobsLog.Warning("SERVER/COURIER: у игрока '" + player.GetIdentity().GetName() + "' найден пакет без задания, удалён.");
			}
		}
	}

	// The one parcel this player is entitled to, or null. Everything else they
	// are carrying is an orphan by definition.
	protected EntityAI FindOwnedParcel(PlayerBase player)
	{
		JobsModAssignment assignment = m_Jobs.GetAssignment(player.GetIdentity().GetId());

		if (!assignment || assignment.GetType() != JobsModJobType.MESSENGER || !assignment.IsActive())
			return null;

		return assignment.GetParcel();
	}
}
