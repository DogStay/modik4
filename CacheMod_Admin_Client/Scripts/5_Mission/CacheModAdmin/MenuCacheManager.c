// MenuCacheManager.c
//
// The Cache Mod page inside VPP Admin Tools.
//
// It edits one thing — the placed points — through typed fields, and never
// exposes raw JSON. Everything it changes is applied by the server: this class
// sends a request and redraws whatever comes back, so a client that lost its
// permission between opening the menu and pressing a button simply sees nothing
// happen, which is what the server decided.
//
// Zones and tiers are edited in their files for now (loot_zones.json,
// loot_tiers.json) and picked up by ПЕРЕЧИТАТЬ without a restart. The Loot
// Workbench that will replace that is the next piece of work; the button here
// is what makes editing them usable in the meantime.

class MenuCacheManager extends AdminHudSubMenu
{
	protected static const string LAYOUT_PATH = "CacheMod_Admin_Client/GUI/Layouts/MenuCacheManager.layout";

	// The CF RPC channel VPP's menus travel on.
	protected static const int CACHEMOD_CF_RPC_ID = 10042;

	protected TextListboxWidget m_RecordList;
	protected TextWidget m_RecordIdText;
	protected TextWidget m_StatusText;
	protected TextWidget m_InfoText;

	protected EditBoxWidget m_EditRadius;
	protected EditBoxWidget m_EditChance;
	protected EditBoxWidget m_EditTool;

	protected ButtonWidget m_BtnType;
	protected ButtonWidget m_BtnSave;
	protected ButtonWidget m_BtnDelete;
	protected ButtonWidget m_BtnTeleport;
	protected ButtonWidget m_BtnRefresh;
	protected ButtonWidget m_BtnSpheres;
	protected ButtonWidget m_BtnReload;

	protected string m_SelectedId;
	protected int m_SelectedType;

	void MenuCacheManager()
	{
		GetRPCManager().AddRPC("RPC_CacheModAdminClient", "OnPoints", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("RPC_CacheModAdminClient", "OnAdminMessage", this, SingleplayerExecutionType.Client);
	}

	override void OnCreate(Widget RootW)
	{
		super.OnCreate(RootW);

		M_SUB_WIDGET = CreateWidgets(LAYOUT_PATH);
		if (!M_SUB_WIDGET)
		{
			CacheLog.Error(CacheLog.ADMIN, "Layout не создан: " + LAYOUT_PATH);
			return;
		}

		M_SUB_WIDGET.SetHandler(this);

		m_RecordList = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RecordList"));
		m_RecordIdText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RecordIdText"));
		m_StatusText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("StatusText"));
		m_InfoText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("InfoText"));

