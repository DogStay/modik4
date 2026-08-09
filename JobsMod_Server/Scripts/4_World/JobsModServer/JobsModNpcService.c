// JobsModNpcService.c
//
// Puts the employers in the world and answers "which NPC is this object?".
//
// An NPC is an ordinary survivor entity of whatever class the admin named. The
// mod deliberately does not ship a script class of its own for it: a script
// class would need a config class behind it, and that would fix the model to
// whatever we picked instead of letting the config choose. Invulnerability is
// set on the instance instead, which works for any class an admin writes.
//
// Which object is an NPC is decided here and nowhere else. The talk action only
// reports the object the player interacted with; this service is what turns it
// into an npc id, so a player interacting with an ordinary survivor — or with
// another player — resolves to nothing and the request dies.

class JobsModNpcRecord
{
	string m_Id;
	ref JobsModNpcJson m_Definition;
	PlayerBase m_Entity;

	void JobsModNpcRecord(JobsModNpcJson definition)
	{
		m_Id = definition.id;
		m_Definition = definition;
		m_Entity = null;
	}
}

class JobsModNpcService
{
	protected ref JobsModConfig m_Config;
	protected ref array<ref JobsModNpcRecord> m_Records;

	void JobsModNpcService(JobsModConfig config)
	{
		m_Config = config;
		m_Records = new array<ref JobsModNpcRecord>();
	}

	// =====================================================================
	// Spawning
	// =====================================================================
	void SpawnAll()
	{
		map<string, ref JobsModNpcJson> definitions = m_Config.GetNpcs();
		int spawned = 0;

		for (int i = 0; i < definitions.Count(); i++)
		{
			JobsModNpcRecord record = new JobsModNpcRecord(definitions.GetElement(i));
			m_Records.Insert(record);

			if (Respawn(record))
				spawned++;
		}

		JobsLog.Info("SERVER/NPC: создано NPC: " + spawned.ToString() + " из " + m_Records.Count().ToString() + ".");

		if (spawned == 0 && m_Records.Count() > 0)
			JobsLog.Error("SERVER/NPC: ни один NPC не создан — проверьте player_class и координаты.");
	}

	protected bool Respawn(JobsModNpcRecord record)
	{
		JobsModNpcJson definition = record.m_Definition;

		vector position = definition.GetPosition();

		// A height at or below zero means the config was written without one.
		// Snapping to the terrain is the only sane reading of that: a survivor
		// spawned under the ground never becomes reachable.
		if (position[1] <= 0)
			position[1] = GetGame().SurfaceY(position[0], position[2]);

		// init_ai is what makes a survivor class come up standing and animated
		// instead of an inert prop, so it is not optional here.
		Object created = GetGame().CreateObject(definition.player_class, position, false, true, true);
		PlayerBase entity = PlayerBase.Cast(created);

		if (!entity)
		{
			if (created)
				GetGame().ObjectDelete(created);

			JobsLog.Error("SERVER/NPC: класс '" + definition.player_class + "' для NPC '" + definition.id + "' не создан или не является персонажем.");
			return false;
		}

		entity.SetPosition(position);
		entity.SetOrientation(Vector(definition.rotation, 0, 0));

		if (definition.invulnerable)
			entity.SetAllowDamage(false);

		Dress(entity, definition);

		record.m_Entity = entity;
		JobsLog.Debug("SERVER/NPC: '" + definition.id + "' создан на " + position.ToString() + ".");
		return true;
	}

	// Clothing goes in as plain inventory creation: the engine puts each piece
	// in the slot its config declares. An item that does not fit anywhere is
	// reported and skipped rather than left lying in the NPC's hands.
	protected void Dress(PlayerBase entity, JobsModNpcJson definition)
	{
		for (int i = 0; i < definition.clothing.Count(); i++)
		{
			string className = definition.clothing.Get(i);
			if (className == "")
				continue;

			EntityAI piece = entity.GetInventory().CreateInInventory(className);
			if (!piece)
			{
				JobsLog.Warning("SERVER/NPC: NPC '" + definition.id + "' не удалось надеть '" + className + "' — проверьте имя класса.");
			}
		}
	}

