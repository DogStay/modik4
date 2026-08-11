// ItemBaseJobsModTrash.c
//
// A physical janitor work point is a normal vanilla item model chosen at
// random. The synchronized flag turns only the server-spawned instance into a
// non-loot work object. Normal Firewood, bottles and cans elsewhere keep all
// vanilla behaviour.

modded class ItemBase
{
	protected bool m_JobsModWorldTrash;

	// The equipment locker rides on the same class for the same reason the NPC
	// flag rides on PlayerBase: a client cannot tell a contract locker from any
	// other cupboard, the class name is the admin's choice and so says nothing,
	// and a flag that travels with the entity is right the moment the entity is
	// visible. Lockers therefore have to be ItemBase-derived — every vanilla
	// container is.
	protected bool m_JobsModIsLocker;

	void ItemBase()
	{
		RegisterNetSyncVariableBool("m_JobsModWorldTrash");
		RegisterNetSyncVariableBool("m_JobsModIsLocker");
	}

	void JobsModSetLocker(bool value)
	{
		if (!GetGame().IsServer())
			return;

		m_JobsModIsLocker = value;
		SetSynchDirty();
	}

	bool JobsModIsLocker()
	{
		return m_JobsModIsLocker;
	}

	void JobsModSetWorldTrash(bool value)
	{
		if (!GetGame().IsServer())
			return;

		m_JobsModWorldTrash = value;
		SetAllowDamage(!value);
		SetSynchDirty();
	}

	bool JobsModIsWorldTrash()
	{
		return m_JobsModWorldTrash;
	}

	override bool IsTakeable()
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.IsTakeable();
	}

	override bool CanPutInCargo(EntityAI parent)
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.CanPutInCargo(parent);
	}

	override bool CanRemoveFromCargo(EntityAI parent)
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.CanRemoveFromCargo(parent);
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.CanPutIntoHands(parent);
	}

	override bool CanRemoveFromHands(EntityAI parent)
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.CanRemoveFromHands(parent);
	}

	override bool CanDetachAttachment(EntityAI parent)
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.CanDetachAttachment(parent);
	}

	override bool CanBeSplit()
	{
		if (m_JobsModWorldTrash)
			return false;

		return super.CanBeSplit();
	}
}
