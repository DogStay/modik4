// ActionSearchCache.c
//
// "Обыскать" — the only way a player ever interacts with a cache.
//
// It appears when, and only when, the cursor is on a CacheInteractionProxy. A
// proxy exists only for a cache that is ACTIVE and available right now, so the
// action's own condition never has to ask the server whether the cache is
// worth searching: the server answered that by creating (or deleting) the
// proxy. What the condition does check is the part that is about this player —
// distance and, for a TOOL_REQUIRED cache, the tool.
//
// Everything that changes the world happens in OnFinishProgressServer, which
// the engine calls only when the whole progress ran out. Cancelling — letting
// go, walking off, turning away, being interrupted — never reaches it, so no
// path exists where an aborted search spawns a chest or marks a cache as found.

// The progress component of one running search.
//
// CreateActionComponent lives on the callback, not on the action: there is a
// single action object shared by everyone, and a callback per player actually
// performing it. How long the search takes comes from the server config, which
// each client is sent after connect — an admin changing search_duration_seconds
// needs no client update.
class ActionSearchCacheCB extends ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		float duration = CacheModSearchTuning.GetDurationSeconds();
		m_ActionData.m_ActionComponent = new CAContinuousTime(duration);
	}
}

class ActionSearchCache extends ActionContinuousBase
{
	// How far the player may drift from where they started before the search is
	// called off. Small enough that stepping away ends it, large enough that
	// the stance shuffle at the start of the animation does not.
	protected static const float MAX_DRIFT_METRES = 0.9;

	// How far the character may turn before the search is called off. This is
	// what stops a player spinning to face a threat mid-search and carrying on
	// as if nothing happened.
	//
	// It is written as the cosine of the angle rather than as the angle,
	// because comparing dot products needs no trigonometry at all: two facings
	// this far apart or less have a dot product of at least this. 0.57 is 55
	// degrees.
	protected static const float MIN_FACING_DOT = 0.57;

	void ActionSearchCache()
	{
		// The callback is where the progress component lives: the engine builds
		// one per running action, and the duration belongs to that instance
		// rather than to the action object shared by every player.
		m_CallbackClass = ActionSearchCacheCB;

		// The looping hands-on interaction: the character works at something in
		// front of them for as long as the action runs. It is deliberately not
		// the deploy motion and not the book-reading one.
		//
		// This is the INTERACT pair of the one-shot command the JobsMod actions
		// already use on this build, and that is why it is the one chosen: the
		// full-body digging commands are named differently between game
		// versions, and a constant that does not resolve costs the whole World
		// module rather than just the animation. A server whose build has a
		// digging loop can point m_CommandUID at it — nothing else in the
		// action depends on which animation plays.
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTLOOP;

		// Half body, because that is what an ACTIONMOD command is. Standing
		// still is not enforced by the animation then, but it was never the
		// animation's job: ActionConditionContinue below cancels the search the
		// moment the player drifts 0.9 m or turns past 55 degrees, which is a
		// stricter leash than a full-body animation gives and one that works
		// the same on every build.
		m_FullBody = false;

		m_Text = "Обыскать";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();

		// The cursor has to be on the proxy. The distance passed here is the
		// engine's own reach; the real bound is the cache's radius, checked
		// below against the proxy's position and again on the server.
		m_ConditionTarget = new CCTCursor(CacheModRPC.MAX_CACHE_RADIUS);
	}

	// Where the player stood and which way they faced when the search began.
	// Both are compared against every tick to decide whether they are still
	// doing the thing they started.
	override void OnStart(ActionData action_data)
	{
		super.OnStart(action_data);

		if (!action_data || !action_data.m_Player)
			return;

		CacheSearchActionData searchData = CacheSearchActionData.Cast(action_data);
		if (!searchData)
			return;

		searchData.m_StartPosition = action_data.m_Player.GetPosition();
		searchData.m_StartDirection = action_data.m_Player.GetDirection();
	}

	override ActionData CreateActionData()
	{
		CacheSearchActionData actionData = new CacheSearchActionData();
		return actionData;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player)
			return false;

		CacheInteractionProxy proxy = ResolveProxy(target);
		if (!proxy)
			return false;

		// Distance is measured against the proxy, which the server put at the
		// cache position. MAX_CACHE_RADIUS is the ceiling any cache may use;
		// the exact per-cache radius is enforced by the server on completion,
		// because the client is not told what it is.
		vector playerPosition = player.GetPosition();
		vector proxyPosition = proxy.GetPosition();
		float distance = vector.Distance(playerPosition, proxyPosition);
		if (distance > CacheModRPC.MAX_CACHE_RADIUS)
			return false;

