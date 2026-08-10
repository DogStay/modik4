// ActionTalkToNpc.c
//
// The "ask for work" interaction offered on one of the mod's employers.
//
// The action grants nothing by itself: it only tells the server that this player
// interacted with this object. Which object counts as an employer, whether the
// player is close enough, and what they are allowed to take are all decided
// server-side, because a client can drive an action pipeline but cannot be
// trusted about it.

class ActionTalkToNpc extends ActionInteractBase
{
	void ActionTalkToNpc()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "Поговорить о работе";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		// CCTMan is the target component for a human, which is what an employer
		// is: an ordinary survivor entity the server spawned and stood still.
		m_ConditionTarget = new CCTMan(JobsModRPC.NPC_INTERACTION_DISTANCE);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return ResolveNpc(player, target) != null;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		if (!action_data || !action_data.m_Player)
			return;

		Object npc = ResolveNpc(action_data.m_Player, action_data.m_Target);
		if (!npc)
			return;

		JobsModNpcActionBridge.RequestTalk(action_data.m_Player, npc);
	}

	// On the client this decides whether the entry shows up at all; on the
	// server it only narrows the object down to an employer before handing it
	// over. Both sides now ask the same question of the same replicated flag, so
	// there is no client-only branch left to disagree about.
	protected Object ResolveNpc(PlayerBase player, ActionTarget target)
	{
		if (!target)
			return null;

		PlayerBase other = PlayerBase.Cast(target.GetObject());
		if (!other || other == player)
			return null;

		if (!other.JobsModIsNpc())
			return null;

		return other;
	}
}
