// JobsModTrashCatalog.c
//
// The 9 trash items and 3 bins of the sorting minigame, exactly as specified in
// DESIGN_REFERENCE/TRASH_SORTING_FINAL/dayz_layout_spec.md.
//
// Both sides read the catalog from here. The server shuffles item ids and sends
// the order; the client renders that order. Because the material is derived
// from the id on both sides, the client cannot claim a different material for
// an item than the server will check against.
//
// This is deliberately not hidden information: the player is supposed to tell
// wood from plastic by looking at the item. The anti-cheat boundary is the
// session nonce and the server-side answer check, not secrecy of the material.

class JobsModTrashBin
{
	static const string WOOD = "wood";
	static const string PLASTIC = "plastic";
	static const string METAL = "metal";

	// Bins in the fixed left-to-right order used by the layout and by the
	// index carried in a submitted sequence.
	static const int COUNT = 3;

	static string GetIdByIndex(int index)
	{
		switch (index)
		{
			case 0:
				return WOOD;
			case 1:
				return PLASTIC;
			case 2:
				return METAL;
		}

		return "";
	}

	static int GetIndexById(string binId)
	{
		if (binId == WOOD)
			return 0;
		if (binId == PLASTIC)
			return 1;
		if (binId == METAL)
			return 2;

		return -1;
	}

	static string GetLabel(string binId)
	{
		if (binId == WOOD)
			return "ДЕРЕВО";
		if (binId == PLASTIC)
			return "ПЛАСТИК";
		if (binId == METAL)
			return "МЕТАЛЛ";

		return "";
	}
}

class JobsModTrashItem
{
	string m_Id;
	string m_BinId;
	string m_DisplayName;
	// Vanilla class used to render the card preview in 3D. The mod ships no
	// textures, so the item picture is a real game model instead of a .paa.
	string m_PreviewClassName;

	void JobsModTrashItem(string id, string binId, string displayName, string previewClassName)
	{
		m_Id = id;
		m_BinId = binId;
		m_DisplayName = displayName;
		m_PreviewClassName = previewClassName;
	}

	string GetId() { return m_Id; }
	string GetBinId() { return m_BinId; }
	string GetDisplayName() { return m_DisplayName; }
	string GetPreviewClassName() { return m_PreviewClassName; }
}

class JobsModTrashCatalog
{
	// One full round is exactly these 9 items: 3 per bin, as the design states.
	static const int ITEM_COUNT = 9;
	static const int ITEMS_PER_BIN = 3;

	protected static ref array<ref JobsModTrashItem> s_Items;

	// Built once and cached. Vanilla preview classes are chosen so the card
	// shows a model whose material is visually obvious to the player.
	protected static void Build()
	{
		if (s_Items)
			return;

		s_Items = new array<ref JobsModTrashItem>();

		s_Items.Insert(new JobsModTrashItem("wood_01", JobsModTrashBin.WOOD, "Обломок доски", "WoodenPlank"));
		s_Items.Insert(new JobsModTrashItem("wood_02", JobsModTrashBin.WOOD, "Деревянная палка", "WoodenStick"));
		s_Items.Insert(new JobsModTrashItem("wood_03", JobsModTrashBin.WOOD, "Щепки", "Firewood"));

		s_Items.Insert(new JobsModTrashItem("plastic_01", JobsModTrashBin.PLASTIC, "Пластиковая бутылка", "WaterBottle"));
		s_Items.Insert(new JobsModTrashItem("plastic_02", JobsModTrashBin.PLASTIC, "Смятый стакан", "Canteen"));
		s_Items.Insert(new JobsModTrashItem("plastic_03", JobsModTrashBin.PLASTIC, "Пластиковая канистра", "CanisterGasoline"));

		s_Items.Insert(new JobsModTrashItem("metal_01", JobsModTrashBin.METAL, "Жестяная банка", "BakedBeansCan"));
		s_Items.Insert(new JobsModTrashItem("metal_02", JobsModTrashBin.METAL, "Металлический обломок", "SheetMetal"));
		s_Items.Insert(new JobsModTrashItem("metal_03", JobsModTrashBin.METAL, "Ржавая деталь", "Pot"));
	}

	static array<ref JobsModTrashItem> GetItems()
	{
		Build();
		return s_Items;
	}

	static JobsModTrashItem Find(string itemId)
	{
		Build();

		for (int i = 0; i < s_Items.Count(); i++)
		{
			if (s_Items.Get(i).GetId() == itemId)
				return s_Items.Get(i);
		}

		return null;
	}

	// Material the item belongs to, or "" when the id is not in the catalog.
	// The server uses this to grade a submitted sequence.
	static string GetBinIdForItem(string itemId)
	{
		JobsModTrashItem item = Find(itemId);
		if (!item)
			return "";

		return item.GetBinId();
	}

	// Packs an ordered id list into one transferable string.
	static string PackOrder(array<string> itemIds)
	{
		string packed = "";

		for (int i = 0; i < itemIds.Count(); i++)
		{
			if (i > 0)
				packed = packed + JobsModRPC.FIELD_SEPARATOR;

			packed = packed + itemIds.Get(i);
		}

		return packed;
	}

	// Reverse of PackOrder. Returns false when the string does not hold exactly
	// ITEM_COUNT known catalog ids without repeats — the shape every caller on
	// both sides relies on.
	static bool UnpackOrder(string packed, out array<string> itemIds)
	{
		itemIds = new array<string>();

		if (packed == "")
			return false;

		array<string> parts = new array<string>();
		packed.Split(JobsModRPC.FIELD_SEPARATOR, parts);

		if (parts.Count() != ITEM_COUNT)
			return false;

		for (int i = 0; i < parts.Count(); i++)
		{
			string id = parts.Get(i);

			if (!Find(id))
				return false;

			if (itemIds.Find(id) != -1)
				return false;

			itemIds.Insert(id);
		}

		return true;
	}

	// Guards the assumption PackOrder/UnpackOrder rest on: no id may contain the
	// separator, and the catalog must hold exactly 3 items per bin. Called once
	// at server start so a bad edit fails loudly instead of at runtime.
	static bool Validate(out string errorMessage)
	{
		Build();
		errorMessage = "";

		if (s_Items.Count() != ITEM_COUNT)
		{
			errorMessage = "каталог содержит " + s_Items.Count().ToString() + " предметов вместо " + ITEM_COUNT.ToString();
			return false;
		}

		int wood = 0;
		int plastic = 0;
		int metal = 0;

		for (int i = 0; i < s_Items.Count(); i++)
		{
			JobsModTrashItem item = s_Items.Get(i);

			if (item.GetId().Contains(JobsModRPC.FIELD_SEPARATOR))
			{
				errorMessage = "id '" + item.GetId() + "' содержит разделитель";
				return false;
			}

			if (item.GetBinId() == JobsModTrashBin.WOOD)
				wood++;
			else if (item.GetBinId() == JobsModTrashBin.PLASTIC)
				plastic++;
			else if (item.GetBinId() == JobsModTrashBin.METAL)
				metal++;
			else
			{
				errorMessage = "id '" + item.GetId() + "' ссылается на неизвестный контейнер '" + item.GetBinId() + "'";
				return false;
			}
		}

		if (wood != ITEMS_PER_BIN || plastic != ITEMS_PER_BIN || metal != ITEMS_PER_BIN)
		{
			errorMessage = "распределение по контейнерам " + wood.ToString() + "/" + plastic.ToString() + "/" + metal.ToString() + " вместо 3/3/3";
			return false;
		}

		return true;
	}
}
