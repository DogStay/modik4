// ActionCreateCachePoint.c
//
// Confirms a cache position from the placement hologram.
//
// The position is taken from Hologram.GetProjectionPosition() and from nowhere
// else. That is the point the engine has already validated as a legal surface
// for a placed object, at the distance and slope the placement system allows —
// a hand-rolled camera raycast would happily return a spot inside a rock.
//
// The hologram only exists on the client that owns it, so the position has to
// be read client-side and sent. That is safe because the server treats the
// number as a request: it re-checks the sender's admin rights, that the point
// is near the sender, and that it is inside the map before writing anything.

class ActionCreateCachePoint extends ActionSingleUseBase
{
	void ActionCreateCachePoint()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "Разместить тайник";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTNone();
	}

	// No GetInputType override on purpose. ActionSingleUseBase already binds to
	// the default use input, which is the same key the placement mode is
	// confirmed with, and naming an input class by hand is how the last build
	// broke: the set of input typenames differs between game versions.
	override bool HasTarget()
	{
		return false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player)
			return false;

		CacheCreatorBook book = CacheCreatorBook.Cast(item);
		if (!book)
			return false;

		// Only while a hologram is up: the action is "confirm this position",
		// and with no hologram there is no position to confirm.
		return player.IsPlacingLocal();
	}

	// Client-side half. The hologram is a local object, so this is the only
	// side that can read it — hence the request travels as an RPC rather than
	// through the action's own server half.
	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);

		if (!action_data)
			return;

		PlayerBase player = action_data.m_Player;
		if (!player)
			return;

		CacheCreatorBook book = CacheCreatorBook.Cast(action_data.m_MainItem);
		if (!book)
			return;

		Hologram hologram = player.GetHologramLocal();
		if (!hologram)
			return;

		vector position = hologram.GetProjectionPosition();
		float radius = book.CacheModGetDefaultRadius();
		int cacheType = book.CacheModGetCacheType();

		CacheLog.Info(CacheLog.PLACEMENT, "Запрос размещения: " + position.ToString() + ", радиус " + radius.ToString());

		GetGame().RPCSingleParam(
			player,
			CacheModRPC.REQUEST_CREATE_CACHE,
			new Param3<vector, float, int>(position, radius, cacheType),
			true,
			null);

		// Close placement mode straight away. The book stays in the hands and
		// nothing is dropped; toggling it again starts the next placement.
		player.TogglePlacingLocal();
	}
}
