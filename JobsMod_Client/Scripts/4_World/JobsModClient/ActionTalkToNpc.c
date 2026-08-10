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

	// Narrows the target down to a survivor other than the player. Whether that
	// survivor is actually an employer is decided by the server.
	//
	// The replicated flag is deliberately not allowed to block the action. It is
	// the right way to know, but it only knows once entity synchronisation has
	// delivered it, and when that has not happened the action does not appear
	// and says nothing — which is the failure that has cost this evening. The
	// entry is offered on any survivor instead, and JobsModJobService.HandleTalk
	// resolves the object against the real NPC registry before granting
	// anything, so pointing it at an ordinary player costs one refused request.
	//
	// The flag is still read, only to report itself: if this line stops
	// appearing, synchronisation works and the check can go back to blocking.
	protected Object ResolveNpc(PlayerBase player, ActionTarget target)
	{
		if (!target)
			return null;

		PlayerBase other = PlayerBase.Cast(target.GetObject());
		if (!other || other == player)
			return null;

		if (!other.JobsModIsNpc())
			Report("флаг NPC на цели не выставлен — действие предложено без него");

		return other;
	}

	// This condition runs every frame the player is looking at something, so the
	// one interesting case — a survivor under the cursor that the mod refuses to
	// treat as an employer — is reported at most once every few seconds. Without
	// it the two failures that look identical in game, "this PBO is not loaded"
	// and "the flag never arrived", cannot be told apart from outside.
	protected static int s_NextReportMs;

	protected void Report(string reason)
	{
		if (GetGame().IsDedicatedServer())
			return;

		int now = GetGame().GetTime();
		if (now < s_NextReportMs)
			return;

		s_NextReportMs = now + 5000;
		JobsLog.Info("CLIENT/ACTIONS: 'поговорить о работе' не предложено — " + reason + ".");
	}
}
