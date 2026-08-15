// CacheModJson.c
//
// Plain data classes mirroring the files in $profile:CacheMod. They carry no
// logic beyond reading their own coordinate strings: JsonFileLoader fills the
// fields by name, so renaming a field here silently changes the on-disk format
// and is a breaking change.
//
// Four files, split by how often each is edited and by who edits it:
//
//   settings.json      global switches: search duration, default tier, chest
//   caches.json        every placed point; written by the game as admins place
//   loot_zones.json    the spheres that decide which tier a point pays out
//   loot_tiers.json    what each tier can contain
//
// caches.json is the only one the running server writes on its own, and it
// writes the whole file at once: a cache is a short record and a rewrite is
// cheaper than tracking which line changed.
//
// Positions are single strings in the shape #position prints them, so an admin
// can paste a coordinate straight out of the game. See CacheModCoords.

// ---------------------------------------------------------------------------
// settings.json
// ---------------------------------------------------------------------------
class CacheModChestSettingsJson
{
	// How long a spawned chest stands before it is deleted, whether or not
	// anyone emptied it.
	int lifetime_seconds;

	// Delete a chest as soon as its cargo is empty, rather than waiting out the
	// lifetime. Keeps looted chests from littering the map.
	bool despawn_when_empty;

	// Lifts the chest off the cache position. Zero puts it on the surface the
	// cache sits on, which is what almost every point wants.
	float height_offset;

	// Which container class is spawned. A server that ships its own container
	// mod points this at one of theirs.
	string chest_class;
}

class CacheModSettingsJson
{
	// How long the search action runs, in seconds. Sent to every client after
	// connect, because the progress bar is built client-side.
	float search_duration_seconds;

	// The tier used when a cache sits outside every loot zone.
	string default_tier;

	// Whether a TOOL_REQUIRED cache demands the tool in hand, or accepts it
	// anywhere in the inventory.
	bool tool_in_hands_only;

	// How long an emptied cache stays on cooldown before it can be found again.
	int respawn_seconds;

	// The spawn chance written into a cache that an admin places in-game and
	// has not given one of its own.
	float default_spawn_chance;

	bool debug_logging;

	ref CacheModChestSettingsJson chest_settings;
}

// ---------------------------------------------------------------------------
// caches.json
// ---------------------------------------------------------------------------
class CacheModCacheJson
{
	string id;

	// "1000 20 2000".
	string position;

	// The one and only radius a cache has. It is the admin sphere, the area a
	// player must be inside to search, and what the server validates distance
	// against. There is deliberately no second radius anywhere in this mod.
	float radius;

	// "BASIC" or "TOOL_REQUIRED".
	string cache_type;

	// Classname of the tool a TOOL_REQUIRED cache needs. Must be one of the
	// entries in CacheModTools, because that table is what the client is told.
	string required_tool;

	// 0..100. Rolled independently for this point at every server start; the
	// result decides whether it is reachable at all this run. It is not a count
	// and not a quota — every point rolls on its own, so a restart may bring up
	// none of them or all of them.
	float spawn_chance;

	// False while the point is on cooldown after being emptied. Survives a
	// restart together with next_respawn_time, so a cache emptied two minutes
	// before a restart is still empty two minutes after it.
	bool available;

	int last_search_time;
	int next_respawn_time;

	// Steam64 of whoever placed it, taken from PlayerIdentity on the server and
	// never from anything a client sent.
	string created_by;
	int created_utc;

	vector GetPosition()
	{
		vector parsed;
		CacheModCoords.Parse(position, parsed);
		return parsed;
	}

	void SetPosition(vector value)
	{
		position = CacheModCoords.Format(value);
	}
}

class CacheModCachesFileJson
{
	ref array<ref CacheModCacheJson> caches;
}

// ---------------------------------------------------------------------------
// loot_zones.json
// ---------------------------------------------------------------------------
class CacheModZoneJson
{
	string id;
	string display_name;

	// "12000 10 8000". Height is ignored when a point is tested against the
	// zone: zones are drawn on a map, and a cache in a basement is still in the
	// airfield.
	string center;

	float radius;

	// Which loot tier a cache inside this zone pays out.
	string tier;

	// Highest priority wins where zones overlap; on a tie the smaller zone
	// wins, because the smaller one is the more specific statement.
	int priority;

	// How many items a chest from this zone holds. Zero on both means "use the
	// tier's own numbers".
	int min_items;
	int max_items;

	vector GetCenter()
	{
		vector parsed;
		CacheModCoords.Parse(center, parsed);
		return parsed;
	}
}

class CacheModZonesFileJson
{
	ref array<ref CacheModZoneJson> zones;
}

// ---------------------------------------------------------------------------
// loot_tiers.json
// ---------------------------------------------------------------------------
class CacheModAttachmentJson
{
	string classname;

	// Percentage, 0..100.
	float chance;

	// How many to attach, for slots that take a count — magazines mostly.
	int quantity;
}

class CacheModLootItemJson
{
	string classname;

	// Relative weight inside its tier. Not a percentage: an entry of weight 20
	// against one of weight 10 is twice as likely, whatever else is in the list.
	float weight;

	// How many of this item one draw produces.
	int min;
	int max;

	ref array<ref CacheModAttachmentJson> attachments;
}

class CacheModTierJson
{
	string id;
	string display_name;

	// Fallback item count for zones that do not state their own.
	int min_items;
	int max_items;

	ref array<ref CacheModLootItemJson> items;
}

class CacheModTiersFileJson
{
	ref array<ref CacheModTierJson> tiers;
}

// ---------------------------------------------------------------------------
// admins.json
// ---------------------------------------------------------------------------
// Steam64s allowed to place caches and to see the admin point list, for servers
// that run the mod without VPP Admin Tools. See CacheModAuthority.
class CacheModAdminsJson
{
	ref array<string> steam_ids;
}
