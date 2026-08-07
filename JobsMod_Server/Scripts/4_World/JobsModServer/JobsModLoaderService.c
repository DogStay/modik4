// JobsModLoaderService.c
//
// The freight job: boxes appear in the loading yard, the player carries them to
// the unloading yard one at a time, and a box left standing there is done.
//
// There is no "pick up" and no "put down" action, on purpose. The box is an
// ordinary heavy item, so taking it into your hands and dropping it are the
// game's own controls, with the game's own two-handed carry. The mod only has
// to notice a box that is standing on the ground inside the drop-off circle —
// which is exactly what the job is asking for — and take it away.
//
// That also removes a whole class of problems by not creating them: nothing to
// desynchronise between a client-side "carrying" flag and the inventory, no
// box stuck in a state after a disconnect, no way to be holding a box the
// server thinks is still on the ground.

class JobsModLoaderService
{
	// How often the drop-off circles are swept. Freight is carried at walking
	// pace, so anything faster only costs work.
	static const float SWEEP_INTERVAL_SECONDS = 2.0;

	// Attempts to find a free spot for one box before giving up on it.
	protected static const int PLACEMENT_ATTEMPTS = 10;
	protected static const float MIN_CARGO_SPACING = 1.5;

	protected ref JobsModConfig m_Config;
	protected JobsModJobService m_Jobs;

	void JobsModLoaderService(JobsModConfig config)
	{
		m_Config = config;
	}

	// Wired after construction: the two services need each other, and the job
	// service is the one that must exist first.
	void SetJobService(JobsModJobService jobs)
	{
		m_Jobs = jobs;
	}

	// =====================================================================
	// Spawning the freight
	// =====================================================================
	// Boxes belong to one assignment. Two players working the same route each
	// get their own, so neither can take work off the other's pile and neither
	// is left waiting for boxes somebody else is carrying.
	bool SpawnFor(JobsModAssignment assignment, JobsModJobJson job)
	{
		JobsModLoaderAreaJson area = m_Config.GetLoaderArea(job.loader_area_id);
		if (!area)
		{
			JobsLog.Error("SERVER/LOADER: маршрут '" + job.loader_area_id + "' не найден.");
			return false;
		}

		int spawned = 0;

		for (int i = 0; i < job.cargos_required; i++)
		{
			vector position;
			if (!FindPlacement(area.source, assignment, position))
			{
				JobsLog.Warning("SERVER/LOADER: не нашлось места для ящика в зоне погрузки '" + area.id + "'.");
				continue;
			}

			Object created = GetGame().CreateObjectEx(job.cargo_class, position, ECE_PLACE_ON_SURFACE);
			EntityAI cargo = EntityAI.Cast(created);

			if (!cargo)
			{
				if (created)
					GetGame().ObjectDelete(created);

				JobsLog.Error("SERVER/LOADER: класс груза '" + job.cargo_class + "' не создан.");
				break;
			}

			assignment.TrackCargo(cargo);
			spawned++;
		}

		if (spawned < job.cargos_required)
		{
			JobsLog.Warning("SERVER/LOADER: создано " + spawned.ToString() + " из "
				+ job.cargos_required.ToString() + " ящиков для '" + job.id + "'.");
		}

		// Fewer boxes than the job asks for would make it impossible to finish,
		// so an assignment that could not be fully stocked is refused outright.
		if (spawned < job.cargos_required)
		{
			assignment.DeleteAllCargo();
			return false;
		}

		JobsLog.Debug("SERVER/LOADER: создано ящиков: " + spawned.ToString()
			+ " для задания " + assignment.GetId().ToString() + ".");
		return true;
	}

	protected bool FindPlacement(JobsModAreaJson area, JobsModAssignment assignment, out vector position)
	{
		for (int attempt = 0; attempt < PLACEMENT_ATTEMPTS; attempt++)
		{
			float angle = Math.RandomFloat(0.0, Math.PI2);
			float distance = Math.RandomFloat(0.0, area.radius);

			float x = area.x + Math.Cos(angle) * distance;
			float z = area.z + Math.Sin(angle) * distance;
			vector candidate = Vector(x, GetGame().SurfaceY(x, z), z);

			if (IsClearOfOtherCargo(candidate, assignment))
			{
				position = candidate;
				return true;
			}
		}

		return false;
	}

	protected bool IsClearOfOtherCargo(vector candidate, JobsModAssignment assignment)
	{
		array<EntityAI> cargo = assignment.GetCargo();

		for (int i = 0; i < cargo.Count(); i++)
		{
			EntityAI box = cargo.Get(i);
			if (!box)
				continue;

			if (vector.Distance(box.GetPosition(), candidate) < MIN_CARGO_SPACING)
				return false;
		}

		return true;
	}

	// =====================================================================
	// Delivery
	// =====================================================================
	// Sweeps every loading job for boxes that are standing free inside their
	// drop-off circle. A box in someone's hands or in a container has a parent
	// in the hierarchy and is skipped, so carrying one through the yard does not
	// count — it has to be put down.
	void Update()
	{
		if (!m_Jobs)
			return;

		array<JobsModAssignment> assignments = new array<JobsModAssignment>();
		m_Jobs.CollectAssignments(assignments);

		for (int i = 0; i < assignments.Count(); i++)
		{
			JobsModAssignment assignment = assignments.Get(i);

			if (assignment.GetType() != JobsModJobType.LOADING || !assignment.IsActive())
				continue;

			SweepAssignment(assignment);
		}
	}

	protected void SweepAssignment(JobsModAssignment assignment)
	{
		JobsModLoaderAreaJson area = m_Config.GetLoaderArea(assignment.GetLoaderAreaId());
		if (!area)
			return;

		PlayerBase player = FindPlayer(assignment.GetPlayerId());
		if (!player || !player.GetIdentity())
			return;

		array<EntityAI> cargo = assignment.GetCargo();

		// Walked backwards because a delivered box is removed from this list.
		for (int i = cargo.Count() - 1; i >= 0; i--)
		{
			EntityAI box = cargo.Get(i);

			// The engine nulls the slot when a box is destroyed by something
			// else — a fire, a vehicle, a cleanup pass. Stop tracking it, but do
			// not credit it: it never reached the yard.
			if (!box)
			{
				cargo.Remove(i);
				continue;
			}

			if (box.GetHierarchyParent())
				continue;

			if (!IsInsideArea(box.GetPosition(), area.destination))
				continue;

			assignment.ForgetCargo(box);
			GetGame().ObjectDelete(box);

			bool finished = m_Jobs.ReportDeliveredCargo(player, player.GetIdentity(), assignment);

			JobsLog.Debug("SERVER/LOADER: ящик сдан; задание " + assignment.GetId().ToString()
				+ ", прогресс " + assignment.GetProgress().ToString() + "/" + assignment.GetRequired().ToString() + ".");

			if (finished)
				return;
		}
	}

	// Height is ignored: the yard is a circle on the map, and a box on a ramp or
	// a pier inside it has still arrived.
	bool IsInsideArea(vector position, JobsModAreaJson area)
	{
		float dx = position[0] - area.x;
		float dz = position[2] - area.z;
		return (dx * dx + dz * dz) <= (area.radius * area.radius);
	}

	protected PlayerBase FindPlayer(string playerId)
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
