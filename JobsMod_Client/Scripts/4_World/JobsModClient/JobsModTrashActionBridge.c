// JobsModTrashActionBridge.c
//
// The seam between the two PBOs.
//
// The action lives in the client PBO because every player needs it. The logic
// that answers it lives in the server PBO, which clients never download — so
// the action cannot name a single server type or the client would fail to
// compile. A ScriptInvoker inverts that dependency: the client side only
// announces "this player asked to sort at this object", and the server mod
// subscribes to it during bootstrap.
//
// With no server mod loaded the invoker simply has no subscribers and the
// request is dropped, which is the correct behaviour for a client that joined
// a server not running JobsMod_Server.

class JobsModTrashActionBridge
{
	protected static ref ScriptInvoker s_OnSortRequested;

	static ScriptInvoker GetOnSortRequested()
	{
		if (!s_OnSortRequested)
			s_OnSortRequested = new ScriptInvoker();

		return s_OnSortRequested;
	}

	// Raised from the server half of ActionSortTrash only. Guarded here as well
	// as at the call site so a future caller cannot accidentally raise it on a
	// client, where no subscriber could legitimately act on it.
	static void RequestSort(PlayerBase player, Object pile)
	{
		if (!GetGame().IsServer() || !player || !pile)
			return;

		GetOnSortRequested().Invoke(player, pile);
	}
}