		m_EditRadius = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("EditRadius"));
		m_EditChance = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("EditChance"));
		m_EditTool = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("EditTool"));

		m_BtnType = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnType"));
		m_BtnSave = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSave"));
		m_BtnDelete = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnDelete"));
		m_BtnTeleport = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnTeleport"));
		m_BtnRefresh = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnRefresh"));
		m_BtnSpheres = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSpheres"));
		m_BtnReload = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnReload"));

		// Whatever the client already has is drawn at once, so the menu is
		// never blank while the request is in flight.
		RedrawList();
		RequestPoints();
	}

	// =====================================================================
	// Server traffic
	// =====================================================================
	protected void SendAdminRpc(string functionName, Param payload = null)
	{
		array<ref Param> rpcData = new array<ref Param>;
		rpcData.Insert(new Param2<string, string>("RPC_CacheModAdmin", functionName));

		if (payload)
			rpcData.Insert(payload);

		GetGame().RPC(NULL, CACHEMOD_CF_RPC_ID, rpcData, true);
	}

	protected void RequestPoints()
	{
		SendAdminRpc("GetPoints");
	}

	// The point list, delivered through VPP's channel. It is stored in the same
	// place the ordinary client receiver puts it, so the spheres and this list
	// can never disagree about what the server said.
	void OnPoints(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;

		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
			return;

		CacheModClientContext.ApplyPackedPoints(data.param1);
		RedrawList();
	}

	void OnAdminMessage(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;

		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
			return;

		SetStatus(data.param1);
	}

	// =====================================================================
	// Drawing
	// =====================================================================
	protected void RedrawList()
	{
		if (!m_RecordList)
			return;

		m_RecordList.ClearItems();

		array<ref CacheModAdminPoint> points = CacheModClientContext.GetAdminPoints();

		for (int i = 0; i < points.Count(); i++)
		{
			CacheModAdminPoint point = points.Get(i);
			if (!point)
				continue;

			string row = point.m_Id;
			row = row + "   " + point.GetStateText();
			row = row + "   " + point.m_SpawnChance.ToString() + "%";
			row = row + "   R" + point.m_Radius.ToString();

			m_RecordList.AddItem(row, null, 0);
		}

		// Keep the form pointing at the same cache across a refresh, so a save
		// followed by the reply does not clear the fields under the admin.
		CacheModAdminPoint selected = FindPoint(m_SelectedId);
		if (selected)
			ShowPoint(selected);
	}

	protected void ShowPoint(CacheModAdminPoint point)
	{
		m_SelectedId = point.m_Id;
		m_SelectedType = point.m_CacheType;

		m_RecordIdText.SetText("Тайник: " + point.m_Id);
		m_EditRadius.SetText(point.m_Radius.ToString());
		m_EditChance.SetText(point.m_SpawnChance.ToString());
		UpdateTypeButton();

		string info = "Состояние: " + point.GetStateText() + "\n";
		info = info + "Позиция: " + point.m_Position.ToString() + "\n";

		if (point.m_RespawnSecondsLeft > 0)
			info = info + "До респавна: " + point.m_RespawnSecondsLeft.ToString() + " c\n";

		info = info + "\nSPAWN CHANCE применяется при следующем рестарте сервера.\n";
		info = info + "RADIUS — единственный радиус тайника: сфера, зона поиска и проверка дистанции.";

		m_InfoText.SetText(info);
	}

	protected void UpdateTypeButton()
	{
		if (!m_BtnType)
			return;

		m_BtnType.SetText(CacheModType.ToText(m_SelectedType));
	}

	protected void SetStatus(string text)
	{
		if (m_StatusText)
			m_StatusText.SetText(text);
	}

	protected CacheModAdminPoint FindPoint(string id)
	{
		if (id == "")
			return null;

		array<ref CacheModAdminPoint> points = CacheModClientContext.GetAdminPoints();

		for (int i = 0; i < points.Count(); i++)
		{
			CacheModAdminPoint point = points.Get(i);
			if (point && point.m_Id == id)
				return point;
		}

		return null;
	}

	// =====================================================================
	// Input
	// =====================================================================
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (super.OnClick(w, x, y, button))
			return true;

		if (w == m_BtnRefresh)
		{
			RequestPoints();
			SetStatus("Список обновляется…");
			return true;
		}

		if (w == m_BtnSpheres)
		{
			CacheModSphereRenderer.Toggle();

			if (CacheModSphereRenderer.IsEnabled())
				SetStatus("Сферы включены: зелёная — ACTIVE, серая — INACTIVE, красная — COOLDOWN, жёлтая — нужен инструмент.");
			else
				SetStatus("Сферы выключены.");

			return true;
		}

		if (w == m_BtnReload)
		{
			SendAdminRpc("ReloadStatic");
			SetStatus("Запрошено перечитывание зон, тиров и настроек…");
			return true;
		}

		if (w == m_BtnType)
		{
			if (m_SelectedType == CacheModType.BASIC)
				m_SelectedType = CacheModType.TOOL_REQUIRED;
			else
				m_SelectedType = CacheModType.BASIC;

			UpdateTypeButton();
			return true;
		}

		if (w == m_BtnSave)
		{
			SaveSelected();
			return true;
		}

		if (w == m_BtnDelete)
		{
			DeleteSelected();
			return true;
		}

		if (w == m_BtnTeleport)
		{
			if (m_SelectedId == "")
			{
				SetStatus("Сначала выберите тайник.");
				return true;
			}

			SendAdminRpc("TeleportToCache", new Param1<string>(m_SelectedId));
			return true;
		}

		return false;
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (w != m_RecordList)
			return false;

		array<ref CacheModAdminPoint> points = CacheModClientContext.GetAdminPoints();

		if (row < 0 || row >= points.Count())
			return false;

		ShowPoint(points.Get(row));
		return true;
	}

	protected void SaveSelected()
	{
		if (m_SelectedId == "")
		{
			SetStatus("Сначала выберите тайник.");
			return;
		}

		string radiusText;
		string chanceText;
		string toolText;

		m_EditRadius.GetText(radiusText);
		m_EditChance.GetText(chanceText);
		m_EditTool.GetText(toolText);

		float radius = radiusText.ToFloat();
		float chance = chanceText.ToFloat();

		// The server clamps both again; this is only so an obvious typo is
		// answered here instead of silently becoming something else there.
		float minRadius = CacheModRPC.MIN_CACHE_RADIUS;
		float maxRadius = CacheModRPC.MAX_CACHE_RADIUS;

		if (radius < minRadius || radius > maxRadius)
		{
			SetStatus("Радиус должен быть от " + minRadius.ToString() + " до " + maxRadius.ToString() + " м.");
			return;
		}

		if (chance < 0.0 || chance > 100.0)
		{
			SetStatus("Шанс появления должен быть от 0 до 100.");
			return;
		}

		string packedTail = toolText + CacheModRPC.FIELD_SEPARATOR + chanceText;

		SendAdminRpc("SaveCache", new Param4<string, float, int, string>(m_SelectedId, radius, m_SelectedType, packedTail));
		SetStatus("Сохранение…");
	}

	protected void DeleteSelected()
	{
		if (m_SelectedId == "")
		{
			SetStatus("Сначала выберите тайник.");
			return;
		}

		SendAdminRpc("DeleteCache", new Param1<string>(m_SelectedId));
		SetStatus("Удаление…");

		m_SelectedId = "";
		m_RecordIdText.SetText("Тайник не выбран");
	}
}
