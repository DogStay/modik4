// JobsMod VPP admin forms.
// No raw JSON editor is exposed to the administrator.
// All values are edited through typed form controls and are saved server-side
// through JobsModJsonFileIO.

class MenuJobsModAdmin extends AdminHudSubMenu
{
	protected static const string LAYOUT_PATH = "JobsMod_Admin_Client/GUI/Layouts/MenuJobsModAdmin.layout";

	protected static const int TAB_SETTINGS = 0;
	protected static const int TAB_JOBS = 1;
	protected static const int TAB_NPCS = 2;
	protected static const int TAB_WORLD = 3;

	protected static const int FIELD_ACTION_NONE = 0;
	protected static const int FIELD_ACTION_POSITION = 1;
	protected static const int FIELD_ACTION_CLASS = 2;

	protected Widget m_Header;
	protected ButtonWidget m_TabSettings;
	protected ButtonWidget m_TabJobs;
	protected ButtonWidget m_TabNpcs;
	protected ButtonWidget m_TabWorld;
	protected ButtonWidget m_RefreshButton;
	protected ButtonWidget m_LoadButton;
	protected ButtonWidget m_CreateButton;
	protected ButtonWidget m_SaveButton;
	protected ButtonWidget m_ApplyButton;

	protected Widget m_JobTypeBlock;
	protected TextWidget m_JobTypeText;
	protected ButtonWidget m_JobTypeSorting;
	protected ButtonWidget m_JobTypeLoading;
	protected ButtonWidget m_JobTypeMessenger;
	protected ButtonWidget m_JobTypeGuard;
	protected ButtonWidget m_JobTypeCollect;

	protected TextWidget m_SectionTitle;
	protected TextWidget m_RecordIdText;
	protected TextWidget m_StatusText;
	protected TextListboxWidget m_RecordList;

	protected MultilineEditBoxWidget m_DescriptionEditor;
	protected Widget m_DescriptionBlock;

	protected ref array<Widget> m_FieldRows;
	protected ref array<TextWidget> m_FieldLabels;
	protected ref array<EditBoxWidget> m_FieldEdits;
	protected ref array<CheckBoxWidget> m_FieldChecks;
	protected ref array<ButtonWidget> m_FieldActions;
	protected ref array<int> m_FieldActionTypes;

	protected Widget m_ListABlock;
	protected Widget m_ListBBlock;
	protected TextWidget m_ListALabel;
	protected TextWidget m_ListBLabel;
	protected MultilineEditBoxWidget m_ListAEditor;
	protected MultilineEditBoxWidget m_ListBEditor;
	protected ButtonWidget m_ListAClassButton;
	protected ButtonWidget m_ListACopyPlayerButton;
	protected ButtonWidget m_ListBClassButton;

	protected Widget m_ClassSelectorOverlay;
	protected TextWidget m_ClassSelectorTitle;
	protected EditBoxWidget m_ClassSearchEdit;
	protected TextListboxWidget m_ClassList;
	protected ButtonWidget m_ClassSearchButton;
	protected ButtonWidget m_ClassUseButton;
	protected ButtonWidget m_ClassCloseButton;
	protected ref array<string> m_AllClassNames;
	protected ref array<string> m_VisibleClassNames;
	protected int m_ClassTargetRow;
	protected int m_ClassTargetList;
	protected string m_ClassFilterMode;

	protected Widget m_CreateOverlay;
	protected Widget m_CreateWorldTypeBlock;
	protected TextWidget m_CreateTitle;
	protected TextWidget m_CreateTypeText;
	protected EditBoxWidget m_CreateIdEdit;
	protected ButtonWidget m_CreateTypeZone;
	protected ButtonWidget m_CreateTypePile;
	protected ButtonWidget m_CreateTypeLoader;
	protected ButtonWidget m_CreateTypeGuard;
	protected ButtonWidget m_CreateTypeLocker;
	protected ButtonWidget m_CreateConfirmButton;
	protected ButtonWidget m_CreateCancelButton;
	protected string m_CreateWorldType;

	protected ref array<string> m_JobFiles;
	protected ref array<string> m_NpcFiles;
	protected ref array<string> m_WorldTypes;
	protected ref array<string> m_WorldIds;
	protected ref array<string> m_WorldNames;
	protected ref array<string> m_CurrentFields;

	protected int m_CurrentTab;
	protected string m_CurrentKey;
	protected string m_CurrentWorldType;
	protected string m_CurrentWorldId;
	protected string m_CurrentJobType;

	protected static const int JOBSMOD_CF_RPC_ID = 10042;

	protected void SendJobsModAdminRpc(string functionName, Param payload = null)
	{
		array<ref Param> rpcData = new array<ref Param>;
		rpcData.Insert(new Param2<string, string>("RPC_JobsModAdmin", functionName));

		if (payload)
			rpcData.Insert(payload);

		GetGame().RPC(NULL, JOBSMOD_CF_RPC_ID, rpcData, true);
	}