		int toolIndex = proxy.CacheModGetRequiredToolIndex();
		if (toolIndex == CacheModTools.NONE)
			return true;

		bool handsOnly = proxy.CacheModIsToolInHandsOnly();
		return HasRequiredTool(player, toolIndex, handsOnly);
	}

	// Re-checked while the progress runs. Returning false here is what cancels
	// a search that started legitimately and stopped being legitimate: the
	// player walked off, turned away, or lost the target.
	override bool ActionConditionContinue(ActionData action_data)
	{
		if (!action_data)
			return false;

		PlayerBase player = action_data.m_Player;
		if (!player)
			return false;

		CacheInteractionProxy proxy = ResolveProxy(action_data.m_Target);
		if (!proxy)
			return false;

		CacheSearchActionData searchData = CacheSearchActionData.Cast(action_data);
		if (!searchData)
			return false;

		vector currentPosition = player.GetPosition();
		float drift = vector.Distance(currentPosition, searchData.m_StartPosition);
		if (drift > MAX_DRIFT_METRES)
		{
			CacheLog.Debug(CacheLog.SEARCH, "Обыск прерван: игрок сместился на " + drift.ToString() + " м.");
			return false;
		}

		vector currentDirection = player.GetDirection();
		float facing = GetFacingDot(searchData.m_StartDirection, currentDirection);
		if (facing < MIN_FACING_DOT)
		{
			CacheLog.Debug(CacheLog.SEARCH, "Обыск прерван: игрок развернулся (dot " + facing.ToString() + ").");
			return false;
		}

		return true;
	}

	// The one place the world is allowed to change. Everything past this point
	// is the server's: the client half of this action never learns whether the
	// search produced anything.
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);

		if (!action_data)
			return;

		PlayerBase player = action_data.m_Player;
		if (!player)
			return;

		CacheInteractionProxy proxy = ResolveProxy(action_data.m_Target);
		if (!proxy)
		{
			CacheLog.Info(CacheLog.SEARCH, "Обыск завершён без цели — тайник исчез во время действия.");
			return;
		}

		CacheLog.Info(CacheLog.SEARCH, "Обыск завершён, запрос отправлен на сервер.");
		CacheSearchActionBridge.ReportSearchFinished(player, proxy);
	}

	// =====================================================================
	// Helpers
	// =====================================================================
	protected CacheInteractionProxy ResolveProxy(ActionTarget target)
	{
		if (!target)
			return null;

		Object direct = target.GetObject();
		CacheInteractionProxy proxy = CacheInteractionProxy.Cast(direct);
		if (proxy)
			return proxy;

		Object parent = target.GetParent();
		return CacheInteractionProxy.Cast(parent);
	}

	protected bool HasRequiredTool(PlayerBase player, int toolIndex, bool handsOnly)
	{
		string wanted = CacheModTools.GetClassName(toolIndex);
		if (wanted == "")
			return true;

		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (inHands && inHands.IsKindOf(wanted))
			return true;

		if (handsOnly)
			return false;

		array<EntityAI> carried = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried);

		for (int i = 0; i < carried.Count(); i++)
		{
			EntityAI candidate = carried.Get(i);
			if (!candidate)
				continue;

			if (candidate.IsKindOf(wanted))
				return true;
		}

		return false;
	}

	// How closely two horizontal facings agree: 1 is the same direction, 0 is a
	// right angle, -1 is the opposite way. The vertical component is dropped on
	// purpose — looking down at the ground is part of searching, turning away
	// from it is not.
	protected float GetFacingDot(vector first, vector second)
	{
		vector flatFirst = first;
		flatFirst[1] = 0;
		flatFirst.Normalize();

		vector flatSecond = second;
		flatSecond[1] = 0;
		flatSecond.Normalize();

		return vector.Dot(flatFirst, flatSecond);
	}
}

// Where the player stood and faced when the search began, carried alongside the
// engine's own action data so the continue-check has something to compare to.
class CacheSearchActionData extends ActionData
{
	vector m_StartPosition;
	vector m_StartDirection;
}

// How long one search takes.
//
// It lives on the client because CAContinuousTime is built there, and it is a
// single value rather than a per-cache one because a duration that differs from
// point to point is a difficulty setting nobody asked for. The server sends its
// configured value to each player once, after connect; until then this is the
// shipped default, which is also what a client sees on a server whose server
// PBO is missing.
class CacheModSearchTuning
{
	protected static float s_DurationSeconds = 7.0;

	static void SetDurationSeconds(float seconds)
	{
		if (seconds < 1.0)
			seconds = 1.0;

		if (seconds > 120.0)
			seconds = 120.0;

		s_DurationSeconds = seconds;
	}

	static float GetDurationSeconds()
	{
		return s_DurationSeconds;
	}
}
