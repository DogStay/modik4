// ActionSortTrash.c
//
// The only useful interaction on a server-spawned JobsMod world-trash item.
// The physical object may look like a plank, bottle, can or metal plate, but
// only an ItemBase carrying the synchronized JobsMod flag is accepted.

class ActionSortTrash extends ActionInteractBase
{
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
		return ResolveTrash(target) != null;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		if (!action_data)
			return;

		if (!action_data.m_Player)
			return;

		Object trash = ResolveTrash(action_data.m_Target);
		if (!trash)
			return;

		JobsModTrashActionBridge.RequestSort(action_data.m_Player, trash);
	}

	protected Object ResolveTrash(ActionTarget target)
	{
		if (!target)
			return null;

		Object direct = target.GetObject();
		if (IsJobsModTrash(direct))
			return direct;

		Object parent = target.GetParent();
		if (IsJobsModTrash(parent))
			return parent;

		return null;
	}

	protected bool IsJobsModTrash(Object object)
	{
		if (!object)
			return false;

		ItemBase item = ItemBase.Cast(object);
		if (!item)
			return false;

		return item.JobsModIsWorldTrash();
	}
}
