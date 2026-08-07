// JobsModJobView.c
//
// What the client knows about jobs: the one it is holding, and the offers of
// the NPC it is currently standing in front of.
//
// These are pure carriers filled from RPC payloads. Nothing here decides
// anything — the server has already decided all of it, and every field is
// display material. Keeping them free of logic is what makes it safe for the
// HUD and the menu to read them straight.

// One line in an NPC's offer list.
class JobsModJobOffer
{
	string m_JobId;
	string m_Name;
	string m_Description;
	string m_ZoneName;
	int m_Reward;
	int m_Required;
	int m_CooldownRemaining;
	int m_Availability;

	void JobsModJobOffer(string jobId, string name, string description, string zoneName,
		int reward, int required, int cooldownRemaining, int availability)
	{
		m_JobId = jobId;
		m_Name = name;
		m_Description = description;
		m_ZoneName = zoneName;
		m_Reward = reward;
		m_Required = required;
		m_CooldownRemaining = cooldownRemaining;
		m_Availability = availability;
	}

	bool IsSelectable()
	{
		return m_Availability == JobsModOfferState.AVAILABLE;
	}

	// The one line under the job name that says why it can or cannot be taken.
	string GetStatusText()
	{
		switch (m_Availability)
		{
			case JobsModOfferState.HELD:
				return "Вы уже выполняете эту работу";
			case JobsModOfferState.BLOCKED_BY_OTHER:
				return "Сначала закончите текущую работу";
			case JobsModOfferState.ON_COOLDOWN:
				return "Перерыв: " + m_CooldownRemaining.ToString() + " с";
		}

		return "Оплата: " + m_Reward.ToString() + "   Объём: " + m_Required.ToString();
	}
}

// Everything one NPC menu shows.
class JobsModNpcOffer
{
	string m_NpcId;
	string m_NpcName;
	string m_NpcDescription;
	ref array<ref JobsModJobOffer> m_Offers;

	// Set when this NPC is the one that issued the finished job the player is
	// carrying, which is the only case where the hand-in button does anything.
	bool m_HandInAvailable;
	int m_AssignmentId;
	string m_HeldJobName;
	int m_HeldStatus;

	void JobsModNpcOffer()
	{
		m_Offers = new array<ref JobsModJobOffer>();
		m_HandInAvailable = false;
		m_AssignmentId = 0;
		m_HeldStatus = JobsModJobStatus.NONE;
	}
}

// One place in the world worth running to. Positions come from the server —
// the client has no idea where a pile or a loading yard is otherwise, since
// none of that is in a config it ever downloads.
class JobsModMarker
{
	int m_Kind;
	string m_Label;
	vector m_Position;

	void JobsModMarker(int kind, string label, vector position)
	{
		m_Kind = kind;
		m_Label = label;
		m_Position = position;
	}
}

// The job the player is holding, as the HUD draws it.
class JobsModJobView
{
	int m_Status;
	int m_Progress;
	int m_Required;
	int m_AssignmentId;
	string m_JobName;
	string m_ZoneName;
	string m_NpcName;
	string m_Hint;

	// Class name of this job's freight, so the marker can tell "go and fetch a
	// box" from "carry this one to the yard" by looking at the player's hands.
	// Empty for jobs that have no freight.
	string m_CargoClass;

	ref array<ref JobsModMarker> m_Markers;

	void JobsModJobView()
	{
		m_Status = JobsModJobStatus.NONE;
		m_Markers = new array<ref JobsModMarker>();
	}

	bool HasJob()
	{
		return m_Status != JobsModJobStatus.NONE;
	}

	float GetFraction()
	{
		if (m_Required <= 0)
			return 0.0;

		// Both fields are ints and integer division would collapse every
		// unfinished job to a zero-width bar.
		float done = m_Progress;
		float total = m_Required;
		return Math.Clamp(done / total, 0.0, 1.0);
	}

	// Which of the points the server sent the player should be running to right
	// now. Relevance is decided first and distance only breaks the tie: the
	// loading yard may well be closer than the unloading yard while you are
	// carrying a box, and pointing at it would be worse than pointing nowhere.
	JobsModMarker PickMarker(vector playerPosition, bool carryingCargo)
	{
		int wanted = JobsModMarkerKind.TARGET;

		if (m_Status == JobsModJobStatus.READY_TO_HAND_IN)
		{
			wanted = JobsModMarkerKind.EMPLOYER;
		}
		else if (m_CargoClass != "")
		{
			if (carryingCargo)
				wanted = JobsModMarkerKind.DESTINATION;
			else
				wanted = JobsModMarkerKind.SOURCE;
		}

		JobsModMarker best = null;
		float bestDistance = 0;

		for (int i = 0; i < m_Markers.Count(); i++)
		{
			JobsModMarker marker = m_Markers.Get(i);
			if (marker.m_Kind != wanted)
				continue;

			float distance = vector.Distance(playerPosition, marker.m_Position);
			if (best && distance >= bestDistance)
				continue;

			best = marker;
			bestDistance = distance;
		}

		return best;
	}
}
