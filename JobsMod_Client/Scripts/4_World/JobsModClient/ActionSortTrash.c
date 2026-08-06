// ActionSortTrash.c
//
// The "sort the trash" interaction offered on a JobsMod_TrashPile.
//
// The pile is recognised by its config class name rather than by casting to a
// script class. The mod ships no script class for the pile, so there is nothing
// to get wrong about which vanilla base it inherits — and one less class means
// one less way for the item to lose its container behaviour.
//
// The action grants nothing by itself: it only tells the server that this
// player asked. Distance, ownership and cooldown are re-checked server-side,
// because a client can drive an action pipeline but cannot be trusted about it.

class ActionSortTrash extends ActionInteractBase
{
	protected static const string TRASH_PILE_CLASS = "JobsMod_TrashPile";

	void ActionSortTrash()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "Сортировать мусор";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTCursor(JobsModRPC.INTERACTION_DISTANCE);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return ResolvePile(target) != null;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		if (!action_data || !action_data.m_Player)
			return;

		Object pile = ResolvePile(action_data.m_Target);
		if (!pile)
			return;

		JobsModTrashActionBridge.RequestSort(action_data.m_Player, pile);
	}

	// The cursor may land on the pile itself or on something parented to it,
	// so both are accepted before giving up.
	protected Object ResolvePile(ActionTarget target)
	{
		if (!target)
			return null;

		Object direct = target.GetObject();
		if (direct && direct.GetType() == TRASH_PILE_CLASS)
			return direct;

		Object parent = target.GetParent();
		if (parent && parent.GetType() == TRASH_PILE_CLASS)
			return parent;

		return null;
	}
}
