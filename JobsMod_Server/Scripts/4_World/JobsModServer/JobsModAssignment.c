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
	protected string m_NpcId;
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

	void JobsModAssignment(int id, string playerId, string npcId, JobsModJobJson job)
	{
		m_Id = id;
		m_PlayerId = playerId;
		m_NpcId = npcId;
		m_JobId = job.id;
		m_Type = JobsModJobType.FromText(job.type);
		m_ZoneId = job.zone_id;
		m_LoaderAreaId = job.loader_area_id;

		if (m_Type == JobsModJobType.LOADING)
			m_Required = job.cargos_required;
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
	string GetJobId() { return m_JobId; }
	int GetType() { return m_Type; }
	string GetZoneId() { return m_ZoneId; }
	string GetLoaderAreaId() { return m_LoaderAreaId; }
	int GetRequired() { return m_Required; }
	int GetProgress() { return m_Progress; }
	int GetStatus() { return m_Status; }
	array<EntityAI> GetCargo() { return m_Cargo; }

	bool IsActive() { return m_Status == JobsModJobStatus.ACTIVE; }
	bool IsFinished() { return m_Status == JobsModJobStatus.READY_TO_HAND_IN; }

	// Returns true when this step was the last one. Progress never runs past the
	// requirement, so a duplicate report cannot pay twice.
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
		for (int i = 0; i < m_Cargo.Count(); i++)
		{
			EntityAI cargo = m_Cargo.Get(i);
			if (cargo)
				GetGame().ObjectDelete(cargo);
		}

		m_Cargo.Clear();
	}
}