	void MenuJobsModAdmin()
	{
		m_FieldRows = new array<Widget>;
		m_FieldLabels = new array<TextWidget>;
		m_FieldEdits = new array<EditBoxWidget>;
		m_FieldChecks = new array<CheckBoxWidget>;
		m_FieldActions = new array<ButtonWidget>;
		m_FieldActionTypes = new array<int>;
		m_AllClassNames = new array<string>;
		m_VisibleClassNames = new array<string>;
		m_JobFiles = new array<string>;
		m_NpcFiles = new array<string>;
		m_WorldTypes = new array<string>;
		m_WorldIds = new array<string>;
		m_WorldNames = new array<string>;
		m_CurrentFields = new array<string>;

		GetRPCManager().AddRPC("RPC_JobsModAdminClient", "OnCatalog", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("RPC_JobsModAdminClient", "OnForm", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("RPC_JobsModAdminClient", "OnAdminMessage", this, SingleplayerExecutionType.Client);
	}

	override void OnCreate(Widget RootW)
	{
		super.OnCreate(RootW);

		M_SUB_WIDGET = CreateWidgets(LAYOUT_PATH);
		if (!M_SUB_WIDGET)
		{
			Print("[JobsMod/Admin] ERROR: layout not created: " + LAYOUT_PATH);
			return;
		}

		M_SUB_WIDGET.SetHandler(this);
		m_TitlePanel = Widget.Cast(M_SUB_WIDGET.FindAnyWidget("Header"));
		m_closeButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClose"));

		m_TabSettings = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabSettings"));
		m_TabJobs = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabJobs"));
		m_TabNpcs = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabNpcs"));
		m_TabWorld = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabWorld"));
		m_RefreshButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnRefresh"));
		m_LoadButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnLoad"));
		m_CreateButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnCreate"));
		m_SaveButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSave"));
		m_ApplyButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnApply"));

		m_JobTypeBlock = M_SUB_WIDGET.FindAnyWidget("JobTypeBlock");
		m_JobTypeText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("JobTypeText"));
		m_JobTypeSorting = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("JobTypeSorting"));
		m_JobTypeLoading = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("JobTypeLoading"));
		m_JobTypeMessenger = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("JobTypeMessenger"));
		m_JobTypeGuard = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("JobTypeGuard"));
		m_JobTypeCollect = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("JobTypeCollect"));

		m_SectionTitle = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SectionTitle"));
		m_RecordIdText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RecordIdText"));
		m_StatusText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("StatusText"));
		m_RecordList = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RecordList"));

		m_DescriptionBlock = M_SUB_WIDGET.FindAnyWidget("DescriptionBlock");
		m_DescriptionEditor = MultilineEditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("DescriptionEditor"));

		m_ListABlock = M_SUB_WIDGET.FindAnyWidget("ListABlock");
		m_ListBBlock = M_SUB_WIDGET.FindAnyWidget("ListBBlock");
		m_ListALabel = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ListALabel"));
		m_ListBLabel = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ListBLabel"));
		m_ListAEditor = MultilineEditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ListAEditor"));
		m_ListBEditor = MultilineEditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ListBEditor"));
		m_ListAClassButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnListAClass"));
		m_ListACopyPlayerButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnListACopyPlayer"));
		m_ListBClassButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnListBClass"));

		m_ClassSelectorOverlay = M_SUB_WIDGET.FindAnyWidget("ClassSelectorOverlay");
		m_ClassSelectorTitle = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ClassSelectorTitle"));
		m_ClassSearchEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ClassSearchEdit"));
		m_ClassList = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ClassList"));
		m_ClassSearchButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClassSearch"));
		m_ClassUseButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClassUse"));
		m_ClassCloseButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClassClose"));

		m_CreateOverlay = M_SUB_WIDGET.FindAnyWidget("CreateOverlay");
		m_CreateWorldTypeBlock = M_SUB_WIDGET.FindAnyWidget("CreateWorldTypeBlock");
		m_CreateTitle = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTitle"));
		m_CreateTypeText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTypeText"));
		m_CreateIdEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateIdEdit"));
		m_CreateTypeZone = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTypeZone"));
		m_CreateTypePile = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTypePile"));
		m_CreateTypeLoader = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTypeLoader"));
		m_CreateTypeGuard = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTypeGuard"));
		m_CreateTypeLocker = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CreateTypeLocker"));
		m_CreateConfirmButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnCreateConfirm"));
		m_CreateCancelButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnCreateCancel"));

		for (int i = 0; i < 12; i++)
		{
			m_FieldRows.Insert(M_SUB_WIDGET.FindAnyWidget("FieldRow" + i.ToString()));
			m_FieldLabels.Insert(TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("FieldLabel" + i.ToString())));
			m_FieldEdits.Insert(EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("FieldEdit" + i.ToString())));
			m_FieldChecks.Insert(CheckBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("FieldCheck" + i.ToString())));
			m_FieldActions.Insert(ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("FieldAction" + i.ToString())));
			m_FieldActionTypes.Insert(FIELD_ACTION_NONE);
		}

		if (!ValidateWidgets())
			return;

		if (m_ApplyButton)
			m_ApplyButton.Show(false);

		m_CurrentTab = TAB_SETTINGS;
		SetStatus("Запрашиваю конфигурацию JobsMod...");
		RequestCatalog();
	}

	protected bool ValidateWidgets()
	{
		if (!m_TitlePanel || !m_closeButton)
		{
			Print("[JobsMod/Admin] ERROR: Header/BtnClose missing");
			return false;
		}

		if (!m_RecordList || !m_StatusText || !m_LoadButton)
		{
			Print("[JobsMod/Admin] ERROR: main form widgets missing group 1");
			return false;
		}

		if (!m_SaveButton || !m_ApplyButton || !m_CreateButton)
		{
			Print("[JobsMod/Admin] ERROR: main form widgets missing group 2");
			return false;
		}

		if (!m_ClassSelectorOverlay || !m_ClassList || !m_ClassSearchEdit)
		{
			Print("[JobsMod/Admin] ERROR: class selector widgets missing");
			return false;
		}

		if (!m_CreateOverlay || !m_CreateIdEdit || !m_CreateConfirmButton)
		{
			Print("[JobsMod/Admin] ERROR: create widgets missing");
			return false;
		}

		if (!m_JobTypeBlock || !m_JobTypeText || !m_JobTypeSorting)
		{
			Print("[JobsMod/Admin] ERROR: job type widgets missing group 1");
			return false;
		}

		if (!m_JobTypeLoading || !m_JobTypeMessenger || !m_JobTypeGuard)
		{
			Print("[JobsMod/Admin] ERROR: job type widgets missing group 2");
			return false;
		}

		if (!m_JobTypeCollect)
		{
			Print("[JobsMod/Admin] ERROR: job type widgets missing group 3");
			return false;
		}

		for (int i = 0; i < 12; i++)
		{
			if (!m_FieldRows.Get(i) || !m_FieldLabels.Get(i))
			{
				Print("[JobsMod/Admin] ERROR: form row missing group 1: " + i.ToString());
				return false;
			}

			if (!m_FieldEdits.Get(i) || !m_FieldChecks.Get(i) || !m_FieldActions.Get(i))
			{
				Print("[JobsMod/Admin] ERROR: form row missing group 2: " + i.ToString());
				return false;
			}
		}

		return true;
	}

	protected void SetStatus(string text)
	{
		if (m_StatusText)
			m_StatusText.SetText(text);
	}

	protected void RequestCatalog()
	{
		SendJobsModAdminRpc("GetCatalog", null);
	}

	protected void RequestSettings()
	{
		SendJobsModAdminRpc("LoadSettingsForm", null);
	}

	protected void RequestJob(string fileName)
	{
		SendJobsModAdminRpc("LoadJobForm", new Param1<string>(fileName));
	}

	protected void RequestNpc(string fileName)
	{
		SendJobsModAdminRpc("LoadNpcForm", new Param1<string>(fileName));
	}

	protected void RequestWorld(string worldType, string worldId)
	{
		SendJobsModAdminRpc("LoadWorldForm", new Param2<string, string>(worldType, worldId));
	}

	protected void ClearForm()
	{
		m_CurrentKey = "";
		m_CurrentWorldType = "";
		m_CurrentWorldId = "";
		m_CurrentJobType = "";

		if (m_JobTypeBlock)
			m_JobTypeBlock.Show(false);

		if (m_RecordIdText)
			m_RecordIdText.SetText("—");

		if (m_DescriptionEditor)
			m_DescriptionEditor.SetText("");

		if (m_DescriptionBlock)
			m_DescriptionBlock.Show(false);

		if (m_ListAEditor)
			m_ListAEditor.SetText("");

		if (m_ListBEditor)
			m_ListBEditor.SetText("");

		if (m_ListABlock)
			m_ListABlock.Show(false);

		if (m_ListBBlock)
			m_ListBBlock.Show(false);

		if (m_ListAClassButton)
			m_ListAClassButton.Show(false);

		if (m_ListACopyPlayerButton)
			m_ListACopyPlayerButton.Show(false);

		if (m_ListBClassButton)
			m_ListBClassButton.Show(false);

		for (int i = 0; i < 12; i++)
		{
			m_FieldRows.Get(i).Show(false);
			m_FieldEdits.Get(i).Show(true);
			m_FieldChecks.Get(i).Show(false);
			m_FieldEdits.Get(i).SetText("");
			m_FieldChecks.Get(i).SetChecked(false);
			m_FieldActions.Get(i).Show(false);
			m_FieldActionTypes.Set(i, FIELD_ACTION_NONE);
		}
	}

	protected void ShowEditRow(int row, string label, string value)
	{
		m_FieldRows.Get(row).Show(true);
		m_FieldLabels.Get(row).SetText(label);
		m_FieldEdits.Get(row).SetText(value);
		m_FieldEdits.Get(row).Show(true);
		m_FieldChecks.Get(row).Show(false);
		m_FieldActions.Get(row).Show(false);
		m_FieldActionTypes.Set(row, FIELD_ACTION_NONE);
	}

	protected void ShowEditRowAction(int row, string label, string value, int actionType, string actionText)
	{
		ShowEditRow(row, label, value);
		m_FieldActionTypes.Set(row, actionType);
		m_FieldActions.Get(row).SetText(actionText);
		m_FieldActions.Get(row).Show(true);
	}

	protected void ShowCheckRow(int row, string label, bool checked)
	{
		m_FieldRows.Get(row).Show(true);
		m_FieldLabels.Get(row).SetText(label);
		m_FieldEdits.Get(row).Show(false);
		m_FieldChecks.Get(row).Show(true);
		m_FieldChecks.Get(row).SetChecked(checked);
		m_FieldActions.Get(row).Show(false);
		m_FieldActionTypes.Set(row, FIELD_ACTION_NONE);
	}

	protected void ShowDescription(string value)
	{
		m_DescriptionBlock.Show(true);
		m_DescriptionEditor.SetText(value);
	}

	protected void ShowListA(string label, string value)
	{
		m_ListABlock.Show(true);
		m_ListALabel.SetText(label);
		m_ListAEditor.SetText(value);
	}

	protected void ShowListB(string label, string value)
	{
		m_ListBBlock.Show(true);
		m_ListBLabel.SetText(label);
		m_ListBEditor.SetText(value);
	}

	protected string GetField(array<string> fields, string key)
	{
		for (int i = 0; i + 1 < fields.Count(); i = i + 2)
		{
			if (fields.Get(i) == key)
				return fields.Get(i + 1);
		}

		return "";
	}

	protected bool GetBoolField(array<string> fields, string key)
	{
		string value = GetField(fields, key);
		return value == "1" || value == "true";
	}

	protected void PushField(out array<string> fields, string key, string value)
	{
		fields.Insert(key);
		fields.Insert(value);
	}

	protected void PushList(out array<string> fields, string key, string text)
	{
		array<string> lines = new array<string>;
		text.Split("\n", lines);

		foreach (string line : lines)
		{
			line.TrimInPlace();
			if (line != "")
				PushField(fields, key, line);
		}
	}

	protected string JoinRepeated(array<string> fields, string key)
	{
		string result = "";
		for (int i = 0; i + 1 < fields.Count(); i = i + 2)
		{
			if (fields.Get(i) != key)
				continue;

			if (result != "")
				result = result + "\n";

			result = result + fields.Get(i + 1);
		}

		return result;
	}

	protected string EditValue(int row)
	{
		return m_FieldEdits.Get(row).GetText();
	}

	protected string DescriptionValue()
	{
		string value;
		m_DescriptionEditor.GetText(value);
		return value;
	}

	protected string ListAValue()
	{
		string value;
		m_ListAEditor.GetText(value);
		return value;
	}

	protected string ListBValue()
	{
		string value;
		m_ListBEditor.GetText(value);
		return value;
	}

	protected void SelectTab(int tab)
	{
		m_CurrentTab = tab;
		ClearForm();
		RefreshListForTab();

		if (tab == TAB_SETTINGS)
		{
			m_RecordList.Show(false);
			m_SectionTitle.SetText("ОБЩИЕ НАСТРОЙКИ");
			RequestSettings();
			return;
		}

		m_RecordList.Show(true);

		if (tab == TAB_JOBS)
			m_SectionTitle.SetText("РАБОТЫ");
		else if (tab == TAB_NPCS)
			m_SectionTitle.SetText("NPC");
		else
			m_SectionTitle.SetText("МИР / ЗОНЫ / ТОЧКИ");
	}

	protected void RefreshListForTab()
	{
		m_RecordList.ClearItems();

		if (m_CurrentTab == TAB_JOBS)
		{
			foreach (string jobFile : m_JobFiles)
				m_RecordList.AddItem(jobFile, null, 0);
			return;
		}

		if (m_CurrentTab == TAB_NPCS)
		{
			foreach (string npcFile : m_NpcFiles)
				m_RecordList.AddItem(npcFile, null, 0);
			return;
		}

		if (m_CurrentTab == TAB_WORLD)
		{
			for (int i = 0; i < m_WorldIds.Count(); i++)
			{
				string title = "[" + m_WorldTypes.Get(i) + "] " + m_WorldIds.Get(i);
				if (m_WorldNames.Get(i) != "")
					title = title + " — " + m_WorldNames.Get(i);

				m_RecordList.AddItem(title, null, 0);
			}
		}
	}

	protected void LoadSelectedRecord()
	{
		int row = m_RecordList.GetSelectedRow();
		if (row < 0)
		{
			SetStatus("Выберите запись слева.");
			return;
		}

		if (m_CurrentTab == TAB_JOBS)
		{
			if (row >= m_JobFiles.Count())
				return;

			RequestJob(m_JobFiles.Get(row));
			return;
		}

		if (m_CurrentTab == TAB_NPCS)
		{
			if (row >= m_NpcFiles.Count())
				return;

			RequestNpc(m_NpcFiles.Get(row));
			return;
		}

		if (m_CurrentTab == TAB_WORLD)
		{
			if (row >= m_WorldIds.Count())
				return;

			RequestWorld(m_WorldTypes.Get(row), m_WorldIds.Get(row));
		}
	}

	protected void ConfigureSettings(array<string> fields)
	{
		ClearForm();
		m_CurrentKey = "settings";
		m_RecordIdText.SetText("settings.json");
		ShowEditRow(0, "Респавн мусора, сек.", GetField(fields, "pile_respawn_seconds"));
		ShowEditRow(1, "Таймаут работы, сек.", GetField(fields, "assignment_timeout_seconds"));
		ShowCheckRow(2, "Debug logging", GetBoolField(fields, "debug_logging"));
		ShowEditRowAction(3, "Класс награды / валюты", GetField(fields, "reward_class"), FIELD_ACTION_CLASS, "КЛАСС");
	}

	protected void ConfigureJob(string fileName, array<string> fields)
	{
		ClearForm();
		m_CurrentKey = fileName;
		m_RecordIdText.SetText(GetField(fields, "id"));
		ShowEditRow(0, "Название", GetField(fields, "name"));
		ShowDescription(GetField(fields, "description"));
		m_JobTypeBlock.Show(true);
		m_CurrentJobType = GetField(fields, "type");
		m_JobTypeText.SetText("Тип: " + m_CurrentJobType);
		ShowEditRow(2, "Zone ID", GetField(fields, "zone_id"));
		ShowEditRow(3, "Награда", GetField(fields, "reward"));
		ShowEditRow(4, "Cooldown, сек.", GetField(fields, "cooldown_seconds"));
		ShowEditRow(8, "Equipment locker ID", GetField(fields, "equipment_locker_id"));
		ShowEditRow(9, "Штраф за снаряжение", GetField(fields, "equipment_fine"));
		ShowListA("Снаряжение, по одному classname на строку", JoinRepeated(fields, "equipment"));
		m_ListAClassButton.Show(true);

		ConfigureJobTypeFields(m_CurrentJobType, fields);
	}

	protected void ConfigureJobTypeFields(string jobType, array<string> fields)
	{
		m_CurrentJobType = jobType;
		m_JobTypeText.SetText("Тип: " + jobType);

		m_FieldRows.Get(5).Show(false);
		m_FieldRows.Get(6).Show(false);
		m_FieldRows.Get(7).Show(false);
		m_ListBBlock.Show(false);

		if (jobType == "sorting")
		{
			ShowEditRow(5, "Куч мусора (для текущего режима = 1)", GetField(fields, "piles_required"));
		}
		else if (jobType == "loading")
		{
			ShowEditRow(5, "Loader area ID", GetField(fields, "loader_area_id"));
			ShowEditRow(6, "Количество грузов", GetField(fields, "cargos_required"));
			ShowEditRowAction(7, "Cargo classname", GetField(fields, "cargo_class"), FIELD_ACTION_CLASS, "КЛАСС");
		}
		else if (jobType == "messenger")
		{
			ShowEditRow(5, "Target NPC ID", GetField(fields, "target_npc_id"));
			ShowEditRowAction(6, "Package classname", GetField(fields, "package_class"), FIELD_ACTION_CLASS, "КЛАСС");
		}
		else if (jobType == "guard")
		{
			ShowEditRow(5, "Guard post ID", GetField(fields, "guard_post_id"));
			ShowEditRow(6, "Длительность смены, сек.", GetField(fields, "guard_seconds"));
		}
		else if (jobType == "collect")
		{
			ShowEditRow(5, "Количество предметов", GetField(fields, "collect_required"));
			ShowEditRow(6, "Подпись цели", GetField(fields, "collect_label"));
			ShowListB("Что засчитывается, по classname на строку", JoinRepeated(fields, "collect_classes"));
			m_ListBClassButton.Show(true);
		}
	}

	protected void ChangeJobType(string jobType)
	{
		array<string> emptyFields = new array<string>;
		ConfigureJobTypeFields(jobType, emptyFields);
	}

	protected void ConfigureNpc(string fileName, array<string> fields)
	{
		ClearForm();
		m_CurrentKey = fileName;
		m_RecordIdText.SetText(GetField(fields, "id"));
		ShowEditRow(0, "Имя NPC", GetField(fields, "name"));
		ShowDescription(GetField(fields, "description"));
		ShowEditRowAction(1, "Позиция X Y Z", GetField(fields, "position"), FIELD_ACTION_POSITION, "МОЯ ПОЗ.");
		ShowEditRow(2, "Поворот, градусов", GetField(fields, "rotation"));
		ShowEditRowAction(3, "Survivor classname", GetField(fields, "player_class"), FIELD_ACTION_CLASS, "КЛАСС");
		ShowCheckRow(4, "Неуязвимый", GetBoolField(fields, "invulnerable"));
		ShowListA("Одежда, по classname на строку", JoinRepeated(fields, "clothing"));
		m_ListAClassButton.Show(true);
		m_ListACopyPlayerButton.Show(true);
		ShowListB("Работы NPC, по job id на строку", JoinRepeated(fields, "jobs"));
	}

	protected void ConfigureWorld(string key, array<string> fields)
	{
		ClearForm();
		m_CurrentWorldType = GetField(fields, "world_type");
		m_CurrentWorldId = GetField(fields, "id");
		m_CurrentKey = key;
		m_RecordIdText.SetText(m_CurrentWorldType + ": " + m_CurrentWorldId);

		if (m_CurrentWorldType == "zone")
		{
			ShowEditRow(0, "Название зоны", GetField(fields, "name"));
			return;
		}

		if (m_CurrentWorldType == "pile")
		{
			ShowEditRow(0, "Название точки", GetField(fields, "name"));
			ShowEditRow(1, "Zone ID", GetField(fields, "zone_id"));
			ShowEditRowAction(2, "Позиция X Y Z", GetField(fields, "position"), FIELD_ACTION_POSITION, "МОЯ ПОЗ.");
			return;
		}

		if (m_CurrentWorldType == "loader")
		{
			ShowEditRow(0, "Название маршрута", GetField(fields, "name"));
			ShowEditRow(1, "Zone ID", GetField(fields, "zone_id"));
			ShowEditRowAction(2, "Источник X Y Z", GetField(fields, "source"), FIELD_ACTION_POSITION, "МОЯ ПОЗ.");
			ShowEditRow(3, "Радиус источника", GetField(fields, "source_radius"));
			ShowEditRowAction(4, "Назначение X Y Z", GetField(fields, "destination"), FIELD_ACTION_POSITION, "МОЯ ПОЗ.");
			ShowEditRow(5, "Радиус назначения", GetField(fields, "destination_radius"));
			return;
		}

		if (m_CurrentWorldType == "guard")
		{
			ShowEditRow(0, "Название поста", GetField(fields, "name"));
			ShowEditRow(1, "Zone ID", GetField(fields, "zone_id"));
			ShowEditRowAction(2, "Позиция X Y Z", GetField(fields, "position"), FIELD_ACTION_POSITION, "МОЯ ПОЗ.");
			ShowEditRow(3, "Радиус", GetField(fields, "radius"));
			return;
		}

		if (m_CurrentWorldType == "locker")
		{
			ShowEditRow(0, "Название шкафчика", GetField(fields, "name"));
			ShowEditRowAction(1, "Позиция X Y Z", GetField(fields, "position"), FIELD_ACTION_POSITION, "МОЯ ПОЗ.");
			ShowEditRow(2, "Поворот, градусов", GetField(fields, "rotation"));
			ShowEditRowAction(3, "Container classname", GetField(fields, "class_name"), FIELD_ACTION_CLASS, "КЛАСС");
		}
	}

	protected array<string> BuildSettingsFields()
	{
		array<string> fields = new array<string>;
		PushField(fields, "pile_respawn_seconds", EditValue(0));
		PushField(fields, "assignment_timeout_seconds", EditValue(1));
		if (m_FieldChecks.Get(2).IsChecked())
			PushField(fields, "debug_logging", "true");
		else
			PushField(fields, "debug_logging", "false");
		PushField(fields, "reward_class", EditValue(3));
		return fields;
	}

	protected array<string> BuildJobFields()
	{
		array<string> fields = new array<string>;
		PushField(fields, "name", EditValue(0));
		PushField(fields, "description", DescriptionValue());
		PushField(fields, "type", m_CurrentJobType);
		PushField(fields, "zone_id", EditValue(2));
		PushField(fields, "reward", EditValue(3));
		PushField(fields, "cooldown_seconds", EditValue(4));
		PushField(fields, "equipment_locker_id", EditValue(8));
		PushField(fields, "equipment_fine", EditValue(9));
		PushList(fields, "equipment", ListAValue());

		string type = m_CurrentJobType;
		if (type == "sorting")
			PushField(fields, "piles_required", EditValue(5));
		else if (type == "loading")
		{
			PushField(fields, "loader_area_id", EditValue(5));
			PushField(fields, "cargos_required", EditValue(6));
			PushField(fields, "cargo_class", EditValue(7));
		}
		else if (type == "messenger")
		{
			PushField(fields, "target_npc_id", EditValue(5));
			PushField(fields, "package_class", EditValue(6));
		}
		else if (type == "guard")
		{
			PushField(fields, "guard_post_id", EditValue(5));
			PushField(fields, "guard_seconds", EditValue(6));
		}
		else if (type == "collect")
		{
			PushField(fields, "collect_required", EditValue(5));
			PushField(fields, "collect_label", EditValue(6));
			PushList(fields, "collect_classes", ListBValue());
		}

		return fields;
	}

	protected array<string> BuildNpcFields()
	{
		array<string> fields = new array<string>;
		PushField(fields, "name", EditValue(0));
		PushField(fields, "description", DescriptionValue());
		PushField(fields, "position", EditValue(1));
		PushField(fields, "rotation", EditValue(2));
		PushField(fields, "player_class", EditValue(3));
		if (m_FieldChecks.Get(4).IsChecked())
			PushField(fields, "invulnerable", "true");
		else
			PushField(fields, "invulnerable", "false");
		PushList(fields, "clothing", ListAValue());
		PushList(fields, "jobs", ListBValue());
		return fields;
	}

	protected array<string> BuildWorldFields()
	{
		array<string> fields = new array<string>;
		PushField(fields, "name", EditValue(0));

		if (m_CurrentWorldType == "pile")
		{
			PushField(fields, "zone_id", EditValue(1));
			PushField(fields, "position", EditValue(2));
		}
		else if (m_CurrentWorldType == "loader")
		{
			PushField(fields, "zone_id", EditValue(1));
			PushField(fields, "source", EditValue(2));
			PushField(fields, "source_radius", EditValue(3));
			PushField(fields, "destination", EditValue(4));
			PushField(fields, "destination_radius", EditValue(5));
		}
		else if (m_CurrentWorldType == "guard")
		{
			PushField(fields, "zone_id", EditValue(1));
			PushField(fields, "position", EditValue(2));
			PushField(fields, "radius", EditValue(3));
		}
		else if (m_CurrentWorldType == "locker")
		{
			PushField(fields, "position", EditValue(1));
			PushField(fields, "rotation", EditValue(2));
			PushField(fields, "class_name", EditValue(3));
		}

		return fields;
	}

	protected string CurrentPlayerPositionText()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return "";

		vector position = player.GetPosition();
		string result = position[0].ToString();
		result = result + " " + position[1].ToString();
		result = result + " " + position[2].ToString();
		return result;
	}

	protected void PutCurrentPositionIntoRow(int row)
	{
		string position = CurrentPlayerPositionText();
		if (position == "")
		{
			SetStatus("Не удалось получить позицию персонажа.");
			return;
		}

		m_FieldEdits.Get(row).SetText(position);
		SetStatus("Позиция вставлена: " + position);
	}

	protected bool ConfigInheritsFrom(string className, string baseClass)
	{
		if (className == baseClass)
			return true;

		string current = className;
		string parent;
		int guard = 0;

		while (guard < 64)
		{
			if (!GetGame().ConfigGetBaseName("CfgVehicles " + current, parent))
				return false;

			if (parent == baseClass)
				return true;

			if (parent == "" || parent == current)
				return false;

			current = parent;
			guard++;
		}

		return false;
	}

	protected void BuildClassCatalog()
	{
		if (m_AllClassNames.Count() > 0)
			return;

		int count = GetGame().ConfigGetChildrenCount("CfgVehicles");
		for (int i = 0; i < count; i++)
		{
			string className;
			if (!GetGame().ConfigGetChildName("CfgVehicles", i, className))
				continue;

			if (className == "")
				continue;

			int scope = GetGame().ConfigGetInt("CfgVehicles " + className + " scope");
			if (scope != 2)
				continue;

			m_AllClassNames.Insert(className);
		}
	}

	protected bool ClassMatchesMode(string className)
	{
		if (m_ClassFilterMode == "survivor")
			return ConfigInheritsFrom(className, "SurvivorBase");

		return true;
	}

	protected void RefreshClassSelector()
	{
		BuildClassCatalog();
		m_ClassList.ClearItems();
		m_VisibleClassNames.Clear();

		string query = m_ClassSearchEdit.GetText();
		int shown = 0;

		foreach (string className : m_AllClassNames)
		{
			if (!ClassMatchesMode(className))
				continue;

			if (query != "" && className.IndexOf(query) == -1)
				continue;

			m_VisibleClassNames.Insert(className);
			m_ClassList.AddItem(className, null, 0);
			shown++;

			if (shown >= 600)
				break;
		}

		m_ClassSelectorTitle.SetText("ВЫБОР CLASSNAME — найдено " + shown.ToString());
	}

	protected void OpenClassSelectorForRow(int row)
	{
		m_ClassTargetRow = row;
		m_ClassTargetList = 0;
		m_ClassFilterMode = "";

		if (m_CurrentTab == TAB_NPCS && row == 3)
			m_ClassFilterMode = "survivor";

		m_ClassSearchEdit.SetText("");
		m_ClassSelectorOverlay.Show(true);
		RefreshClassSelector();
	}

	protected void OpenClassSelectorForList(int listIndex)
	{
		m_ClassTargetRow = -1;
		m_ClassTargetList = listIndex;
		m_ClassFilterMode = "";
		m_ClassSearchEdit.SetText("");
		m_ClassSelectorOverlay.Show(true);
		RefreshClassSelector();
	}

	protected void AppendLineToEditor(MultilineEditBoxWidget editor, string value)
	{
		if (!editor || value == "")
			return;

		string current;
		editor.GetText(current);
		if (current != "" && current.Get(current.Length() - 1) != "\n")
			current = current + "\n";

		editor.SetText(current + value);
	}

	protected void UseSelectedClass()
	{
		int row = m_ClassList.GetSelectedRow();
		if (row < 0 || row >= m_VisibleClassNames.Count())
		{
			SetStatus("Выберите classname в списке.");
			return;
		}

		string className = m_VisibleClassNames.Get(row);
		if (m_ClassTargetList == 1)
			AppendLineToEditor(m_ListAEditor, className);
		else if (m_ClassTargetList == 2)
			AppendLineToEditor(m_ListBEditor, className);
		else if (m_ClassTargetRow >= 0)
			m_FieldEdits.Get(m_ClassTargetRow).SetText(className);

		m_ClassSelectorOverlay.Show(false);
		SetStatus("Выбран classname: " + className);
	}

	protected void CopyPlayerOutfit()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
		{
			SetStatus("Не удалось получить персонажа администратора.");
			return;
		}

		string result = "";
		int count = player.GetInventory().AttachmentCount();
		for (int i = 0; i < count; i++)
		{
			EntityAI attachment = player.GetInventory().GetAttachmentFromIndex(i);
			if (!attachment)
				continue;

			string className = attachment.GetType();
			bool wearable = ConfigInheritsFrom(className, "Clothing");
			if (!wearable)
				wearable = ConfigInheritsFrom(className, "Bag_Base");

			if (!wearable)
				continue;

			if (result != "")
				result = result + "\n";

			result = result + className;
		}

		m_ListAEditor.SetText(result);
		SetStatus("Одежда скопирована с вашего персонажа.");
	}

	protected void OpenCreateDialog()
	{
		if (m_CurrentTab == TAB_SETTINGS)
		{
			SetStatus("Для общих настроек новая запись не требуется.");
			return;
		}

		m_CreateIdEdit.SetText("");
		m_CreateWorldType = "pile";
		m_CreateWorldTypeBlock.Show(m_CurrentTab == TAB_WORLD);

		if (m_CurrentTab == TAB_JOBS)
			m_CreateTitle.SetText("НОВЫЙ ФАЙЛ РАБОТЫ");
		else if (m_CurrentTab == TAB_NPCS)
			m_CreateTitle.SetText("НОВЫЙ ФАЙЛ NPC");
		else
			m_CreateTitle.SetText("НОВАЯ ЗАПИСЬ МИРА");

		UpdateCreateTypeText();
		m_CreateOverlay.Show(true);
	}

	protected void UpdateCreateTypeText()
	{
		if (m_CurrentTab == TAB_WORLD)
			m_CreateTypeText.SetText("Тип: " + m_CreateWorldType + ". Позиция берётся с вашего персонажа.");
		else
			m_CreateTypeText.SetText("Введите ID без .json");
	}

	protected void SetCreateWorldType(string worldType)
	{
		m_CreateWorldType = worldType;
		UpdateCreateTypeText();
	}

	protected void ConfirmCreateRecord()
	{
		string id = m_CreateIdEdit.GetText();
		id.TrimInPlace();
		if (id == "")
		{
			SetStatus("Введите ID новой записи.");
			return;
		}

		string position = CurrentPlayerPositionText();
		SendJobsModAdminRpc(
			"CreateRecord",
			new Param4<int, string, string, string>(m_CurrentTab, m_CreateWorldType, id, position)
		);
		m_CreateOverlay.Show(false);
		SetStatus("Создание: " + id + "...");
	}

	protected void SaveCurrentForm()
	{
		if (m_CurrentTab == TAB_SETTINGS)
		{
			SendJobsModAdminRpc("SaveSettingsForm", new Param1<ref array<string>>(BuildSettingsFields()));
			return;
		}

		if (m_CurrentKey == "")
		{
			SetStatus("Сначала выберите запись.");
			return;
		}

		if (m_CurrentTab == TAB_JOBS)
		{
			SendJobsModAdminRpc("SaveJobForm", new Param2<string, ref array<string>>(m_CurrentKey, BuildJobFields()));
			return;
		}

		if (m_CurrentTab == TAB_NPCS)
		{
			SendJobsModAdminRpc("SaveNpcForm", new Param2<string, ref array<string>>(m_CurrentKey, BuildNpcFields()));
			return;
		}

		if (m_CurrentTab == TAB_WORLD)
		{
			SendJobsModAdminRpc("SaveWorldForm", new Param3<string, string, ref array<string>>(m_CurrentWorldType, m_CurrentWorldId, BuildWorldFields()));
		}
	}

	protected void ApplyConfig()
	{
		SendJobsModAdminRpc("ApplyConfig", null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (super.OnClick(w, x, y, button))
			return true;

		if (w == m_TabSettings)
		{
			SelectTab(TAB_SETTINGS);
			return true;
		}

		if (w == m_TabJobs)
		{
			SelectTab(TAB_JOBS);
			return true;
		}

		if (w == m_TabNpcs)
		{
			SelectTab(TAB_NPCS);
			return true;
		}

		if (w == m_TabWorld)
		{
			SelectTab(TAB_WORLD);
			return true;
		}

		if (w == m_JobTypeSorting)
		{
			ChangeJobType("sorting");
			return true;
		}

		if (w == m_JobTypeLoading)
		{
			ChangeJobType("loading");
			return true;
		}

		if (w == m_JobTypeMessenger)
		{
			ChangeJobType("messenger");
			return true;
		}

		if (w == m_JobTypeGuard)
		{
			ChangeJobType("guard");
			return true;
		}

		if (w == m_JobTypeCollect)
		{
			ChangeJobType("collect");
			return true;
		}

		if (w == m_RefreshButton)
		{
			RequestCatalog();
			return true;
		}

		if (w == m_LoadButton)
		{
			LoadSelectedRecord();
			return true;
		}

		if (w == m_CreateButton)
		{
			OpenCreateDialog();
			return true;
		}

		for (int fieldIndex = 0; fieldIndex < m_FieldActions.Count(); fieldIndex++)
		{
			if (w != m_FieldActions.Get(fieldIndex))
				continue;

			int actionType = m_FieldActionTypes.Get(fieldIndex);
			if (actionType == FIELD_ACTION_POSITION)
				PutCurrentPositionIntoRow(fieldIndex);
			else if (actionType == FIELD_ACTION_CLASS)
				OpenClassSelectorForRow(fieldIndex);

			return true;
		}

		if (w == m_ListAClassButton)
		{
			OpenClassSelectorForList(1);
			return true;
		}

		if (w == m_ListBClassButton)
		{
			OpenClassSelectorForList(2);
			return true;
		}

		if (w == m_ListACopyPlayerButton)
		{
			CopyPlayerOutfit();
			return true;
		}

		if (w == m_ClassSearchButton)
		{
			RefreshClassSelector();
			return true;
		}

		if (w == m_ClassUseButton)
		{
			UseSelectedClass();
			return true;
		}

		if (w == m_ClassCloseButton)
		{
			m_ClassSelectorOverlay.Show(false);
			return true;
		}

		if (w == m_CreateTypeZone)
		{
			SetCreateWorldType("zone");
			return true;
		}

		if (w == m_CreateTypePile)
		{
			SetCreateWorldType("pile");
			return true;
		}

		if (w == m_CreateTypeLoader)
		{
			SetCreateWorldType("loader");
			return true;
		}

		if (w == m_CreateTypeGuard)
		{
			SetCreateWorldType("guard");
			return true;
		}

		if (w == m_CreateTypeLocker)
		{
			SetCreateWorldType("locker");
			return true;
		}

		if (w == m_CreateConfirmButton)
		{
			ConfirmCreateRecord();
			return true;
		}

		if (w == m_CreateCancelButton)
		{
			m_CreateOverlay.Show(false);
			return true;
		}

		if (w == m_SaveButton)
		{
			SaveCurrentForm();
			return true;
		}

		if (w == m_ApplyButton)
		{
			ApplyConfig();
			return true;
		}

		return false;
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == m_RecordList)
		{
			LoadSelectedRecord();
			return true;
		}

		if (w == m_ClassList)
		{
			UseSelectedClass();
			return true;
		}

		return super.OnDoubleClick(w, x, y, button);
	}

	void OnCatalog(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<ref array<string>> data;
		if (!ctx.Read(data))
			return;

		if (type != CallType.Client || !data.param1)
			return;

		m_JobFiles.Clear();
		m_NpcFiles.Clear();
		m_WorldTypes.Clear();
		m_WorldIds.Clear();
		m_WorldNames.Clear();

		array<string> catalog = data.param1;
		int i = 0;
		while (i < catalog.Count())
		{
			string kind = catalog.Get(i);
			i++;

			if (kind == "job" && i < catalog.Count())
			{
				m_JobFiles.Insert(catalog.Get(i));
				i++;
			}
			else if (kind == "npc" && i < catalog.Count())
			{
				m_NpcFiles.Insert(catalog.Get(i));
				i++;
			}
			else if (kind == "world" && i + 2 < catalog.Count())
			{
				m_WorldTypes.Insert(catalog.Get(i));
				m_WorldIds.Insert(catalog.Get(i + 1));
				m_WorldNames.Insert(catalog.Get(i + 2));
				i = i + 3;
			}
			else
			{
				break;
			}
		}

		RefreshListForTab();
		SetStatus("Каталог обновлён: работ " + m_JobFiles.Count().ToString() + ", NPC " + m_NpcFiles.Count().ToString() + ".");

		if (m_CurrentTab == TAB_SETTINGS)
			RequestSettings();
	}

	void OnForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param3<string, string, ref array<string>> data;
		if (!ctx.Read(data))
			return;

		if (type != CallType.Client || !data.param3)
			return;

		m_CurrentFields.Clear();
		foreach (string value : data.param3)
			m_CurrentFields.Insert(value);

		if (data.param1 == "settings")
		{
			ConfigureSettings(m_CurrentFields);
			SetStatus("Общие настройки загружены.");
		}
		else if (data.param1 == "job")
		{
			ConfigureJob(data.param2, m_CurrentFields);
			SetStatus("Работа загружена: " + data.param2);
		}
		else if (data.param1 == "npc")
		{
			ConfigureNpc(data.param2, m_CurrentFields);
			SetStatus("NPC загружен: " + data.param2);
		}
		else if (data.param1 == "world")
		{
			ConfigureWorld(data.param2, m_CurrentFields);
			SetStatus("Объект мира загружен: " + data.param2);
		}
	}

	void OnAdminMessage(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> data;
		if (!ctx.Read(data))
			return;

		if (type != CallType.Client)
			return;

		SetStatus(data.param1);
		RequestCatalog();
	}
}
