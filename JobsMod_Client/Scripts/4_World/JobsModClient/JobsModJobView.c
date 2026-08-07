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

	void JobsModJobView()
	{
		m_Status = JobsModJobStatus.NONE;
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
}
