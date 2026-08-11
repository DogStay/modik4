// ActionUseLocker.c
//
// The one interaction on an equipment locker. What it does depends on where
// the player is in their contract — draw the kit, hand it back, or settle the
// fine for losing it — and that is decided entirely by the server.
//
// Deliberately one action rather than three. The client cannot know whether a
// kit has been issued, whether it is still complete, or what a fine costs
// without being told, and telling it would mean a state message per locker per
// player. One action that says "this player used this locker" needs none of
// that, and the answer comes back as a notification.

class ActionUseLocker extends ActionInteractBase
{
	void ActionUseLocker()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "Шкафчик со снаряжением";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTCursor(JobsModRPC.INTERACTION_DISTANCE);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return ResolveLocker(target) != null;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		if (!action_data || !action_data.m_Player)
			return;

		Object locker = ResolveLocker(action_data.m_Target);
		if (!locker)
			return;

		JobsModLockerActionBridge.RequestLockerUse(action_data.m_Player, locker);
	}

	// Recognised by the replicated flag rather than by class name: a locker is
	// an ordinary piece of furniture whose class the admin chooses, so the
	// class tells us nothing. The flag travels with the entity, which is what
	// the NPCs already do and for the same reason.
	//
	// The cursor may land on the locker itself or on something parented to it,
	// so both are tried before giving up.
	protected Object ResolveLocker(ActionTarget target)
	{
		if (!target)
			return null;

		ItemBase direct = ItemBase.Cast(target.GetObject());
		if (direct && direct.JobsModIsLocker())
			return direct;

		ItemBase parent = ItemBase.Cast(target.GetParent());
		if (parent && parent.JobsModIsLocker())
			return parent;

		return null;
	}
}
