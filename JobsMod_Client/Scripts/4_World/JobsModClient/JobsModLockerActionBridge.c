// JobsModLockerActionBridge.c
//
// The seam between the two PBOs for the equipment locker, built the same way
// as the NPC bridge: the action ships to every player, the logic that answers
// it lives in the server PBO which clients never download, and a ScriptInvoker
// inverts the dependency so the action never names a server type.
//
// As with the NPC, no locker id is sent. The client has no authoritative idea
// which cupboard belongs to the mod, so it passes the object it interacted
// with and lets the server decide what that is.

class JobsModLockerActionBridge
{
	protected static ref ScriptInvoker s_OnLockerUsed;

	static ScriptInvoker GetOnLockerUsed()
	{
		if (!s_OnLockerUsed)
			s_OnLockerUsed = new ScriptInvoker();

		return s_OnLockerUsed;
	}

	static void RequestLockerUse(PlayerBase player, Object locker)
	{
		if (!GetGame().IsServer() || !player || !locker)
			return;

		GetOnLockerUsed().Invoke(player, locker);
	}
}
