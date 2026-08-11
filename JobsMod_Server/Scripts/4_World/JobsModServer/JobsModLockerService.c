// JobsModLockerService.c
//
// The equipment cupboard: where a contract kit is drawn and where it goes back.
//
// It exists because taking the gear off a player automatically was both
// unreliable and rude. A locker makes the exchange something the player does,
// which means they can see it happen, and it gives the mod a place to stand
// when the gear does not come back — the fine.
//
// One action does all three jobs. Which one it is depends on state only the
// server has: whether a kit was drawn, whether it is still complete, what a
// replacement costs. Splitting it into three actions would mean shipping that
// state to every client near every cupboard, so the client sends "used this
// locker" and reads the answer as a notification.
//
// A job with no locker configured keeps the old behaviour exactly: the
// employer hands the kit over with the work and takes it back when the work
// ends. Lockers are opt-in, per job.

class JobsModLockerService
{
	protected static const string DEFAULT_LOCKER_CLASS = "Wardrobe";

	protected ref JobsModConfig m_Config;
	protected JobsModJobService m_Jobs;
	protected JobsModGuardService m_Guard;

	// Every locker this service placed, by the object it created. A cupboard
	// the service did not place is refused, so a player cannot carry a wardrobe
	// to a quiet corner and mint kits out of it.
	protected ref map<Object, ref JobsModLockerJson> m_Placed;

	void JobsModLockerService(JobsModConfig config)
	{
		m_Config = config;
		m_Placed = new map<Object, ref JobsModLockerJson>();
	}

	void SetServices(JobsModJobService jobs, JobsModGuardService guard)
	{
		m_Jobs = jobs;
		m_Guard = guard;
	}

	// =====================================================================
	// Placing
	// =====================================================================
	void SpawnAll()
	{
		array<ref JobsModLockerJson> lockers = m_Config.GetLockers();
		int spawned = 0;

		for (int i = 0; i < lockers.Count(); i++)
		{
			if (Place(lockers.Get(i)))
				spawned++;
		}

		if (lockers.Count() > 0)
			JobsLog.Info("SERVER/LOCKER: установлено шкафчиков: " + spawned.ToString() + " из " + lockers.Count().ToString() + ".");
	}

	protected bool Place(JobsModLockerJson locker)
	{
		vector position = locker.GetPosition();

		if (position[1] <= 0)
			position[1] = GetGame().SurfaceY(position[0], position[2]);

		string className = locker.class_name;
		if (className == "")
			className = DEFAULT_LOCKER_CLASS;

		Object created = GetGame().CreateObjectEx(className, position, ECE_PLACE_ON_SURFACE);
		ItemBase entity = ItemBase.Cast(created);

		if (!entity)
		{
			if (created)
				GetGame().ObjectDelete(created);

			JobsLog.Error("SERVER/LOCKER: класс '" + className + "' для шкафчика '" + locker.id + "' не создан или не является предметом.");
			return false;
		}

		// Flat, like the freight boxes, and for the same reason.
		entity.SetOrientation(Vector(locker.rotation, 0.0, 0.0));
		entity.SetPosition(position);

		// What makes the action appear on this cupboard and on no other.
		entity.JobsModSetLocker(true);

		m_Placed.Set(entity, locker);

		JobsLog.Debug("SERVER/LOCKER: шкафчик '" + locker.id + "' установлен на " + position.ToString() + ".");
		return true;
	}

	void DeleteAll()
	{
		for (int i = 0; i < m_Placed.Count(); i++)
		{
			Object locker = m_Placed.GetKey(i);
			if (locker)
				GetGame().ObjectDelete(locker);
		}

		m_Placed.Clear();
	}

	// =====================================================================
	// Using one
	// =====================================================================
	void HandleUse(PlayerBase player, Object locker)
	{
		if (!GetGame().IsServer() || !player || !locker || !m_Jobs)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		JobsModLockerJson placed;
		if (!m_Placed.Find(locker, placed))
			return;

		JobsModAssignment assignment = m_Jobs.GetAssignment(identity.GetId());
		if (!assignment)
		{
			m_Jobs.SendMessage(player, identity, "ШКАФЧИК", "Снаряжение выдаётся только под контракт. Возьмите работу у нанимателя.");
			return;
		}

		JobsModJobJson job = m_Config.GetJob(assignment.GetJobId());
		if (!job)
			return;

		// A job pointed at a different cupboard is not served here. Otherwise
		// any locker would do for any contract, and the one by the warehouse
		// would be decoration.
		if (job.equipment_locker_id != placed.id)
		{
			m_Jobs.SendMessage(player, identity, "ШКАФЧИК", "Этот шкафчик не относится к вашей работе.");
			return;
		}

		if (!assignment.IsKitIssued())
		{
			Draw(player, identity, assignment, job);
			return;
		}

		Return(player, identity, assignment, job);
	}

