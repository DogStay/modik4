// CacheCreatorBook.c
//
// The administrator's placement tool.
//
// The book is deployable so that the vanilla placement system will build a
// hologram for it — that hologram is the whole reason it exists, because it is
// the engine's own answer to "where exactly is the player pointing, on ground
// the player can actually stand on". The mod never computes a position from the
// camera by hand.
//
// What the book is not is a thing you put down. Vanilla deployment would move
// the item out of the hands and onto the ground at the end of the animation,
// which is exactly wrong here: the admin needs to place ten caches without
// picking the book back up ten times. So the vanilla deploy action is refused
// for this class (see ActionDeployObjectCacheMod) and our own single-use action
// takes its place — it reads the projection position, sends it, and closes the
// hologram, leaving the book in the hands and nothing on the ground.

class CacheCreatorBook extends ItemBase
{
	// Deployable purely to unlock the hologram. Nothing is ever deployed.
	override bool IsDeployable()
	{
		return true;
	}

	// What kind of cache this book writes. Overridden by the tooled variant.
	int CacheModGetCacheType()
	{
		return CacheModType.BASIC;
	}

	// The radius the new cache gets. One number, and the only radius a cache
	// has: it is the admin sphere, the interaction area and the distance the
	// server validates against, all at once. It can be changed afterwards in
	// the Cache Manager.
	float CacheModGetDefaultRadius()
	{
		return 2.0;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionTogglePlaceObject);
		AddAction(ActionCreateCachePoint);
	}
}

// A cache anyone can search with bare hands.
class CacheCreatorBook_Basic extends CacheCreatorBook
{
}

// A cache that will not offer its action unless the player carries the tool the
// admin later assigns to it in the Cache Manager. The book only marks the type;
// which tool is required is a property of the cache, not of the book.
class CacheCreatorBook_Tooled extends CacheCreatorBook
{
	override int CacheModGetCacheType()
	{
		return CacheModType.TOOL_REQUIRED;
	}
}

// Vanilla deployment refuses to run for the creator books.
//
// Without this, F during placement offers two actions — the vanilla one that
// drops the book on the ground and ours that writes a cache — and which one
// wins is down to registration order. Refusing the vanilla one outright leaves
// exactly one meaning for the key.
modded class ActionDeployObject
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CacheCreatorBook book = CacheCreatorBook.Cast(item);
		if (book)
			return false;

		return super.ActionCondition(player, target, item);
	}
}
