// JobsModNpcActionBridge.c
//
// The seam between the two PBOs for the "talk to the employer" interaction.
//
// The action lives in the client PBO because every player needs it. The logic
// that answers it lives in the server PBO, which clients never download — so
// the action cannot name a single server type or the client would fail to
// compile. A ScriptInvoker inverts that dependency: the client side only
// announces "this player interacted with this object", and the server mod
// subscribes to it during bootstrap.
//
// Note what is not sent: no npc id. The client has no authoritative idea which
// object is an employer, so it does not claim to — it passes the object it
// actually interacted with, and the server decides what that is.

class JobsModNpcActionBridge
{
	protected static ref ScriptInvoker s_OnTalkRequested;

	static ScriptInvoker GetOnTalkRequested()
	{
		if (!s_OnTalkRequested)
			s_OnTalkRequested = new ScriptInvoker();

		return s_OnTalkRequested;
	}

	// Raised from the server half of ActionTalkToNpc only. Guarded here as well
	// as at the call site so a future caller cannot accidentally raise it on a
	// client, where no subscriber could legitimately act on it.
	static void RequestTalk(PlayerBase player, Object npc)
	{
		if (!GetGame().IsServer() || !player || !npc)
			return;

		GetOnTalkRequested().Invoke(player, npc);
	}
}