	// --- drawing the kit ---
	protected void Draw(PlayerBase player, PlayerIdentity identity, JobsModAssignment assignment, JobsModJobJson job)
	{
		if (!m_Guard.IssueKit(player, assignment, job))
		{
			m_Jobs.SendMessage(player, identity, "ШКАФЧИК", "Не удалось выдать снаряжение — освободите место в инвентаре.");
			return;
		}

		assignment.SetKitIssued(true);
		m_Jobs.SendState(player, identity);
		m_Jobs.SendMessage(player, identity, "СНАРЯЖЕНИЕ ПОЛУЧЕНО", "Верните его в этот же шкафчик после смены.");
	}

	// --- putting it back, or paying for it ---
	protected void Return(PlayerBase player, PlayerIdentity identity, JobsModAssignment assignment, JobsModJobJson job)
	{
		if (assignment.IsKitReturned())
		{
			m_Jobs.SendMessage(player, identity, "ШКАФЧИК", "Снаряжение уже сдано. Вернитесь к нанимателю за оплатой.");
			return;
		}

		int missing = CountMissing(assignment);

		if (missing == 0)
		{
			assignment.DeleteAllCargo();
			assignment.SetKitReturned(true);
			m_Jobs.SendState(player, identity);
			m_Jobs.SendMessage(player, identity, "СНАРЯЖЕНИЕ СДАНО", "Вернитесь к нанимателю за оплатой.");
			return;
		}

		// Nothing to charge for: a job that set no fine simply writes the loss
		// off rather than stranding the player at a cupboard they cannot use.
		if (job.equipment_fine <= 0)
		{
			assignment.DeleteAllCargo();
			assignment.SetKitReturned(true);
			m_Jobs.SendState(player, identity);
			m_Jobs.SendMessage(player, identity, "СНАРЯЖЕНИЕ СДАНО", "Часть комплекта утеряна, но списана. Вернитесь к нанимателю.");
			return;
		}

		if (!TakeFine(player, job.equipment_fine))
		{
			m_Jobs.SendMessage(player, identity, "НЕ ХВАТАЕТ СНАРЯЖЕНИЯ", "Утеряно предметов: " + missing.ToString() + ". Штраф " + job.equipment_fine.ToString() + " — принесите деньги и подойдите снова.");
			return;
		}

		assignment.DeleteAllCargo();
		assignment.SetKitReturned(true);
		m_Jobs.SendState(player, identity);
		m_Jobs.SendMessage(player, identity, "ШТРАФ ОПЛАЧЕН", "Списано " + job.equipment_fine.ToString() + " за утерянное снаряжение. Вернитесь к нанимателю.");
	}

	// How much of the issued kit is no longer on the player. The assignment's
	// list holds what was handed over; an item that was dropped, eaten or lost
	// with a body is null or is somewhere else by now.
	protected int CountMissing(JobsModAssignment assignment)
	{
		array<EntityAI> kit = assignment.GetCargo();
		int missing = 0;

		for (int i = 0; i < kit.Count(); i++)
		{
			if (!kit.Get(i))
				missing++;
		}

		return missing;
	}

	// Paid in the same currency the rewards are, so a server that swapped its
	// economy mod does not end up fining players in a coin that does not exist.
	protected bool TakeFine(PlayerBase player, int amount)
	{
		string currency = m_Config.GetRewardClass();
		if (currency == "" || !player.GetInventory())
			return false;

		array<EntityAI> carried = new array<EntityAI>();
		if (!player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried))
			return false;

		array<EntityAI> notes = new array<EntityAI>();

		for (int i = 0; i < carried.Count(); i++)
		{
			EntityAI item = carried.Get(i);
			if (item && item.GetType() == currency)
				notes.Insert(item);
		}

		if (notes.Count() < amount)
			return false;

		for (int n = 0; n < amount; n++)
			notes.Get(n).Delete();

		return true;
	}
}
