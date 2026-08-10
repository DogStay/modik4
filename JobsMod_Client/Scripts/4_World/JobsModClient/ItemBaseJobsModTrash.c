// ItemBaseJobsModTrash.c
//
// A physical janitor work point is a normal vanilla item model chosen at
// random. The synchronized flag turns only the server-spawned instance into a
// non-loot work object. Normal Firewood, bottles and cans elsewhere keep all
// vanilla behaviour.

modded class ItemBase
{
	protected bool m_JobsModWorldTrash;

	void ItemBase()
	{
		RegisterNetSyncVariableBool("m_JobsModWorldTrash");
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