	// =====================================================================
	// Lookup
	// =====================================================================
	// The authoritative answer to "did this player interact with an employer?".
	string GetNpcIdByObject(Object object)
	{
		if (!object)
			return "";

		for (int i = 0; i < m_Records.Count(); i++)
		{
			JobsModNpcRecord record = m_Records.Get(i);
			if (record.m_Entity && record.m_Entity == object)
				return record.m_Id;
		}

		return "";
	}

	PlayerBase GetNpcEntity(string npcId)
	{
		JobsModNpcRecord record = FindRecord(npcId);
		if (!record)
			return null;

		return record.m_Entity;
	}

	// Used for every request that names an npc id instead of pointing at the
	// object: the id may be anything the client felt like sending, so being at
	// the NPC is checked against where the NPC actually stands.
	bool IsPlayerAtNpc(PlayerBase player, string npcId)
	{
		PlayerBase entity = GetNpcEntity(npcId);
		if (!entity || !player)
			return false;

		return vector.Distance(player.GetPosition(), entity.GetPosition()) <= JobsModRPC.NPC_INTERACTION_DISTANCE;
	}

	protected JobsModNpcRecord FindRecord(string npcId)
	{
		for (int i = 0; i < m_Records.Count(); i++)
		{
			if (m_Records.Get(i).m_Id == npcId)
				return m_Records.Get(i);
		}

		return null;
	}

	// =====================================================================
	// Upkeep
	// =====================================================================
	// An NPC that is gone — deleted by an admin tool, or lost with the chunk it
	// stood in — is put back. Nothing else here is periodic: they do not move.
	void Update()
	{
		for (int i = 0; i < m_Records.Count(); i++)
		{
			JobsModNpcRecord record = m_Records.Get(i);
			if (!NeedsRespawn(record))
				continue;

			if (record.m_Entity)
			{
				GetGame().ObjectDelete(record.m_Entity);
				record.m_Entity = null;
			}

			if (Respawn(record))
				JobsLog.Warning("SERVER/NPC: NPC '" + record.m_Id + "' пропал и создан заново.");
		}
	}

	// A deleted entity nulls this reference by itself, and that is the only
	// signal used for an invulnerable NPC.
	//
	// IsAlive() is deliberately not consulted for those: if a freshly spawned
	// survivor ever failed that test, an invulnerable NPC would be torn down and
	// rebuilt every fifteen seconds forever, and the failure would look like the
	// mod flickering NPCs rather than like a bad test. A mortal NPC can actually
	// die, so there the check is both meaningful and self-limiting.
	protected bool NeedsRespawn(JobsModNpcRecord record)
	{
		if (!record.m_Entity)
			return true;

		if (record.m_Definition.invulnerable)
			return false;

		return !record.m_Entity.IsAlive();
	}

	void DeleteAll()
	{
		for (int i = 0; i < m_Records.Count(); i++)
		{
			JobsModNpcRecord record = m_Records.Get(i);
			if (record.m_Entity)
			{
				GetGame().ObjectDelete(record.m_Entity);
				record.m_Entity = null;
			}
		}

		m_Records.Clear();
		JobsLog.Info("SERVER/NPC: все NPC удалены.");
	}

	// =====================================================================
	// Client directory
	// =====================================================================
	// The client cannot tell an employer from any other survivor: which ones are
	// NPCs lives in a server-only config. It is told where they stand so it can
	// offer the talk action on them; that is presentation only, and the server
	// still resolves the object itself when the action fires.
	void SendDirectory(PlayerBase player, PlayerIdentity identity)
	{
		if (!player || !identity)
			return;

		array<ref Param> message = new array<ref Param>();
		array<ref Param> entries = new array<ref Param>();

		for (int i = 0; i < m_Records.Count(); i++)
		{
			JobsModNpcRecord record = m_Records.Get(i);
			if (!record.m_Entity)
				continue;

			entries.Insert(new Param2<string, vector>(record.m_Definition.name, record.m_Entity.GetPosition()));
		}

		message.Insert(new Param1<int>(entries.Count()));

		for (int e = 0; e < entries.Count(); e++)
			message.Insert(entries.Get(e));

		GetGame().RPC(player, JobsModRPC.NOTIFY_NPC_DIRECTORY, message, true, identity);
		JobsLog.Debug("SERVER/NPC: справочник из " + entries.Count().ToString() + " NPC отправлен '" + identity.GetName() + "'.");
	}
}
