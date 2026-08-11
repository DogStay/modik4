// JobsModAssignment.c
//
// One job a player is currently holding. There is at most one per player, which
// is what makes "can I take this job?" and "does this pile count?" single
// lookups instead of searches.
//
// It is created by the server when an NPC hands the job out and destroyed when
// the NPC signs it off, when the player gives up, when they disconnect, or when
// it times out. Nothing about it is ever read from a client payload: the client
// is told what its assignment is, never the other way round.

class JobsModAssignment
{
	protected int m_Id;
	protected string m_PlayerId;

	// Who handed the job out. Kept as provenance, not as routing: where the
	// player has to walk is m_HandInNpcId below, and for a courier the two are
	// different people.
	protected string m_NpcId;

	// Who signs the job off. The same person that handed it out for every job
	// but the courier's, where the parcel is taken — and paid for — by the NPC
	// it is addressed to. Resolved once here so no code downstream has to ask
	// what kind of job it is before it knows where the player has to walk.
	protected string m_HandInNpcId;

	protected string m_JobId;
	protected int m_Type;
	protected string m_ZoneId;
	protected string m_LoaderAreaId;

	protected int m_Required;
	protected int m_Progress;
	protected int m_Status;
	protected int m_StartedAtMs;

	// Freight spawned for this assignment. Held here rather than in a separate
	// table so ending the assignment can never leave boxes behind: whatever is
	// in this list is what gets deleted.
	protected ref array<EntityAI> m_Cargo;

	// Only meaningful for a job with a locker. Without one the kit is handed
	// over with the job and there is nothing to draw or return.
	protected bool m_KitIssued;
	protected bool m_KitReturned;

	void JobsModAssignment(int id, string playerId, string npcId, JobsModJobJson job)
	{
		m_Id = id;
		m_PlayerId = playerId;
		m_NpcId = npcId;
		m_JobId = job.id;
		m_Type = JobsModJobType.FromText(job.type);
		m_ZoneId = job.zone_id;
		m_LoaderAreaId = job.loader_area_id;

		m_HandInNpcId = npcId;
		if (m_Type == JobsModJobType.MESSENGER)
			m_HandInNpcId = job.target_npc_id;

		if (m_Type == JobsModJobType.LOADING)
			m_Required = job.cargos_required;
		else if (m_Type == JobsModJobType.MESSENGER)
			m_Required = 1;
		else
			m_Required = job.piles_required;

		m_Progress = 0;
		m_Status = JobsModJobStatus.ACTIVE;
		m_StartedAtMs = GetGame().GetTime();
		m_Cargo = new array<EntityAI>();
	}

	int GetId() { return m_Id; }
	string GetPlayerId() { return m_PlayerId; }
	string GetNpcId() { return m_NpcId; }
	string GetHandInNpcId() { return m_HandInNpcId; }
	string GetJobId() { return m_JobId; }
	int GetType() { return m_Type; }
	string GetZoneId() { return m_ZoneId; }
	string GetLoaderAreaId() { return m_LoaderAreaId; }
	int GetRequired() { return m_Required; }
	int GetProgress() { return m_Progress; }
	int GetStatus() { return m_Status; }
	array<EntityAI> GetCargo() { return m_Cargo; }

	bool IsKitIssued() { return m_KitIssued; }
	bool IsKitReturned() { return m_KitReturned; }
	void SetKitIssued(bool value) { m_KitIssued = value; }
	void SetKitReturned(bool value) { m_KitReturned = value; }

	bool IsActive() { return m_Status == JobsModJobStatus.ACTIVE; }
	bool IsFinished() { return m_Status == JobsModJobStatus.READY_TO_HAND_IN; }

	// Returns true when this step was the last one. Progress never runs past the
	// requirement, so a duplicate report cannot pay twice.
	// Collect work is recounted from the inventory rather than accumulated, so
	// it needs to be able to go down as well as up: goods can be eaten, dropped
	// or stashed after being gathered. The status follows the number both ways,
	// which is what stops a player reaching the total once and then handing in
	// an empty bag.
	void SetProgressTo(int value)
	{
		if (value < 0)
			value = 0;

		if (value > m_Required)
			value = m_Required;

		m_Progress = value;

		if (m_Progress >= m_Required)
			m_Status = JobsModJobStatus.READY_TO_HAND_IN;
		else
			m_Status = JobsModJobStatus.ACTIVE;
	}

	// Guard duty advances in whole seconds at a time rather than one unit per
	// event, so the step is a parameter. Everything else adds one.
	bool AddProgressBy(int amount)
	{
		if (amount < 1)
			return false;

		for (int i = 0; i < amount; i++)
		{
			if (AddProgress())
				return true;
		}

		return false;
	}

	bool AddProgress()
	{
		if (m_Status != JobsModJobStatus.ACTIVE)
			return false;

		m_Progress++;

		if (m_Progress < m_Required)
			return false;

		m_Progress = m_Required;
		m_Status = JobsModJobStatus.READY_TO_HAND_IN;
		return true;
	}

	int GetElapsedSeconds()
	{
		return (GetGame().GetTime() - m_StartedAtMs) / 1000;
	}

	bool IsExpired(int timeoutSeconds)
	{
		return GetElapsedSeconds() >= timeoutSeconds;
	}

	void TrackCargo(EntityAI cargo)
	{
		m_Cargo.Insert(cargo);
	}

	// The courier's parcel. It is kept in the same list the freight is, and for
	// the same reason: whatever ends this assignment deletes that list, so
	// there is no path that can end a courier job and leave the parcel in an
	// inventory. A courier assignment has exactly one entry in it.
	EntityAI GetParcel()
	{
		if (m_Cargo.Count() == 0)
			return null;

		return m_Cargo.Get(0);
	}

	bool OwnsCargo(EntityAI cargo)
	{
		return m_Cargo.Find(cargo) >= 0;
	}

	void ForgetCargo(EntityAI cargo)
	{
		int index = m_Cargo.Find(cargo);
		if (index >= 0)
			m_Cargo.Remove(index);
	}

	// Takes every box this assignment put in the world back out of it, wherever
	// it ended up — on the ground, in a player's hands or in a tent.
	void DeleteAllCargo()
	{
		int removed = 0;

		for (int i = 0; i < m_Cargo.Count(); i++)
		{
			EntityAI cargo = m_Cargo.Get(i);
			if (!cargo)
				continue;

			// Delete() on the entity rather than ObjectDelete() on the world
			// object: contract gear is normally worn or held, so what has to go
			// is the inventory item, and the entity knows how to take itself out
			// of whatever slot it is sitting in.
			cargo.Delete();
			removed++;
		}

		JobsLog.Debug("SERVER/JOBS: снято предметов задания: " + removed.ToString() + " из " + m_Cargo.Count().ToString() + ".");
		m_Cargo.Clear();
	}
}
