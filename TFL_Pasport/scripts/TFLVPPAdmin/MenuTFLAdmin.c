class MenuTFLAdmin extends AdminHudSubMenu
{
    protected static MenuTFLAdmin s_Menu;
    protected static ref TFLFactionAdminPanelDto s_Panel;

    protected TextWidget m_TitleText;
    protected string m_ActiveSection = "АДМИНИСТРИРОВАНИЕ ФРАКЦИИ";
    protected TextWidget m_OperatorText;
    protected ButtonWidget m_BtnClose;
    protected ButtonWidget m_BtnLicenses;
    protected ButtonWidget m_BtnTerritories;

    protected Widget m_FactionHost;
    protected ScrollWidget m_FactionScroll;
    protected EditBoxWidget m_NewFactionId;
    protected EditBoxWidget m_NewFactionName;
    protected ButtonWidget m_BtnCreateFaction;

    protected ButtonWidget m_BtnFactionTab;
    protected ButtonWidget m_BtnMembersTab;
    protected ButtonWidget m_BtnRolesTab;
    protected ButtonWidget m_BtnRanksTab;
    protected Widget m_FactionSidebar;
    protected TextWidget m_RanksTitle;
    protected TextWidget m_RanksHint;
    protected Widget m_FactionPanel;
    protected Widget m_MembersPanel;
    protected Widget m_RolesPanel;
    protected Widget m_RanksPanel;
    protected Widget m_LicensesPanel;
    protected Widget m_TerritoriesPanel;

    protected TextWidget m_SelectedIdText;
    protected EditBoxWidget m_SelectedNameEdit;
    protected EditBoxWidget m_SelectedShortEdit;
    protected ButtonWidget m_BtnSide;
    protected ButtonWidget m_BtnSaveName;
    protected TextWidget m_SpawnText;
    protected TextWidget m_SpawnRotText;
    protected SliderWidget m_SpawnYawSlider;
    protected TextWidget m_SpawnYawValue;
    protected ButtonWidget m_BtnSpawnHere;
    protected ButtonWidget m_BtnSpawnYawApply;
    protected ButtonWidget m_BtnSpawnClearPoints;
    protected TextWidget m_LaptopText;
    protected TextWidget m_LaptopRotText;
    protected SliderWidget m_LaptopYawSlider;
    protected TextWidget m_LaptopYawValue;
    protected ButtonWidget m_BtnLaptopHere;
    protected ButtonWidget m_BtnLaptopYawApply;
    protected ButtonWidget m_BtnChestHere;
    protected ButtonWidget m_BtnPresetClearInventory;
    protected ButtonWidget m_BtnPresetMe;
    protected Widget m_PresetHost;
    protected ScrollWidget m_PresetScroll;
    protected EditBoxWidget m_PresetClassEdit;
    protected ButtonWidget m_BtnPresetAdd;
    protected ButtonWidget m_BtnPresetRemove;
    protected ButtonWidget m_BtnPresetClear;
    protected TextWidget m_FactionStatus;

    // Какая вкладка сейчас открыта — чтобы ответ сервера попал именно на неё.
    protected static const int TAB_FACTION     = 0;
    protected static const int TAB_MEMBERS     = 1;
    protected static const int TAB_ROLES       = 2;
    protected static const int TAB_RANKS       = 3;
    protected static const int TAB_LICENSES    = 4;
    protected static const int TAB_TERRITORIES = 5;
    protected int m_ActiveTab = TAB_FACTION;
    protected TextWidget m_TerrStatus;

    protected Widget m_AdminMemberHost;
    protected ScrollWidget m_AdminMemberScroll;
    protected TextWidget m_AdminMemberName;
    protected TextWidget m_AdminMemberRole;
    protected TextWidget m_AdminMemberRank;
    protected TextWidget m_AdminMemberUid;
    protected ButtonWidget m_AdminBtnPromote;
    protected ButtonWidget m_AdminBtnDemote;
    protected ButtonWidget m_AdminBtnKick;

    protected Widget m_AdminPeopleHost;
    protected ScrollWidget m_AdminPeopleScroll;
    protected TextWidget m_AdminSelectedPersonName;
    protected TextWidget m_AdminSelectedPersonUid;
    protected TextWidget m_AdminAddRoleText;
    protected ButtonWidget m_AdminBtnAddRolePrev;
    protected ButtonWidget m_AdminBtnAddRoleNext;
    protected TextWidget m_AdminAddSelectedName;
    protected ButtonWidget m_AdminBtnAddSelected;
    protected EditBoxWidget m_AdminAddUidEdit;
    protected ButtonWidget m_AdminBtnAddUid;
    protected TextWidget m_AdminAddStatus;

    protected Widget m_AdminRoleHost;
    protected ScrollWidget m_AdminRoleScroll;
    protected ButtonWidget m_AdminBtnNewRole;
    protected EditBoxWidget m_AdminRoleIdEdit;
    protected EditBoxWidget m_AdminRoleNameEdit;
    protected EditBoxWidget m_AdminRoleLevelEdit;
    protected ButtonWidget m_AdminPermUseLaptop;
    protected ButtonWidget m_AdminPermInvite;
    protected ButtonWidget m_AdminPermKick;
    protected ButtonWidget m_AdminPermPromote;
    protected ButtonWidget m_AdminPermDemote;
    protected ButtonWidget m_AdminPermManageRoles;
    protected ButtonWidget m_AdminPermEditRights;
    protected ButtonWidget m_AdminPermIssueLicenses;
    protected ButtonWidget m_AdminBtnSaveRole;
    protected TextWidget m_AdminRoleStatus;

    protected Widget m_AdminRankHost;
    protected ScrollWidget m_AdminRankScroll;
    protected ButtonWidget m_AdminBtnNewRank;
    protected EditBoxWidget m_AdminRankIdEdit;
    protected EditBoxWidget m_AdminRankNameEdit;
    protected EditBoxWidget m_AdminRankLevelEdit;
    protected EditBoxWidget m_AdminRankSalaryEdit;
    protected ButtonWidget m_AdminRankCurrency;
    protected ButtonWidget m_RankCurRub;
    protected ButtonWidget m_RankCurUsd;
    protected ButtonWidget m_RankCurEur;
    protected ButtonWidget m_AdminBtnSaveRank;
    protected ButtonWidget m_AdminBtnDeleteRank;
    protected TextWidget m_RankMemberText;
    protected TextWidget m_RankMemberCurrent;
    protected TextWidget m_SelectedRankText;
    protected TextWidget m_SelectedPlayerRole;
    protected TextListboxWidget m_RankMemberList;
    protected ButtonWidget m_RankAssignSelected;
    protected ButtonWidget m_SalaryEnabled;
    protected EditBoxWidget m_SalaryBankId;
    protected EditBoxWidget m_SalaryCurrencyId;
    protected TextWidget m_BankAccountText;
    protected EditBoxWidget m_SalaryIntervalMin;
    protected EditBoxWidget m_SalaryGraceMin;
    protected ButtonWidget m_SalarySaveSettings;
    protected TextWidget m_SalaryStatus;
    protected Widget m_RankCombatPanel;
    protected ButtonWidget m_RankTabCombat;
    protected ButtonWidget m_RankTabCivilian;
    protected Widget m_CivilianBenefitPanel;
    protected ButtonWidget m_CivilianBenefitEnabled;
    protected EditBoxWidget m_CivilianBenefitAmount;
    protected ButtonWidget m_CivilianBenefitCurrency;
    protected ButtonWidget m_BenefitCurRub;
    protected ButtonWidget m_BenefitCurUsd;
    protected ButtonWidget m_BenefitCurEur;
    protected EditBoxWidget m_CivilianBenefitInterval;
    protected ButtonWidget m_CivilianBenefitSave;
    protected TextWidget m_CivilianBenefitStatus;
    protected ref array<Widget> m_RankRows;
    protected string m_SelectedRank;
    protected bool m_NewRank;
    protected string m_RankCurrencyId;
    protected int m_SalaryEditEnabled;
    protected int m_CivilianBenefitEditEnabled;
    protected string m_CivilianBenefitCurrencyId;

    protected ref array<Widget> m_FactionRows;
    protected string m_PendingFactionSelection;
    protected ref array<Widget> m_MemberRows;
    protected ref array<Widget> m_PeopleRows;
    protected ref array<Widget> m_RoleRows;
    protected ref array<Widget> m_PresetRows;
    protected string m_SelectedMember;
    protected string m_SelectedPerson;
    protected string m_AddRoleId;
    protected string m_SelectedRole;
    protected string m_SelectedPreset;
    protected bool m_NewRole;
    protected ref TFLFactionRolePermissions m_EditPermissions;
    protected string m_EditSideId;

    protected Widget m_LicLicenseHost;
    protected ScrollWidget m_LicLicenseScroll;
    protected EditBoxWidget m_LicLicenseId;
    protected EditBoxWidget m_LicLicenseName;
    protected MultilineEditBoxWidget m_LicLicenseDescription;
    protected ButtonWidget m_LicActive;
    protected ButtonWidget m_LicNew;
    protected ButtonWidget m_LicSave;
    protected Widget m_LicIssuerHost;
    protected ScrollWidget m_LicIssuerScroll;
    protected ButtonWidget m_LicToggleFaction;
    protected ButtonWidget m_LicDelete;
    protected TextWidget m_LicStatus;
    protected ref array<Widget> m_LicLicenseRows;
    protected ref array<Widget> m_LicIssuerRows;
    protected string m_LicSelectedLicenseId;
    protected string m_LicSelectedFactionId;
    protected int m_LicEditActive;
    protected bool m_LicNewMode;

    protected Widget m_TerrPointMode;
    protected Widget m_TerrMapBg;
    protected TextWidget m_TerritoriesTitle;
    protected TextWidget m_TerritoriesHint;
    protected ButtonWidget m_ModeBtnPoints;
    protected ButtonWidget m_ModeBtnChains;
    protected ButtonWidget m_ModeBtnRewards;
    protected Widget m_TerritoryChainHost;
    protected Widget m_TerritoryRewardHost;
    protected TextWidget m_TerrChainName;
    protected TextWidget m_TerrChainPosition;
    protected TextWidget m_TerrChainPrevious;
    protected TextWidget m_TerrChainNext;
    protected TextWidget m_TerrRewardNameText;
    protected ButtonWidget m_TerrBtnPickReward;
    protected MapWidget m_TerrMap;
    protected Widget m_TerrListHost;
    protected ScrollWidget m_TerrListScroll;
    protected EditBoxWidget m_TerrId;
    protected EditBoxWidget m_TerrName;
    protected TextWidget m_TerrMapPoint;
    protected ButtonWidget m_TerrCreate;
    protected ButtonWidget m_TerrDelete;
    protected EditBoxWidget m_TerrRadius;
    protected EditBoxWidget m_TerrMinPlayers;
    protected EditBoxWidget m_TerrCaptureMin;
    protected EditBoxWidget m_TerrPrepMin;
    protected EditBoxWidget m_TerrCooldownMin;
    protected EditBoxWidget m_TerrParticipation;
    protected EditBoxWidget m_TerrStartHour;
    protected EditBoxWidget m_TerrEndHour;
    protected EditBoxWidget m_TerrDailyLimit;
    protected ButtonWidget m_TerrSaveLimit;
    protected ButtonWidget m_TerrDirectCapture;
    protected ref array<ButtonWidget> m_TerrDays;
    protected EditBoxWidget m_TerrRewardClass;
    protected EditBoxWidget m_TerrRewardCount;
    protected ButtonWidget m_TerrRewardAdd;
    protected Widget m_TerrRewardHost;
    protected ScrollWidget m_TerrRewardScroll;
    protected ButtonWidget m_TerrRewardRemove;
    protected ButtonWidget m_TerrSave;
    protected TextWidget m_TerrChainSummary;
    protected ButtonWidget m_TerrBtnChainEditor;
    protected ButtonWidget m_TerrBtnRewardEditor;

    // Dedicated reward preset editor (NEW_LAYOUTS).
    protected Widget m_RewardPresetPanel;
    protected Widget m_RewardPresetList;
    protected ScrollWidget m_RewardPresetListScroll;
    protected ref array<Widget> m_RewardPresetRows;
    protected ButtonWidget m_RewardPresetNew;
    protected ButtonWidget m_RewardPresetAssign;
    protected EditBoxWidget m_RewardPresetId;
    protected EditBoxWidget m_RewardPresetName;
    protected ButtonWidget m_RewardPresetSave;
    protected ButtonWidget m_RewardPresetDelete;
    protected ButtonWidget m_RewardPresetClose;
    protected ButtonWidget m_RewardPresetWindowClose;
    protected EditBoxWidget m_RewardItemSearch;
    protected ButtonWidget m_RewardItemSearchBtn;
    protected Widget m_RewardSearchResults;
    protected ScrollWidget m_RewardSearchResultsScroll;
    protected ref array<Widget> m_RewardSearchRows;
    protected ItemPreviewWidget m_RewardItemPreview;
    protected TextWidget m_RewardItemPreviewName;
    protected TextWidget m_RewardItemClass;
    protected EditBoxWidget m_RewardItemCount;
    protected ButtonWidget m_RewardItemAdd;
    protected Widget m_RewardPresetItems;
    protected ScrollWidget m_RewardPresetItemsScroll;
    protected ref array<Widget> m_RewardContentRows;
    protected TextWidget m_RewardPresetStatus;
    protected string m_SelectedRewardPresetId;
    protected int m_SelectedRewardItemIndex;
    protected string m_RewardSelectedClassName;
    protected ref array<string> m_RewardSearchClasses;
    protected EntityAI m_RewardPreviewEntity;

    // Dedicated visual frontline-chain editor (NEW_LAYOUTS).
    protected Widget m_ChainPanel;
    protected MapWidget m_ChainMap;
    protected TextListboxWidget m_ChainList;
    protected EditBoxWidget m_ChainId;
    protected EditBoxWidget m_ChainName;
    protected ButtonWidget m_ChainSideA;
    protected ButtonWidget m_ChainSideB;
    protected TextWidget m_ChainSideAText;
    protected TextWidget m_ChainSideBText;
    protected ButtonWidget m_ChainNew;
    protected ButtonWidget m_ChainSave;
    protected ButtonWidget m_ChainDelete;
    protected ButtonWidget m_ChainClose;
    protected ButtonWidget m_ChainWindowClose;
    protected TextWidget m_ChainPath;
    protected Widget m_ChainPointList;
    protected ScrollWidget m_ChainPointScroll;
    protected ref array<Widget> m_ChainPointRows;
    protected string m_SelectedChainId;
    protected string m_ChainSideAId;
    protected string m_ChainSideBId;
    protected int m_SelectedChainPointIndex;
    protected bool m_ChainEditMode;
    protected ref array<string> m_PendingChainPoints;

    protected ref array<Widget> m_TerritoryRows;
    protected ref array<Widget> m_TerrRewardRows;
    protected string m_TerrSelectedId;
    protected int m_TerrRewardSelected;
    protected vector m_TerrClickPosition;
    protected bool m_TerrHaveMapPoint;

    protected ref TFLJsonAssembler m_VppAssembler;
    protected int m_VppRequestAttempts;

    protected void SendVPP(string functionName, Param payload = null)
    {
        // This RPC namespace is registered through Community Framework's RPCManager.
        // Always send through RPCManager as well; raw GetGame().RPC with a custom
        // id bypasses CF and never reaches RequestPanel/Command.
        GetRPCManager().SendRPC("RPC_TFLVPPAdmin", functionName, payload, true);
    }

    protected void RequestPanel(string factionId)
    {
        SendVPP("RequestPanel", new Param1<string>(factionId));
    }

    protected void SendCommand(int command, TFLFactionCommandDto dto = null)
    {
        ref TFLFactionCommandDto safeDto = dto;
        if (!safeDto)
            safeDto = new TFLFactionCommandDto();
        string payload;
        string err;
        if (!JsonFileLoader<TFLFactionCommandDto>.MakeData(safeDto, payload, err, false))
        {
            Print("[TFL/VPP] command serialization failed: " + err);
            return;
        }
        SendVPP("Command", new Param2<int, string>(command, payload));
    }

    void OnVPPPanelChunk(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client)
            return;
        Param3<int, int, string> packet;
        if (!ctx.Read(packet) || !packet)
            return;
        if (!m_VppAssembler)
            m_VppAssembler = new TFLJsonAssembler();
        m_VppRequestAttempts = 0;
        string full = m_VppAssembler.Feed(9001, packet.param1, packet.param2, packet.param3);
        if (full == "")
            return;
        TFLFactionAdminPanelDto panel;
        string err;
        if (!JsonFileLoader<TFLFactionAdminPanelDto>.LoadData(full, panel, err) || !panel)
        {
            Print("[TFL/VPP] admin panel parse failed: " + err);
            return;
        }
        s_Panel = panel;
        if (s_Menu)
        {
            if (s_Menu.m_PendingFactionSelection == panel.selected_id) s_Menu.m_PendingFactionSelection = "";
            s_Menu.FlushPendingChainPoints();
            s_Menu.RefreshAll();
        }
    }

    // Ответ сервера показываем на той вкладке, где админ сейчас находится:
    // на чужой вкладке он невидим, и отказ выглядит как молчащая кнопка.
    protected void SetStatus(string text)
    {
        TextWidget target = m_FactionStatus;
        if (m_ActiveTab == TAB_MEMBERS) target = m_AdminAddStatus;
        else if (m_ActiveTab == TAB_ROLES) target = m_AdminRoleStatus;
        else if (m_ActiveTab == TAB_RANKS) target = m_SalaryStatus;
        else if (m_ActiveTab == TAB_LICENSES) target = m_LicStatus;
        else if (m_ActiveTab == TAB_TERRITORIES) target = m_TerrStatus;
        // Если у вкладки своей строки статуса нет, текст не должен пропасть.
        if (!target) target = m_FactionStatus;
        if (target) target.SetText(text);
    }

    void OnVPPMessage(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client)
            return;
        Param1<string> packet;
        if (!ctx.Read(packet) || !packet)
            return;
        // Ответ сервера — единственное объяснение, почему команда не прошла
        // (например, нет права MenuTFLAdmin:Write). Раньше он попадал только в
        // статус вкладки ФРАКЦИЯ, поэтому на любой другой вкладке отказ выглядел
        // как «кнопка вообще ничего не делает». Пишем во все строки статуса.
        SetStatus(packet.param1);
        Print("[TFL/VPP] " + packet.param1);
    }

    void MenuTFLAdmin()
    {
        m_VppAssembler = new TFLJsonAssembler();
        GetRPCManager().AddRPC("RPC_TFLVPPAdminClient", "OnVPPPanelChunk", this, SingleplayerExecutionType.Client);
        GetRPCManager().AddRPC("RPC_TFLVPPAdminClient", "OnVPPMessage", this, SingleplayerExecutionType.Client);
        m_FactionRows = new array<Widget>();
        m_PendingFactionSelection = "";
        m_MemberRows = new array<Widget>();
        m_PeopleRows = new array<Widget>();
        m_RoleRows = new array<Widget>();
        m_PresetRows = new array<Widget>();
        m_SelectedMember = "";
        m_SelectedPerson = "";
        m_AddRoleId = "";
        m_SelectedRole = "";
        m_SelectedPreset = "";
        m_NewRole = false;
        m_EditPermissions = new TFLFactionRolePermissions();
        m_EditSideId = TFLFactionSide.NEUTRAL;
        m_RankRows = new array<Widget>();
        m_SelectedRank = "";
        m_NewRank = false;
        m_RankCurrencyId = TFLFactionCurrency.RUBLE;
        m_SalaryEditEnabled = 1;
        m_CivilianBenefitEditEnabled = 1;
        m_CivilianBenefitCurrencyId = TFLFactionCurrency.RUBLE;
        m_LicLicenseRows = new array<Widget>();
        m_LicIssuerRows = new array<Widget>();
        m_LicSelectedLicenseId = "";
        m_LicSelectedFactionId = "";
        m_LicEditActive = 1;
        m_LicNewMode = false;

        m_TerrDays = new array<ButtonWidget>();
        m_TerritoryRows = new array<Widget>();
        m_TerrRewardRows = new array<Widget>();
        m_TerrSelectedId = "";
        m_TerrRewardSelected = -1;
        m_TerrClickPosition = "0 0 0";
        m_TerrHaveMapPoint = false;
        m_SelectedRewardPresetId = "";
        m_SelectedRewardItemIndex = -1;
        m_RewardSelectedClassName = "";
        m_RewardSearchClasses = new array<string>();
        m_RewardPresetRows = new array<Widget>();
        m_RewardSearchRows = new array<Widget>();
        m_RewardContentRows = new array<Widget>();
        m_RewardPreviewEntity = null;
        m_SelectedChainId = "";
        m_ChainSideAId = TFLFactionSide.VSRF;
        m_ChainSideBId = TFLFactionSide.TERRORISTS;
        m_SelectedChainPointIndex = -1;
        m_ChainEditMode = false;
        m_ChainPointRows = new array<Widget>();
        m_PendingChainPoints = new array<string>();
    }



    override void OnCreate(Widget RootW)
    {
        super.OnCreate(RootW);

        M_SUB_WIDGET = CreateWidgets("TFL_Pasport/gui/layouts/faction_admin_v2.layout");
        if (!M_SUB_WIDGET)
        {
            TFLFactionLog.Error("Не загружен faction_admin_v2.layout");
            return;
        }

        m_TitleText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("HeaderTitle"));
        m_OperatorText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("HeaderOperator"));
        m_BtnClose = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClose"));
        m_BtnLicenses = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabLicenses"));
        m_BtnTerritories = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabTerritories"));
        m_FactionHost = M_SUB_WIDGET.FindAnyWidget("FactionHost");
        m_FactionScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("FactionScroll"));
        m_NewFactionId = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("NewFactionId"));
        m_NewFactionName = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("NewFactionName"));
        m_BtnCreateFaction = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnCreateFaction"));

        m_BtnFactionTab = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabFaction"));
        m_BtnMembersTab = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabMembers"));
        m_BtnRolesTab = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabRoles"));
        m_BtnRanksTab = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TabRanks"));
        m_FactionSidebar = M_SUB_WIDGET.FindAnyWidget("FactionSidebar");
        m_RanksTitle = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RanksTitle"));
        m_RanksHint = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RanksHint"));
        m_FactionPanel = M_SUB_WIDGET.FindAnyWidget("FactionPanel");
        m_MembersPanel = M_SUB_WIDGET.FindAnyWidget("MembersPanel");
        m_RolesPanel = M_SUB_WIDGET.FindAnyWidget("RolesPanel");
        m_RanksPanel = M_SUB_WIDGET.FindAnyWidget("RanksPanel");

        m_LicensesPanel = M_SUB_WIDGET.FindAnyWidget("LicensesPanel");
        m_TerritoriesPanel = M_SUB_WIDGET.FindAnyWidget("TerritoriesPanel");

        m_SelectedIdText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SelectedIdText"));
        m_SelectedNameEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SelectedNameEdit"));
        m_SelectedShortEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SelectedShortEdit"));
        m_BtnSide = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSide"));
        m_BtnSaveName = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSaveName"));
        m_SpawnText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SpawnText"));
        m_SpawnRotText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SpawnRotText"));
        m_SpawnYawSlider = SliderWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SpawnYawSlider"));
        m_SpawnYawValue = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SpawnYawValue"));
        m_BtnSpawnHere = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSpawnHere"));
        m_BtnSpawnYawApply = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSpawnYawApply"));
        m_BtnSpawnClearPoints = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSpawnClearPoints"));
        m_LaptopText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LaptopText"));
        m_LaptopRotText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LaptopRotText"));
        m_LaptopYawSlider = SliderWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LaptopYawSlider"));
        m_LaptopYawValue = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LaptopYawValue"));
        m_BtnLaptopHere = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnLaptopHere"));
        m_BtnLaptopYawApply = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnLaptopYawApply"));
        m_BtnChestHere = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnChestHere"));
        m_BtnPresetClearInventory = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnPresetClearInventory"));
        m_BtnPresetMe = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnPresetMe"));
        m_PresetHost = M_SUB_WIDGET.FindAnyWidget("PresetHost");
        m_PresetScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("PresetScroll"));
        m_PresetClassEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("PresetClassEdit"));
        m_BtnPresetAdd = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnPresetAdd"));
        m_BtnPresetRemove = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnPresetRemove"));
        m_BtnPresetClear = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnPresetClear"));
        m_FactionStatus = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("FactionStatus"));

        m_AdminMemberHost = M_SUB_WIDGET.FindAnyWidget("AdminMemberHost");
        m_AdminMemberScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminMemberScroll"));
        m_AdminMemberName = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminMemberName"));
        m_AdminMemberRole = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminMemberRole"));
        m_AdminMemberRank = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminMemberRank"));
        m_AdminMemberUid = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminMemberUid"));
        m_AdminBtnPromote = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnPromote"));
        m_AdminBtnDemote = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnDemote"));
        m_AdminBtnKick = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnKick"));

        m_AdminPeopleHost = M_SUB_WIDGET.FindAnyWidget("AdminPeopleHost");
        m_AdminPeopleScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPeopleScroll"));
        m_AdminSelectedPersonName = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminSelectedPersonName"));
        m_AdminSelectedPersonUid = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminSelectedPersonUid"));
        m_AdminAddRoleText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminAddRoleText"));
        m_AdminBtnAddRolePrev = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnAddRolePrev"));
        m_AdminBtnAddRoleNext = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnAddRoleNext"));
        m_AdminAddSelectedName = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminAddSelectedName"));
        m_AdminBtnAddSelected = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnAddSelected"));
        m_AdminAddUidEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminAddUidEdit"));
        m_AdminBtnAddUid = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnAddUid"));
        m_AdminAddStatus = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminAddStatus"));

        m_AdminRoleHost = M_SUB_WIDGET.FindAnyWidget("AdminRoleHost");
        m_AdminRoleScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminRoleScroll"));
        m_AdminBtnNewRole = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnNewRole"));
        m_AdminRoleIdEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminRoleIdEdit"));
        m_AdminRoleNameEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminRoleNameEdit"));
        m_AdminRoleLevelEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminRoleLevelEdit"));
        m_AdminPermUseLaptop = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermUseLaptop"));
        m_AdminPermInvite = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermInvite"));
        m_AdminPermKick = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermKick"));
        m_AdminPermPromote = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermPromote"));
        m_AdminPermDemote = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermDemote"));
        m_AdminPermManageRoles = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermManageRoles"));
        m_AdminPermEditRights = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermEditRights"));
        m_AdminPermIssueLicenses = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminPermIssueLicenses"));
        m_AdminBtnSaveRole = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminBtnSaveRole"));
        m_AdminRoleStatus = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AdminRoleStatus"));

        m_AdminRankHost = M_SUB_WIDGET.FindAnyWidget("RankHost");
        m_AdminRankScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankList"));
        m_AdminBtnNewRank = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankCreateButton"));
        m_AdminRankIdEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankIdEdit"));
        m_AdminRankNameEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankNameEdit"));
        m_AdminRankLevelEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankLevelEdit"));
        m_AdminRankSalaryEdit = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankSalaryEdit"));
        m_RankCurRub = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankCurRub"));
        m_RankCurUsd = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankCurUsd"));
        m_RankCurEur = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankCurEur"));
        m_AdminBtnSaveRank = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankSaveButton"));
        m_AdminBtnDeleteRank = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankDeleteButton"));
        m_RankMemberText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SelectedPlayerName"));
        m_RankMemberCurrent = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CurrentRankText"));
        m_SelectedRankText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SelectedRankText"));
        m_SelectedPlayerRole = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SelectedPlayerRole"));
        m_RankMemberList = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankPlayerList"));
        m_RankAssignSelected = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("AssignRankButton"));
        m_SalaryBankId = null;
        m_SalaryCurrencyId = null;
        m_BankAccountText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BankAccountText"));
        m_SalaryIntervalMin = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SalaryIntervalEdit"));
        m_SalaryGraceMin = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("GracePeriodEdit"));
        m_SalarySaveSettings = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BankSaveButton"));
        m_SalaryStatus = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BankStatusText"));
        m_RankCombatPanel = M_SUB_WIDGET.FindAnyWidget("RankCombatPanel");
        m_RankTabCombat = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankTabCombat"));
        m_RankTabCivilian = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("RankTabCivilian"));
        m_CivilianBenefitPanel = M_SUB_WIDGET.FindAnyWidget("CivilianBenefitPanel");
        m_CivilianBenefitEnabled = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitEnabled"));
        m_CivilianBenefitAmount = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitAmountEdit"));
        m_BenefitCurRub = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitCurRub"));
        m_BenefitCurUsd = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitCurUsd"));
        m_BenefitCurEur = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitCurEur"));
        m_CivilianBenefitInterval = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitIntervalEdit"));
        m_CivilianBenefitSave = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BenefitSaveButton"));
        m_CivilianBenefitStatus = null;

        m_LicLicenseHost = M_SUB_WIDGET.FindAnyWidget("LicLicenseHost");
        m_LicLicenseScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicLicenseScroll"));
        m_LicLicenseId = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicLicenseId"));
        m_LicLicenseName = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicLicenseName"));
        m_LicLicenseDescription = MultilineEditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicLicenseDescription"));
        m_LicActive = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicBtnActive"));
        m_LicNew = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicBtnNew"));
        m_LicSave = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicBtnSave"));
        m_LicIssuerHost = M_SUB_WIDGET.FindAnyWidget("LicIssuerHost");
        m_LicIssuerScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicIssuerScroll"));
        m_LicToggleFaction = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicBtnToggleFaction"));
        m_LicDelete = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicBtnDelete"));
        m_LicStatus = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LicStatus"));

        m_TerrPointMode = M_SUB_WIDGET.FindAnyWidget("TerritoryPointsHost");
        m_ModeBtnPoints = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ModeBtnPoints"));
        m_ModeBtnChains = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ModeBtnChains"));
        m_ModeBtnRewards = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ModeBtnRewards"));
        m_TerrStatus = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryStatus"));
        m_TerritoryChainHost = M_SUB_WIDGET.FindAnyWidget("TerritoryChainHost");
        m_TerritoryRewardHost = M_SUB_WIDGET.FindAnyWidget("TerritoryRewardHost");
        m_TerrMap = MapWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryMap"));
        m_TerrListHost = M_SUB_WIDGET.FindAnyWidget("TerritoryHost");
        m_TerrListScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryList"));
        m_TerrId = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryIdEdit"));
        m_TerrName = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryNameEdit"));
        m_TerrMapPoint = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryPointName"));
        m_TerrCreate = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryCreateButton"));
        m_TerrDelete = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryDeleteButton"));
        m_TerrRadius = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryRadiusEdit"));
        m_TerrMinPlayers = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryMinPlayersEdit"));
        m_TerrCaptureMin = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryCaptureMinEdit"));
        m_TerrPrepMin = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryPrepMinEdit"));
        m_TerrCooldownMin = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryCooldownMinEdit"));
        m_TerrParticipation = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryParticipationEdit"));
        // Daily capture window (start/end hour, per-day toggles, daily limit) has
        // no widget in the redesigned point editor; these stay unbound (null) and
        // every use below is already null-safe, so it is simply inert now.
        m_TerrDirectCapture = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryDirectCaptureButton"));
        // Inline reward add/remove is replaced by a reference to a reward preset,
        // edited on the НАГРАДЫ screen; these stay unbound (null), also inert.
        m_TerrChainName = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryChainName"));
        m_TerrChainPosition = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryChainPosition"));
        m_TerrChainPrevious = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryPrevious"));
        m_TerrChainNext = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryNext"));
        m_TerrRewardNameText = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryRewardName"));
        m_TerrBtnChainEditor = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryOpenChainEditorButton"));
        m_TerrBtnRewardEditor = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryOpenRewardEditorButton"));
        m_TerrBtnPickReward = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritoryPickRewardButton"));

        m_TerrSave = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TerritorySaveButton"));

        // NEW_LAYOUTS are mounted as dedicated editor screens. The old inline chain/reward
        // panels are not created or used by faction_admin_v2.layout.
        m_ChainPanel = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryChainEditor.layout", m_TerritoryChainHost);
        if (m_ChainPanel)
        {
            m_ChainPanel.Show(false);
            m_ChainMap = MapWidget.Cast(m_ChainPanel.FindAnyWidget("ChainMap"));
            m_ChainList = TextListboxWidget.Cast(m_ChainPanel.FindAnyWidget("ChainList"));
            m_ChainId = EditBoxWidget.Cast(m_ChainPanel.FindAnyWidget("ChainIdEdit"));
            m_ChainName = EditBoxWidget.Cast(m_ChainPanel.FindAnyWidget("ChainNameEdit"));
            m_ChainSideA = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("ChainEntryAButton"));
            m_ChainSideB = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("ChainEntryBButton"));
            m_ChainSideAText = TextWidget.Cast(m_ChainPanel.FindAnyWidget("ChainEntryAText"));
            m_ChainSideBText = TextWidget.Cast(m_ChainPanel.FindAnyWidget("ChainEntryBText"));
            m_ChainNew = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("ChainCreateButton"));
            m_ChainSave = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("ChainSaveButton"));
            m_ChainDelete = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("ChainDeleteButton"));
            m_ChainClose = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("ChainBackButton"));
            m_ChainWindowClose = ButtonWidget.Cast(m_ChainPanel.FindAnyWidget("BtnClose"));
            m_ChainPath = TextWidget.Cast(m_ChainPanel.FindAnyWidget("ChainPointsHint"));
            m_ChainPointList = m_ChainPanel.FindAnyWidget("ChainPointList");
            m_ChainPointScroll = ScrollWidget.Cast(m_ChainPanel.FindAnyWidget("ChainPointScroll"));
        }

        m_RewardPresetPanel = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryRewardPresetEditor.layout", m_TerritoryRewardHost);
        if (m_RewardPresetPanel)
        {
            m_RewardPresetPanel.Show(false);
            m_RewardPresetList = m_RewardPresetPanel.FindAnyWidget("RewardPresetList");
            m_RewardPresetListScroll = ScrollWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardPresetScroll"));
            m_RewardPresetNew = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardPresetCreateButton"));
            m_RewardPresetAssign = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardPresetAssignButton"));
            m_RewardPresetId = EditBoxWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardPresetIdEdit"));
            m_RewardPresetName = EditBoxWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardPresetNameEdit"));
            m_RewardPresetSave = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardSaveButton"));
            m_RewardPresetDelete = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardDeleteButton"));
            m_RewardPresetClose = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardBackButton"));
            m_RewardPresetWindowClose = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("BtnClose"));
            m_RewardItemSearch = EditBoxWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardSearchEdit"));
            m_RewardItemSearchBtn = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardSearchButton"));
            m_RewardSearchResults = m_RewardPresetPanel.FindAnyWidget("RewardSearchResults");
            m_RewardSearchResultsScroll = ScrollWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardSearchResultsScroll"));
            m_RewardItemPreview = ItemPreviewWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardPreview"));
            m_RewardItemPreviewName = TextWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardSelectedItemName"));
            m_RewardItemClass = TextWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardSelectedClassname"));
            m_RewardItemCount = EditBoxWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardQuantityEdit"));
            m_RewardItemAdd = ButtonWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardAddButton"));
            m_RewardPresetItems = m_RewardPresetPanel.FindAnyWidget("RewardContentList");
            m_RewardPresetItemsScroll = ScrollWidget.Cast(m_RewardPresetPanel.FindAnyWidget("RewardContentScroll"));
        }

    
        if (!M_SUB_WIDGET)
            return;
        M_SUB_WIDGET.SetHandler(this);
        // Critical interactive widgets receive an explicit handler as well.
        // VPP can create/reparent sub-widgets in a way where relying only on the root handler
        // leaves list selections visible but non-interactive.
        if (m_RankMemberList) m_RankMemberList.SetHandler(this);
        if (m_RewardPresetPanel) m_RewardPresetPanel.SetHandler(this);
        if (m_RewardPresetList) m_RewardPresetList.SetHandler(this);
        if (m_RewardSearchResults) m_RewardSearchResults.SetHandler(this);
        if (m_RewardPresetItems) m_RewardPresetItems.SetHandler(this);
        if (m_ChainPanel) m_ChainPanel.SetHandler(this);
        if (m_ChainList) m_ChainList.SetHandler(this);
        if (m_ChainPointList) m_ChainPointList.SetHandler(this);
        if (m_ChainMap) m_ChainMap.SetHandler(this);
        if (m_TerrMap) m_TerrMap.SetHandler(this);
        if (m_FactionScroll) m_FactionScroll.SetHandler(this);
        if (m_FactionHost) m_FactionHost.SetHandler(this);
        m_TitlePanel = Widget.Cast(M_SUB_WIDGET.FindAnyWidget("Shell"));
        m_closeButton = m_BtnClose;
        s_Menu = this;
        TerrInitMapView();
        ShowFactionTab();
        RefreshAll();
        if (m_FactionStatus) m_FactionStatus.SetText("ЗАГРУЗКА ДАННЫХ TFL...");
        m_VppRequestAttempts = 1;
        RequestPanel("");
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(TFLRetryInitialPanel, 1200, false);
    }


    void TFLRetryInitialPanel()
    {
        if (!s_Menu || s_Menu != this)
            return;
        if (s_Panel)
            return;
        if (m_VppRequestAttempts >= 4)
        {
            if (m_FactionStatus)
                m_FactionStatus.SetText("НЕТ ОТВЕТА ОТ TFL SERVER // ПРОВЕРЬТЕ SERVER PBO");
            Print("[TFL/VPP] No admin panel response after retries.");
            return;
        }
        m_VppRequestAttempts = m_VppRequestAttempts + 1;
        Print("[TFL/VPP] Retry RequestPanel #" + m_VppRequestAttempts.ToString());
        RequestPanel("");
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(TFLRetryInitialPanel, 1200, false);
    }

    protected void HideAllTabs()
    {
        if (m_FactionPanel) m_FactionPanel.Show(false);
        if (m_MembersPanel) m_MembersPanel.Show(false);
        if (m_RolesPanel) m_RolesPanel.Show(false);
        if (m_RanksPanel) m_RanksPanel.Show(false);
        if (m_LicensesPanel) m_LicensesPanel.Show(false);
        if (m_TerritoriesPanel) m_TerritoriesPanel.Show(false);
        if (m_FactionSidebar) m_FactionSidebar.Show(false);
        // Гасим редакторы вместе с их хостами: иначе видимый хост режима
        // переживёт уход на другую вкладку и накроет её собой.
        if (m_RewardPresetPanel) m_RewardPresetPanel.Show(false);
        if (m_TerritoryRewardHost) m_TerritoryRewardHost.Show(false);
        if (m_ChainPanel) m_ChainPanel.Show(false);
        if (m_TerritoryChainHost) m_TerritoryChainHost.Show(false);
        m_ChainEditMode = false;
    }

    // ТЕРРИТОРИИ has three modes (ТОЧКИ / ЦЕПОЧКИ / НАГРАДЫ) sharing one
    // TerritoriesPanel; only one of the three host panels is visible at a time.
    //
    // Каждый режим — это пара «хост + его содержимое». Показывать содержимое,
    // не показав хост, бесполезно: скрытый родитель гасит показанного ребёнка,
    // и вкладка выглядит пустой. Поэтому режимы переключаются только здесь, и
    // всегда обеими половинами сразу.
    protected void TerrApplyMode(bool points, bool chains, bool rewards)
    {
        if (m_TerrPointMode) m_TerrPointMode.Show(points);

        if (m_TerritoryChainHost) m_TerritoryChainHost.Show(chains);
        if (m_ChainPanel) m_ChainPanel.Show(chains);

        if (m_TerritoryRewardHost) m_TerritoryRewardHost.Show(rewards);
        if (m_RewardPresetPanel) m_RewardPresetPanel.Show(rewards);

        int active = ARGB(255, 42, 53, 32);
        int idle = ARGB(255, 14, 17, 9);
        if (m_ModeBtnPoints)
        {
            if (points) m_ModeBtnPoints.SetColor(active);
            else m_ModeBtnPoints.SetColor(idle);
        }
        if (m_ModeBtnChains)
        {
            if (chains) m_ModeBtnChains.SetColor(active);
            else m_ModeBtnChains.SetColor(idle);
        }
        if (m_ModeBtnRewards)
        {
            if (rewards) m_ModeBtnRewards.SetColor(active);
            else m_ModeBtnRewards.SetColor(idle);
        }
    }

    protected void TerrSetPointMode()
    {
        m_ChainEditMode = false;
        if (m_TerritoriesPanel) m_TerritoriesPanel.Show(true);
        TerrApplyMode(true, false, false);
        if (m_TerrMap) m_TerrMap.Show(true);
    }

    protected void TerrSetChainMode()
    {
        m_ChainEditMode = true;
        if (m_TerritoriesPanel) m_TerritoriesPanel.Show(true);
        TerrApplyMode(false, true, false);
        TerrInitMapWidget(m_ChainMap);
    }

    protected void TerrSetRewardMode()
    {
        m_ChainEditMode = false;
        if (m_TerritoriesPanel) m_TerritoriesPanel.Show(true);
        TerrApplyMode(false, false, true);
    }

    protected void ShowFactionTab()
    {
        HideAllTabs();
        m_ActiveTab = TAB_FACTION;
        SetScreenTitle("АДМИНИСТРИРОВАНИЕ ФРАКЦИИ");
        if (m_FactionSidebar) m_FactionSidebar.Show(true);
        if (m_FactionPanel) m_FactionPanel.Show(true);
    }

    protected void ShowMembersTab()
    {
        HideAllTabs();
        m_ActiveTab = TAB_MEMBERS;
        SetScreenTitle("СОСТАВ");
        if (m_MembersPanel)
            m_MembersPanel.Show(true);
    }

    protected void ShowRolesTab()
    {
        HideAllTabs();
        m_ActiveTab = TAB_ROLES;
        SetScreenTitle("РОЛИ И ПРАВА");
        if (m_RolesPanel)
            m_RolesPanel.Show(true);
    }

    protected void ShowRanksTab()
    {
        HideAllTabs();
        m_ActiveTab = TAB_RANKS;
        SetScreenTitle("ЗВАНИЯ И ОКЛАДЫ");
        if (m_RanksPanel)
            m_RanksPanel.Show(true);
        RebuildRanks();
        RefreshRankEditor();
        RefreshSalarySettings();
    }

    protected void ShowLicensesTab()
    {
        HideAllTabs();
        m_ActiveTab = TAB_LICENSES;
        SetScreenTitle("ЛИЦЕНЗИИ");
        if (m_LicensesPanel)
            m_LicensesPanel.Show(true);
        LicRefresh();
    }

    protected void ShowTerritoriesTab()
    {
        HideAllTabs();
        m_ActiveTab = TAB_TERRITORIES;
        SetScreenTitle("ТЕРРИТОРИИ");
        if (m_TerritoriesPanel) m_TerritoriesPanel.Show(true);
        TerrSetPointMode();
        TerrInitMapView();
        TerrRefresh();
    }

    protected void BindDynamicRow(Widget row)
    {
        if (!row) return;
        row.SetHandler(this);
        Widget button = row.FindAnyWidget("RowBtn");
        if (button) button.SetHandler(this);
    }

    protected bool WidgetInsideRow(Widget clicked, Widget row)
    {
        if (!clicked || !row) return false;
        Widget cursor = clicked;
        while (cursor)
        {
            if (cursor == row) return true;
            cursor = cursor.GetParent();
        }
        return false;
    }

    protected void SelectFactionById(string factionId)
    {
        if (factionId == "") return;
        if (m_PendingFactionSelection == factionId) return;
        if (s_Panel && s_Panel.selected_id == factionId) return;
        m_PendingFactionSelection = factionId;
        // Selection is a read/navigation action. RequestPanel(factionId) is the canonical
        // VPP path: BuildAdminPanelDto also persists m_OpenFaction on the server, so all
        // following write commands target the newly selected faction.
        m_SelectedMember = "";
        m_SelectedPerson = "";
        m_AddRoleId = "";
        m_SelectedRole = "";
        m_SelectedRank = "";
        m_NewRank = false;
        m_SelectedPreset = "";
        RequestPanel(factionId);
    }

    protected bool TrySelectFactionFromWidget(Widget w)
    {
        if (!w || !s_Panel) return false;
        for (int i = 0; i < m_FactionRows.Count(); i++)
        {
            Widget row = m_FactionRows.Get(i);
            if (!row || !WidgetInsideRow(w, row)) continue;
            if (i < 0 || i >= s_Panel.factions.Count()) return true;
            TFLFactionSummaryDto faction = s_Panel.factions.Get(i);
            if (faction) SelectFactionById(faction.id);
            return true;
        }
        return false;
    }

    protected void QueueOrSendChainPoint(string territoryId)
    {
        if (territoryId == "" || m_SelectedChainId == "") return;
        if (ChainById(m_SelectedChainId))
        {
            TFLFactionCommandDto dto = new TFLFactionCommandDto();
            dto.chain_id = m_SelectedChainId;
            dto.target_territory_id = territoryId;
            SendCommand(TFLFactionCommand.ADMIN_CHAIN_ADD_POINT, dto);
            return;
        }
        if (!m_PendingChainPoints) m_PendingChainPoints = new array<string>();
        if (m_PendingChainPoints.Find(territoryId) == -1) m_PendingChainPoints.Insert(territoryId);
        if (m_ChainPath) m_ChainPath.SetText("СОЗДАЮ ЦЕПОЧКУ // ТОЧКА БУДЕТ ДОБАВЛЕНА АВТОМАТИЧЕСКИ");
    }

    protected void FlushPendingChainPoints()
    {
        if (!m_PendingChainPoints || m_PendingChainPoints.Count() == 0) return;
        if (m_SelectedChainId == "" || !ChainById(m_SelectedChainId)) return;
        ref array<string> pending = new array<string>();
        for (int i = 0; i < m_PendingChainPoints.Count(); i++) pending.Insert(m_PendingChainPoints.Get(i));
        m_PendingChainPoints.Clear();
        for (int p = 0; p < pending.Count(); p++)
        {
            TFLFactionCommandDto dto = new TFLFactionCommandDto();
            dto.chain_id = m_SelectedChainId;
            dto.target_territory_id = pending.Get(p);
            SendCommand(TFLFactionCommand.ADMIN_CHAIN_ADD_POINT, dto);
        }
    }

    protected void SetScreenTitle(string section)
    {
        m_ActiveSection = section;
        if (!m_TitleText || !s_Panel) return;
        string factionName = s_Panel.selected_name;
        if (factionName == "") factionName = "TFL";
        m_TitleText.SetText(m_ActiveSection + " // " + factionName);
    }

    protected void ClearWidgets(array<Widget> widgets)
    {
        for (int i = 0; i < widgets.Count(); i++)
        {
            Widget old = widgets.Get(i);
            if (old)
                old.Unlink();
        }
        widgets.Clear();
    }

    void RefreshAll()
    {
        if (!s_Panel)
            return;

        if (m_OperatorText)
        {
            string text = "ADMIN: ";
            Man man = GetGame().GetPlayer();
            if (man)
            {
                if (man.GetIdentity())
                    text = text + man.GetIdentity().GetName();
            }
            m_OperatorText.SetText(text);
        }
        if (m_TitleText)
            m_TitleText.SetText(m_ActiveSection + " // " + s_Panel.selected_name);

        RebuildFactions();
        RefreshFactionFields();
        RebuildPreset();
        RebuildMembers();
        RefreshMember();
        RebuildPeople();
        RefreshAddPanel();
        RebuildRoles();
        RefreshRoleEditor();
        RebuildRanks();
        RefreshRankEditor();
        RefreshSalarySettings();
        LicRefresh();
        TerrRefresh();
        if (m_RewardPresetPanel && m_RewardPresetPanel.IsVisible()) RewardPresetRefresh();
        if (m_ChainPanel && m_ChainPanel.IsVisible())
        {
            ChainRefresh();
            TerrRefreshMarks();
        }
    }

    protected void RebuildFactions()
    {
        if (!m_FactionHost)
            return;
        ClearWidgets(m_FactionRows);
        int count = s_Panel.factions.Count();
        float content = count * 0.14;
        if (content < 1.0)
            content = 1.0;
        m_FactionHost.SetSize(1, content);
        float step = 0.14 / content;

        for (int i = 0; i < count; i++)
        {
            TFLFactionSummaryDto faction = s_Panel.factions.Get(i);
            if (!faction)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_faction_row.layout", m_FactionHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_FactionRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget countText = TextWidget.Cast(row.FindAnyWidget("RowCount"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(faction.name);
            if (countText)
                if (faction.system_civilian == 1) countText.SetText("AUTO");
                else countText.SetText(faction.members.ToString() + " / " + faction.roles.ToString());
            if (mark)
                mark.Show(faction.id == s_Panel.selected_id);
        }
        if (m_FactionScroll)
            m_FactionScroll.VScrollToPos01(0);
    }

    protected void RefreshFactionFields()
    {
        if (m_SelectedIdText)
            m_SelectedIdText.SetText("ID: " + s_Panel.selected_id);
        if (m_SelectedNameEdit)
            m_SelectedNameEdit.SetText(s_Panel.selected_name);
        if (m_SelectedShortEdit)
            m_SelectedShortEdit.SetText(s_Panel.selected_short_name);
        m_EditSideId = s_Panel.selected_side_id;
        if (m_EditSideId == "")
            m_EditSideId = TFLFactionSide.NEUTRAL;
        if (m_BtnSide)
            m_BtnSide.SetText("СТОРОНА: " + TFLFactionSide.Display(m_EditSideId));
        bool civilian = s_Panel.selected_system_civilian == 1;
        if (civilian)
        {
            if (m_SpawnText)
                m_SpawnText.SetText("ТОЧЕК SPAWN: " + s_Panel.spawn_points.Count().ToString() + " // ПОСЛЕДНЯЯ: " + TFLFactionUiText.Vec(s_Panel.spawn_position));
            if (m_SpawnRotText)
                m_SpawnRotText.SetText("ГРАЖДАНСКИЕ // СЛУЧАЙНАЯ ТОЧКА ПРИ СПАВНЕ");
            if (m_BtnSpawnHere)
            {
                m_BtnSpawnHere.SetText("+ ДОБАВИТЬ ТОЧКУ SPAWN");
                m_BtnSpawnHere.SetSize(0.255, 0.055);
            }
            if (m_BtnSpawnClearPoints) m_BtnSpawnClearPoints.Show(true);
        }
        else
        {
            if (m_SpawnText)
                m_SpawnText.SetText("POS: " + TFLFactionUiText.Vec(s_Panel.spawn_position));
            if (m_SpawnRotText)
                m_SpawnRotText.SetText("ROT: " + TFLFactionUiText.Vec(s_Panel.spawn_orientation));
            if (m_BtnSpawnHere)
            {
                m_BtnSpawnHere.SetText("МОЯ ПОЗИЦИЯ = SPAWN");
                m_BtnSpawnHere.SetSize(0.40, 0.055);
            }
            if (m_BtnSpawnClearPoints) m_BtnSpawnClearPoints.Show(false);
        }
        if (m_LaptopText)
            m_LaptopText.SetText("POS: " + TFLFactionUiText.Vec(s_Panel.laptop_position));
        if (m_LaptopRotText)
            m_LaptopRotText.SetText("ROT: " + TFLFactionUiText.Vec(s_Panel.laptop_orientation));

        float spawnYaw = 0;
        if (s_Panel.spawn_orientation && s_Panel.spawn_orientation.Count() > 0) spawnYaw = s_Panel.spawn_orientation.Get(0);
        float laptopYaw = 0;
        if (s_Panel.laptop_orientation && s_Panel.laptop_orientation.Count() > 0) laptopYaw = s_Panel.laptop_orientation.Get(0);
        if (m_SpawnYawSlider) m_SpawnYawSlider.SetCurrent(spawnYaw);
        if (m_SpawnYawValue) m_SpawnYawValue.SetText("YAW " + Math.Round(spawnYaw).ToString() + "°");
        if (m_LaptopYawSlider) m_LaptopYawSlider.SetCurrent(laptopYaw);
        if (m_LaptopYawValue) m_LaptopYawValue.SetText("YAW " + Math.Round(laptopYaw).ToString() + "°");

        if (m_BtnPresetClearInventory)
            m_BtnPresetClearInventory.SetText(TFLFactionUiText.OnOff(s_Panel.clear_inventory) + " ОЧИЩАТЬ ИНВЕНТАРЬ");
        if (m_FactionStatus)
        {
            string status;
            if (civilian)
                status = "SYSTEM=CIVILIAN // SPAWN POINTS=" + s_Panel.spawn_points.Count().ToString() + " // СОСТАВ=AUTO";
            else
            {
                status = "REV " + s_Panel.revision.ToString();
                status = status + " // SPAWN=" + s_Panel.spawn_active.ToString();
                status = status + " // LAPTOP=" + s_Panel.laptop_active.ToString();
            }
            m_FactionStatus.SetText(status);
        }
    }

    protected string ConfigDisplayName(string className)
    {
        string displayName = "";
        string path = "CfgVehicles " + className;
        if (GetGame().ConfigIsExisting(path)) GetGame().ConfigGetText(path + " displayName", displayName);
        if (displayName == "")
        {
            path = "CfgWeapons " + className;
            if (GetGame().ConfigIsExisting(path)) GetGame().ConfigGetText(path + " displayName", displayName);
        }
        if (displayName == "")
        {
            path = "CfgMagazines " + className;
            if (GetGame().ConfigIsExisting(path)) GetGame().ConfigGetText(path + " displayName", displayName);
        }
        if (displayName == "") displayName = className;
        return displayName;
    }

    protected void RebuildPreset()
    {
        if (!m_PresetHost)
            return;
        ClearWidgets(m_PresetRows);
        int count = s_Panel.preset_items.Count();
        float content = count * 0.095;
        if (content < 1.0)
            content = 1.0;
        m_PresetHost.SetSize(1, content);
        float step = 0.095 / content;
        for (int i = 0; i < count; i++)
        {
            string className = s_Panel.preset_items.Get(i);
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_preset_row.layout", m_PresetHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_PresetRows.Insert(row);
            BindDynamicRow(row);
            TextWidget text = TextWidget.Cast(row.FindAnyWidget("RowClass"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (text)
                text.SetText(ConfigDisplayName(className) + "  //  " + className);
            if (mark)
                mark.Show(className == m_SelectedPreset);
        }
        if (m_PresetScroll)
            m_PresetScroll.VScrollToPos01(0);
    }

    protected void RebuildMembers()
    {
        if (!m_AdminMemberHost)
            return;
        ClearWidgets(m_MemberRows);
        int count = s_Panel.members.Count();
        float content = count * 0.12;
        if (content < 1.0)
            content = 1.0;
        m_AdminMemberHost.SetSize(1, content);
        float step = 0.12 / content;
        for (int i = 0; i < count; i++)
        {
            TFLFactionMemberDto member = s_Panel.members.Get(i);
            if (!member)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_member_row.layout", m_AdminMemberHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_MemberRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget role = TextWidget.Cast(row.FindAnyWidget("RowRole"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(member.name);
            if (role)
            {
                string memberLine = member.role_name;
                if (member.rank_name != "") memberLine = member.rank_name + " // " + member.role_name;
                role.SetText(memberLine);
            }
            if (mark)
                mark.Show(member.uid == m_SelectedMember);
        }
        if (m_AdminMemberScroll)
            m_AdminMemberScroll.VScrollToPos01(0);
    }

    protected TFLFactionMemberDto FindMember(string uid)
    {
        for (int i = 0; i < s_Panel.members.Count(); i++)
        {
            TFLFactionMemberDto member = s_Panel.members.Get(i);
            if (member)
            {
                if (member.uid == uid)
                    return member;
            }
        }
        return null;
    }

    protected void RefreshMember()
    {
        TFLFactionMemberDto member = FindMember(m_SelectedMember);
        if (!member)
        {
            m_SelectedMember = "";
            if (m_AdminMemberName)
                m_AdminMemberName.SetText("-");
            if (m_AdminMemberRole)
                m_AdminMemberRole.SetText("Роль: -");
            if (m_AdminMemberRank)
                m_AdminMemberRank.SetText("Звание: -");
            if (m_AdminMemberUid)
                m_AdminMemberUid.SetText("UID: -");
            return;
        }
        if (m_AdminMemberName)
            m_AdminMemberName.SetText(member.name);
        if (m_AdminMemberRole)
            m_AdminMemberRole.SetText("Роль: " + member.role_name);
        if (m_AdminMemberRank)
            m_AdminMemberRank.SetText("Звание: " + member.rank_name + " // " + member.salary.ToString() + " " + TFLFactionCurrency.Symbol(member.currency_id) + "/ч");
        if (m_AdminMemberUid)
            m_AdminMemberUid.SetText("UID: " + member.uid);
    }

    protected void RebuildPeople()
    {
        if (!m_AdminPeopleHost)
            return;
        ClearWidgets(m_PeopleRows);
        int count = s_Panel.people.Count();
        float content = count * 0.095;
        if (content < 1.0)
            content = 1.0;
        m_AdminPeopleHost.SetSize(1, content);
        float step = 0.095 / content;

        for (int i = 0; i < count; i++)
        {
            TFLFactionPassportPersonDto person = s_Panel.people.Get(i);
            if (!person)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_person_row.layout", m_AdminPeopleHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_PeopleRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget serial = TextWidget.Cast(row.FindAnyWidget("RowSerial"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(person.name);
            if (serial)
                serial.SetText(person.serial + " // " + person.uid);
            if (mark)
                mark.Show(person.uid == m_SelectedPerson);
        }
        if (m_AdminPeopleScroll)
            m_AdminPeopleScroll.VScrollToPos01(0);
    }

    protected TFLFactionPassportPersonDto FindPerson(string uid)
    {
        if (!s_Panel)
            return null;
        for (int i = 0; i < s_Panel.people.Count(); i++)
        {
            TFLFactionPassportPersonDto person = s_Panel.people.Get(i);
            if (!person)
                continue;
            if (person.uid == uid)
                return person;
        }
        return null;
    }

    protected void EnsureAddRole()
    {
        if (!s_Panel)
            return;
        if (s_Panel.roles.Count() <= 0)
        {
            m_AddRoleId = "";
            return;
        }
        if (FindRole(m_AddRoleId))
            return;

        TFLFactionRoleDto lowest = null;
        for (int i = 0; i < s_Panel.roles.Count(); i++)
        {
            TFLFactionRoleDto role = s_Panel.roles.Get(i);
            if (!role)
                continue;
            if (!lowest)
                lowest = role;
            else if (role.level < lowest.level)
                lowest = role;
        }
        if (lowest)
            m_AddRoleId = lowest.id;
    }

    protected void CycleAddRole(int direction)
    {
        if (!s_Panel)
            return;
        if (s_Panel.roles.Count() <= 0)
            return;
        EnsureAddRole();

        int current = 0;
        for (int i = 0; i < s_Panel.roles.Count(); i++)
        {
            TFLFactionRoleDto role = s_Panel.roles.Get(i);
            if (!role)
                continue;
            if (role.id == m_AddRoleId)
            {
                current = i;
                break;
            }
        }
        current = current + direction;
        if (current < 0)
            current = s_Panel.roles.Count() - 1;
        if (current >= s_Panel.roles.Count())
            current = 0;

        TFLFactionRoleDto selected = s_Panel.roles.Get(current);
        if (selected)
            m_AddRoleId = selected.id;
        RefreshAddPanel();
    }

    protected void RefreshAddPanel()
    {
        EnsureAddRole();
        TFLFactionPassportPersonDto person = FindPerson(m_SelectedPerson);
        if (!person)
            m_SelectedPerson = "";

        if (m_AdminSelectedPersonName)
        {
            if (person)
                m_AdminSelectedPersonName.SetText("Выбрано: " + person.name);
            else
                m_AdminSelectedPersonName.SetText("Выбрано: -");
        }
        if (m_AdminSelectedPersonUid)
        {
            if (person)
                m_AdminSelectedPersonUid.SetText("UID: " + person.uid);
            else
                m_AdminSelectedPersonUid.SetText("UID: -");
        }
        if (m_AdminAddSelectedName)
        {
            if (person)
                m_AdminAddSelectedName.SetText(person.name);
            else
                m_AdminAddSelectedName.SetText("-");
        }
        if (m_AdminAddRoleText)
        {
            TFLFactionRoleDto role = FindRole(m_AddRoleId);
            if (role)
                m_AdminAddRoleText.SetText(role.name);
            else
                m_AdminAddRoleText.SetText("-");
        }
        if (m_AdminAddStatus)
        {
            string status = "ЛЮДЕЙ: " + s_Panel.people.Count().ToString();
            status = status + " // LIVE";
            m_AdminAddStatus.SetText(status);
        }
    }

    protected void AddMemberByUid(string uid)
    {
        if (!s_Panel)
            return;
        uid = TFLPassText.Safe(uid, 64);
        if (uid == "")
            return;
        EnsureAddRole();
        if (m_AddRoleId == "")
            return;

        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.target_uid = uid;
        dto.role_id = m_AddRoleId;
        SendCommand(TFLFactionCommand.ADMIN_ADD_MEMBER, dto);
    }

    protected void RebuildRoles()
    {
        if (!m_AdminRoleHost)
            return;
        ClearWidgets(m_RoleRows);
        int count = s_Panel.roles.Count();
        float content = count * 0.12;
        if (content < 1.0)
            content = 1.0;
        m_AdminRoleHost.SetSize(1, content);
        float step = 0.12 / content;
        for (int i = 0; i < count; i++)
        {
            TFLFactionRoleDto roleDto = s_Panel.roles.Get(i);
            if (!roleDto)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_role_row.layout", m_AdminRoleHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_RoleRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget level = TextWidget.Cast(row.FindAnyWidget("RowLevel"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(roleDto.name);
            if (level)
                level.SetText("LVL " + roleDto.level.ToString());
            if (mark)
                mark.Show(roleDto.id == m_SelectedRole);
        }
        if (m_AdminRoleScroll)
            m_AdminRoleScroll.VScrollToPos01(0);
    }

    protected TFLFactionRoleDto FindRole(string id)
    {
        for (int i = 0; i < s_Panel.roles.Count(); i++)
        {
            TFLFactionRoleDto role = s_Panel.roles.Get(i);
            if (role)
            {
                if (role.id == id)
                    return role;
            }
        }
        return null;
    }

    protected void RefreshRoleEditor()
    {
        if (m_NewRole)
        {
            SetRoleEditor("", "", 10, new TFLFactionRolePermissions());
            if (m_AdminRoleStatus)
                m_AdminRoleStatus.SetText("НОВАЯ РОЛЬ // ADMIN может задать любые права.");
            return;
        }
        TFLFactionRoleDto role = FindRole(m_SelectedRole);
        if (!role)
        {
            SetRoleEditor("", "", 10, new TFLFactionRolePermissions());
            return;
        }
        SetRoleEditor(role.id, role.name, role.level, role.permissions);
    }

    protected void SetRoleEditor(string id, string name, int level, TFLFactionRolePermissions permissions)
    {
        if (m_AdminRoleIdEdit)
            m_AdminRoleIdEdit.SetText(id);
        if (m_AdminRoleNameEdit)
            m_AdminRoleNameEdit.SetText(name);
        if (m_AdminRoleLevelEdit)
            m_AdminRoleLevelEdit.SetText(level.ToString());
        m_EditPermissions = new TFLFactionRolePermissions();
        if (permissions)
            m_EditPermissions = permissions.Copy();
        RefreshPermissionText();
    }

    protected void RefreshPermissionText()
    {
        if (!m_EditPermissions)
            m_EditPermissions = new TFLFactionRolePermissions();
        if (m_AdminPermUseLaptop)
            m_AdminPermUseLaptop.SetText(TFLFactionUiText.OnOff(m_EditPermissions.use_laptop) + " НОУТБУК");
        if (m_AdminPermInvite)
            m_AdminPermInvite.SetText(TFLFactionUiText.OnOff(m_EditPermissions.invite) + " ПРИГЛАШАТЬ");
        if (m_AdminPermKick)
            m_AdminPermKick.SetText(TFLFactionUiText.OnOff(m_EditPermissions.kick) + " ИСКЛЮЧАТЬ");
        if (m_AdminPermPromote)
            m_AdminPermPromote.SetText(TFLFactionUiText.OnOff(m_EditPermissions.promote) + " ПОВЫШАТЬ");
        if (m_AdminPermDemote)
            m_AdminPermDemote.SetText(TFLFactionUiText.OnOff(m_EditPermissions.demote) + " ПОНИЖАТЬ");
        if (m_AdminPermManageRoles)
            m_AdminPermManageRoles.SetText(TFLFactionUiText.OnOff(m_EditPermissions.manage_roles) + " СОЗДАВАТЬ РОЛИ");
        if (m_AdminPermEditRights)
            m_AdminPermEditRights.SetText(TFLFactionUiText.OnOff(m_EditPermissions.edit_role_permissions) + " РЕДАКТИРОВАТЬ ПРАВА РОЛЕЙ");
        if (m_AdminPermIssueLicenses)
            m_AdminPermIssueLicenses.SetText(TFLFactionUiText.OnOff(m_EditPermissions.issue_licenses) + " ЛИЦЕНЗИИ");
    }

    protected void TogglePermission(ButtonWidget button)
    {
        if (!m_EditPermissions)
            m_EditPermissions = new TFLFactionRolePermissions();
        if (button == m_AdminPermUseLaptop)
            m_EditPermissions.use_laptop = 1 - m_EditPermissions.use_laptop;
        if (button == m_AdminPermInvite)
            m_EditPermissions.invite = 1 - m_EditPermissions.invite;
        if (button == m_AdminPermKick)
            m_EditPermissions.kick = 1 - m_EditPermissions.kick;
        if (button == m_AdminPermPromote)
            m_EditPermissions.promote = 1 - m_EditPermissions.promote;
        if (button == m_AdminPermDemote)
            m_EditPermissions.demote = 1 - m_EditPermissions.demote;
        if (button == m_AdminPermManageRoles)
            m_EditPermissions.manage_roles = 1 - m_EditPermissions.manage_roles;
        if (button == m_AdminPermEditRights)
            m_EditPermissions.edit_role_permissions = 1 - m_EditPermissions.edit_role_permissions;
        if (button == m_AdminPermIssueLicenses)
            m_EditPermissions.issue_licenses = 1 - m_EditPermissions.issue_licenses;
        RefreshPermissionText();
    }

    protected TFLFactionRankDto FindRank(string id)
    {
        if (!s_Panel) return null;
        for (int i = 0; i < s_Panel.ranks.Count(); i++)
        {
            TFLFactionRankDto rank = s_Panel.ranks.Get(i);
            if (rank && rank.id == id) return rank;
        }
        return null;
    }

    protected void RebuildRanks()
    {
        if (!m_AdminRankHost || !s_Panel) return;
        ClearWidgets(m_RankRows);
        int count = s_Panel.ranks.Count();
        float content = count * 0.15;
        if (content < 1.0) content = 1.0;
        m_AdminRankHost.SetSize(1, content);
        float step = 0.15 / content;
        for (int i = 0; i < count; i++)
        {
            TFLFactionRankDto rank = s_Panel.ranks.Get(i);
            if (!rank) continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_rank_row.layout", m_AdminRankHost);
            if (!row) continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_RankRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget salary = TextWidget.Cast(row.FindAnyWidget("RowLevel"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name) name.SetText(rank.name);
            if (salary) salary.SetText(rank.salary.ToString() + " " + TFLFactionCurrency.Code(rank.currency_id) + " / ЧАС   //   УР. " + rank.level.ToString());
            if (mark) mark.Show(rank.id == m_SelectedRank);
        }
        if (m_AdminRankScroll) m_AdminRankScroll.VScrollToPos01(0);
        RefreshRankMemberList();
    }

    protected void RefreshRankMemberList()
    {
        if (!m_RankMemberList || !s_Panel) return;
        m_RankMemberList.ClearItems();
        for (int i = 0; i < s_Panel.members.Count(); i++)
        {
            TFLFactionMemberDto member = s_Panel.members.Get(i);
            if (!member) continue;
            string line = member.name + "   //   " + member.rank_name;
            m_RankMemberList.AddItem(line, null, 0);
        }
    }

    protected void RefreshRankEditor()
    {
        if (!s_Panel) return;
        if (m_NewRank)
        {
            if (m_AdminRankIdEdit) m_AdminRankIdEdit.SetText("");
            if (m_AdminRankNameEdit) m_AdminRankNameEdit.SetText("");
            if (m_AdminRankLevelEdit) m_AdminRankLevelEdit.SetText("10");
            if (m_AdminRankSalaryEdit) m_AdminRankSalaryEdit.SetText("0");
            m_RankCurrencyId = TFLFactionCurrency.RUBLE;
        }
        else
        {
            TFLFactionRankDto rank = FindRank(m_SelectedRank);
            if (rank)
            {
                if (m_AdminRankIdEdit) m_AdminRankIdEdit.SetText(rank.id);
                if (m_AdminRankNameEdit) m_AdminRankNameEdit.SetText(rank.name);
                if (m_AdminRankLevelEdit) m_AdminRankLevelEdit.SetText(rank.level.ToString());
                if (m_AdminRankSalaryEdit) m_AdminRankSalaryEdit.SetText(rank.salary.ToString());
                m_RankCurrencyId = TFLFactionCurrency.Normalize(rank.currency_id);
            }
            else
            {
                if (m_AdminRankIdEdit) m_AdminRankIdEdit.SetText("");
                if (m_AdminRankNameEdit) m_AdminRankNameEdit.SetText("");
                if (m_AdminRankLevelEdit) m_AdminRankLevelEdit.SetText("10");
                if (m_AdminRankSalaryEdit) m_AdminRankSalaryEdit.SetText("0");
                m_RankCurrencyId = TFLFactionCurrency.RUBLE;
            }
        }
        RefreshRankCurrencyButtons();

        TFLFactionMemberDto member = FindMember(m_SelectedMember);
        if (m_RankMemberText)
        {
            if (member) m_RankMemberText.SetText(member.name);
            else m_RankMemberText.SetText("-");
        }
        if (m_SelectedPlayerRole)
        {
            if (member) m_SelectedPlayerRole.SetText(member.role_name);
            else m_SelectedPlayerRole.SetText("-");
        }
        if (m_RankMemberCurrent)
        {
            if (member) m_RankMemberCurrent.SetText(member.rank_name);
            else m_RankMemberCurrent.SetText("-");
        }
        if (m_SelectedRankText)
        {
            TFLFactionRankDto targetRank = FindRank(m_SelectedRank);
            if (targetRank) m_SelectedRankText.SetText(targetRank.name);
            else m_SelectedRankText.SetText("-");
        }
    }

    // The design replaced the single cycling currency button with three explicit
    // RUB/USD/EUR buttons; each sets the value directly and this highlights the
    // one currently selected.
    protected void RefreshRankCurrencyButtons()
    {
        if (m_RankCurRub) m_RankCurRub.SetColor(CurrencyButtonColor(m_RankCurrencyId == TFLFactionCurrency.RUBLE));
        if (m_RankCurUsd) m_RankCurUsd.SetColor(CurrencyButtonColor(m_RankCurrencyId == TFLFactionCurrency.DOLLAR));
        if (m_RankCurEur) m_RankCurEur.SetColor(CurrencyButtonColor(m_RankCurrencyId == TFLFactionCurrency.EURO));
    }

    protected void RefreshBenefitCurrencyButtons()
    {
        if (m_BenefitCurRub) m_BenefitCurRub.SetColor(CurrencyButtonColor(m_CivilianBenefitCurrencyId == TFLFactionCurrency.RUBLE));
        if (m_BenefitCurUsd) m_BenefitCurUsd.SetColor(CurrencyButtonColor(m_CivilianBenefitCurrencyId == TFLFactionCurrency.DOLLAR));
        if (m_BenefitCurEur) m_BenefitCurEur.SetColor(CurrencyButtonColor(m_CivilianBenefitCurrencyId == TFLFactionCurrency.EURO));
    }

    protected int CurrencyButtonColor(bool selected)
    {
        if (selected) return ARGB(255, 42, 53, 32);
        return ARGB(255, 27, 32, 21);
    }

    protected void SaveRankEditor()
    {
        if (!s_Panel || !m_AdminRankIdEdit || !m_AdminRankNameEdit || !m_AdminRankLevelEdit || !m_AdminRankSalaryEdit) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.rank_id = TFLFactionText.SafeId(m_AdminRankIdEdit.GetText());
        dto.rank_name = TFLPassText.Safe(m_AdminRankNameEdit.GetText(), 64);
        dto.rank_level = m_AdminRankLevelEdit.GetText().ToInt();
        dto.salary = m_AdminRankSalaryEdit.GetText().ToInt();
        dto.currency_id = TFLFactionCurrency.Normalize(m_RankCurrencyId);
        if (m_NewRank) SendCommand(TFLFactionCommand.ADMIN_CREATE_RANK, dto);
        else SendCommand(TFLFactionCommand.ADMIN_SAVE_RANK, dto);
        m_NewRank = false;
        m_SelectedRank = dto.rank_id;
    }

    protected void DeleteSelectedRank()
    {
        if (!s_Panel || m_SelectedRank == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.rank_id = m_SelectedRank;
        SendCommand(TFLFactionCommand.ADMIN_DELETE_RANK, dto);
        m_SelectedRank = "";
        m_NewRank = false;
    }

    protected void AssignSelectedRank()
    {
        if (!s_Panel || m_SelectedMember == "" || m_SelectedRank == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.target_uid = m_SelectedMember;
        dto.rank_id = m_SelectedRank;
        SendCommand(TFLFactionCommand.ADMIN_SET_MEMBER_RANK, dto);
    }

    protected void RefreshSalarySettings()
    {
        if (!s_Panel || !s_Panel.salary_settings) return;
        // The redesigned Bank panel no longer exposes an enable toggle, a bank id,
        // or a reserve currency edit box — they are shown read-only (or not shown)
        // and saved back unchanged.
        m_SalaryEditEnabled = s_Panel.salary_settings.enabled;
        if (m_BankAccountText) m_BankAccountText.SetText(s_Panel.salary_settings.bank_id);
        if (m_SalaryIntervalMin) m_SalaryIntervalMin.SetText((s_Panel.salary_settings.pay_interval_seconds / 60).ToString());
        if (m_SalaryGraceMin) m_SalaryGraceMin.SetText((s_Panel.salary_settings.reconnect_grace_seconds / 60).ToString());
        if (m_SalaryStatus)
        {
            if (m_SalaryEditEnabled == 1) m_SalaryStatus.SetText("ПОДКЛЮЧЕНО");
            else m_SalaryStatus.SetText("ОТКЛЮЧЕНО");
        }

        m_CivilianBenefitEditEnabled = s_Panel.salary_settings.civilian_benefit_enabled;
        m_CivilianBenefitCurrencyId = TFLFactionCurrency.Normalize(s_Panel.salary_settings.civilian_currency_id);
        bool isCivilian = s_Panel.selected_system_civilian == 1;
        if (m_CivilianBenefitPanel) m_CivilianBenefitPanel.Show(isCivilian);
        if (m_RankCombatPanel) m_RankCombatPanel.Show(!isCivilian);
        if (m_RankTabCivilian) m_RankTabCivilian.Show(isCivilian);
        if (m_BtnRanksTab)
        {
            if (isCivilian) m_BtnRanksTab.SetText("ПОСОБИЕ");
            else m_BtnRanksTab.SetText("ЗВАНИЯ");
        }
        if (m_CivilianBenefitEnabled)
        {
            if (m_CivilianBenefitEditEnabled == 1) m_CivilianBenefitEnabled.SetText("[X] ПОСОБИЕ ВКЛЮЧЕНО");
            else m_CivilianBenefitEnabled.SetText("[ ] ПОСОБИЕ ВКЛЮЧЕНО");
        }
        if (m_CivilianBenefitAmount) m_CivilianBenefitAmount.SetText(s_Panel.salary_settings.civilian_benefit_amount.ToString());
        RefreshBenefitCurrencyButtons();
        if (m_CivilianBenefitInterval) m_CivilianBenefitInterval.SetText((s_Panel.salary_settings.civilian_interval_seconds / 60).ToString());
        RefreshRankCurrencyButtons();
    }

    protected void SaveSalarySettings()
    {
        if (!s_Panel || !s_Panel.salary_settings || !m_SalaryIntervalMin || !m_SalaryGraceMin) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.salary_enabled = m_SalaryEditEnabled;
        // Bank id and reserve currency are no longer editable in the redesigned
        // panel — resend the values already on the server so they are not reset.
        dto.bank_id = s_Panel.salary_settings.bank_id;
        dto.currency_id = TFLFactionCurrency.Normalize(s_Panel.salary_settings.currency_id);
        dto.pay_interval_seconds = m_SalaryIntervalMin.GetText().ToInt() * 60;
        dto.reconnect_grace_seconds = m_SalaryGraceMin.GetText().ToInt() * 60;
        dto.civilian_benefit_enabled = m_CivilianBenefitEditEnabled;
        if (m_CivilianBenefitAmount) dto.civilian_benefit_amount = m_CivilianBenefitAmount.GetText().ToInt();
        else dto.civilian_benefit_amount = s_Panel.salary_settings.civilian_benefit_amount;
        dto.civilian_currency_id = TFLFactionCurrency.Normalize(m_CivilianBenefitCurrencyId);
        if (m_CivilianBenefitInterval) dto.civilian_interval_seconds = m_CivilianBenefitInterval.GetText().ToInt() * 60;
        else dto.civilian_interval_seconds = s_Panel.salary_settings.civilian_interval_seconds;
        SendCommand(TFLFactionCommand.ADMIN_SALARY_SETTINGS_SAVE, dto);
    }

    protected void CycleSide()
    {
        if (m_EditSideId == TFLFactionSide.VSRF)
            m_EditSideId = TFLFactionSide.TERRORISTS;
        else if (m_EditSideId == TFLFactionSide.TERRORISTS)
            m_EditSideId = TFLFactionSide.NEUTRAL;
        else
            m_EditSideId = TFLFactionSide.VSRF;

        if (m_BtnSide)
            m_BtnSide.SetText("СТОРОНА: " + TFLFactionSide.Display(m_EditSideId));
    }

    protected void SendSimple(int command)
    {
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        SendCommand(command, dto);
    }

    protected void CreateFaction()
    {
        if (!m_NewFactionId)
            return;
        if (!m_NewFactionName)
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = TFLFactionText.SafeId(m_NewFactionId.GetText());
        dto.faction_name = TFLPassText.Safe(m_NewFactionName.GetText(), 64);
        if (dto.faction_id == "")
            return;
        if (dto.faction_name == "")
            return;
        SendCommand(TFLFactionCommand.ADMIN_CREATE, dto);
    }

    protected void SaveFactionName()
    {
        if (!m_SelectedNameEdit)
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.faction_name = TFLPassText.Safe(m_SelectedNameEdit.GetText(), 64);
        if (m_SelectedShortEdit)
            dto.short_name = TFLPassText.Safe(m_SelectedShortEdit.GetText(), 24);
        dto.side_id = m_EditSideId;
        SendCommand(TFLFactionCommand.ADMIN_SAVE_NAME, dto);
    }

    protected void MemberCommand(int command)
    {
        if (m_SelectedMember == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.target_uid = m_SelectedMember;
        SendCommand(command, dto);
    }

    protected void SaveRole()
    {
        if (!m_AdminRoleIdEdit)
            return;
        if (!m_AdminRoleNameEdit)
            return;
        if (!m_AdminRoleLevelEdit)
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.role_id = TFLFactionText.SafeId(m_AdminRoleIdEdit.GetText());
        dto.role_name = TFLPassText.Safe(m_AdminRoleNameEdit.GetText(), 64);
        dto.role_level = m_AdminRoleLevelEdit.GetText().ToInt();
        dto.permissions = m_EditPermissions.Copy();
        if (dto.role_id == "")
            return;
        if (dto.role_name == "")
            return;
        if (m_NewRole)
            SendCommand(TFLFactionCommand.ADMIN_CREATE_ROLE, dto);
        else
            SendCommand(TFLFactionCommand.ADMIN_SAVE_ROLE, dto);
        m_NewRole = false;
        m_SelectedRole = dto.role_id;
    }

    protected void PresetAdd()
    {
        if (!m_PresetClassEdit)
            return;
        string value = TFLPassText.Safe(m_PresetClassEdit.GetText(), 96);
        if (value == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.class_name = value;
        SendCommand(TFLFactionCommand.ADMIN_PRESET_ADD, dto);
    }

    protected void PresetRemove()
    {
        if (m_SelectedPreset == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.faction_id = s_Panel.selected_id;
        dto.class_name = m_SelectedPreset;
        SendCommand(TFLFactionCommand.ADMIN_PRESET_REMOVE, dto);
        m_SelectedPreset = "";
    }


    protected TFLLicenseAdminDto LicFindLicense(string id)
    {
        if (!s_Panel)
            return null;
        for (int i = 0; i < s_Panel.licenses.Count(); i++)
        {
            TFLLicenseAdminDto lic = s_Panel.licenses.Get(i);
            if (lic && lic.id == id)
                return lic;
        }
        return null;
    }

    protected bool LicHasIssuer(TFLLicenseAdminDto lic, string factionId)
    {
        if (!lic)
            return false;
        if (!lic.issuer_factions)
            return false;
        return lic.issuer_factions.Find(factionId) != -1;
    }

    protected string LicMark(bool enabled)
    {
        string result = "[ ]";
        if (enabled)
            result = "[X]";
        return result;
    }

    protected void LicRefreshActiveButton()
    {
        if (!m_LicActive)
            return;
        if (m_LicEditActive == 1)
            m_LicActive.SetText("[X] АКТИВНА");
        else
            m_LicActive.SetText("[ ] АКТИВНА");
    }

    protected void LicLoadSelected()
    {
        TFLLicenseAdminDto lic = LicFindLicense(m_LicSelectedLicenseId);
        if (!lic)
            return;
        m_LicNewMode = false;
        if (m_LicLicenseId)
            m_LicLicenseId.SetText(lic.id);
        if (m_LicLicenseName)
            m_LicLicenseName.SetText(lic.name);
        if (m_LicLicenseDescription)
            m_LicLicenseDescription.SetText(lic.description);
        m_LicEditActive = lic.active;
        LicRefreshActiveButton();
    }

    protected void LicRebuildLicenseRows()
    {
        if (!m_LicLicenseHost || !s_Panel)
            return;
        ClearWidgets(m_LicLicenseRows);
        int count = s_Panel.licenses.Count();
        float content = count * 0.12;
        if (content < 1.0)
            content = 1.0;
        m_LicLicenseHost.SetSize(1, content);
        float step = 0.12 / content;
        for (int i = 0; i < count; i++)
        {
            TFLLicenseAdminDto lic = s_Panel.licenses.Get(i);
            if (!lic)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_faction_row.layout", m_LicLicenseHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_LicLicenseRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget state = TextWidget.Cast(row.FindAnyWidget("RowCount"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(lic.name);
            if (state)
            {
                if (lic.active == 1)
                    state.SetText("ON");
                else
                    state.SetText("OFF");
            }
            if (mark)
                mark.Show(!m_LicNewMode && lic.id == m_LicSelectedLicenseId);
        }
    }

    protected void LicRebuildIssuerRows()
    {
        if (!m_LicIssuerHost || !s_Panel)
            return;
        ClearWidgets(m_LicIssuerRows);
        TFLLicenseAdminDto selected = LicFindLicense(m_LicSelectedLicenseId);
        int count = s_Panel.factions.Count();
        float content = count * 0.10;
        if (content < 1.0)
            content = 1.0;
        m_LicIssuerHost.SetSize(1, content);
        float step = 0.10 / content;
        for (int i = 0; i < count; i++)
        {
            TFLFactionSummaryDto faction = s_Panel.factions.Get(i);
            if (!faction)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_faction_row.layout", m_LicIssuerHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_LicIssuerRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget allowed = TextWidget.Cast(row.FindAnyWidget("RowCount"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(faction.name);
            if (allowed)
                allowed.SetText(LicMark(LicHasIssuer(selected, faction.id)));
            if (mark)
                mark.Show(faction.id == m_LicSelectedFactionId);
        }
    }

    protected void LicRefresh()
    {
        if (!s_Panel || !M_SUB_WIDGET)
            return;
        if (!m_LicLicenseHost)
            return;

        if (!m_LicNewMode)
        {
            if (m_LicSelectedLicenseId == "" && s_Panel.licenses.Count() > 0)
                m_LicSelectedLicenseId = s_Panel.licenses.Get(0).id;
            LicLoadSelected();
        }

        LicRebuildLicenseRows();
        LicRebuildIssuerRows();

        if (m_LicStatus)
        {
            if (m_LicNewMode)
            {
                m_LicStatus.SetText("НОВАЯ // заполните поля и сохраните");
            }
            else
            {
                TFLLicenseAdminDto selected = LicFindLicense(m_LicSelectedLicenseId);
                if (selected)
                {
                    int issuerCount = 0;
                    if (selected.issuer_factions)
                        issuerCount = selected.issuer_factions.Count();
                    m_LicStatus.SetText("ID=" + selected.id + " // ВЫДАЮТ=" + issuerCount.ToString());
                }
                else
                {
                    m_LicStatus.SetText("ЛИЦЕНЗИИ НЕ ВЫБРАНЫ");
                }
            }
        }
    }

    protected void LicBeginNew()
    {
        m_LicNewMode = true;
        m_LicSelectedLicenseId = "";
        m_LicSelectedFactionId = "";
        if (m_LicLicenseId)
            m_LicLicenseId.SetText("");
        if (m_LicLicenseName)
            m_LicLicenseName.SetText("");
        if (m_LicLicenseDescription)
            m_LicLicenseDescription.SetText("");
        m_LicEditActive = 1;
        LicRefreshActiveButton();
        LicRefresh();
    }

    protected void LicSaveLicense()
    {
        if (!m_LicLicenseId || !m_LicLicenseName || !m_LicLicenseDescription)
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        if (m_LicNewMode)
            dto.license_id = m_LicLicenseId.GetText();
        else
            dto.license_id = m_LicSelectedLicenseId;
        dto.license_name = m_LicLicenseName.GetText();
        string descriptionText;
        m_LicLicenseDescription.GetText(descriptionText);
        dto.description = descriptionText;
        dto.int_value = m_LicEditActive;
        if (m_LicNewMode)
        {
            SendCommand(TFLFactionCommand.ADMIN_LICENSE_CREATE, dto);
            m_LicSelectedLicenseId = TFLFactionText.SafeId(dto.license_id);
            m_LicNewMode = false;
        }
        else
        {
            SendCommand(TFLFactionCommand.ADMIN_LICENSE_SAVE, dto);
        }
    }

    protected void LicToggleIssuer()
    {
        if (m_LicSelectedLicenseId == "")
            return;
        if (m_LicSelectedFactionId == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.license_id = m_LicSelectedLicenseId;
        dto.target_faction_id = m_LicSelectedFactionId;
        SendCommand(TFLFactionCommand.ADMIN_LICENSE_TOGGLE_FACTION, dto);
    }

    protected void LicDeleteLicense()
    {
        if (m_LicSelectedLicenseId == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.license_id = m_LicSelectedLicenseId;
        SendCommand(TFLFactionCommand.ADMIN_LICENSE_DELETE, dto);
        m_LicSelectedLicenseId = "";
        m_LicSelectedFactionId = "";
        m_LicNewMode = false;
    }

    protected void TerrInitMapWidget(MapWidget mapWidget)
    {
        if (!mapWidget) return;
        string worldName = GetGame().GetWorldName();
        vector center = GetGame().ConfigGetVector("CfgWorlds " + worldName + " centerPosition");
        if (center[0] == 0 && center[2] == 0)
        {
            int worldSize = GetGame().GetWorld().GetWorldSize();
            center = Vector(worldSize * 0.5, 0, worldSize * 0.5);
        }
        mapWidget.SetScale(0.33);
        mapWidget.SetMapPos(center);
    }

    protected void TerrInitMapView()
    {
        TerrInitMapWidget(m_TerrMap);
        TerrInitMapWidget(m_ChainMap);
    }

    protected TFLTerritoryDto TerrSelected()
    {
        if (!s_Panel)
            return null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.id == m_TerrSelectedId)
                return territory;
        }
        return null;
    }

    protected bool TerrDayAllowed(TFLTerritoryDto territory, int dayIndex)
    {
        if (!territory)
            return false;
        if (!territory.allowed_days)
            return false;
        return territory.allowed_days.Find(dayIndex) != -1;
    }

    protected string TerrDayName(int index)
    {
        string result = "ВС";
        if (index == 0)
            result = "ПН";
        else if (index == 1)
            result = "ВТ";
        else if (index == 2)
            result = "СР";
        else if (index == 3)
            result = "ЧТ";
        else if (index == 4)
            result = "ПТ";
        else if (index == 5)
            result = "СБ";
        return result;
    }

    protected TFLTerritoryDto TerrById(string id)
    {
        if (!s_Panel) return null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.id == id) return territory;
        }
        return null;
    }

    protected void TerrAddChainSegmentOnMap(MapWidget targetMap, TFLTerritoryDto fromTerr, TFLTerritoryDto toTerr, int color)
    {
        if (!targetMap || !fromTerr || !toTerr) return;
        int pieces = 8;
        for (int i = 1; i < pieces; i++)
        {
            float t = (i * 1.0) / pieces;
            vector dot = Vector(fromTerr.x + (toTerr.x - fromTerr.x) * t, fromTerr.y + (toTerr.y - fromTerr.y) * t, fromTerr.z + (toTerr.z - fromTerr.z) * t);
            targetMap.AddUserMark(dot, "", color, "TFL_Pasport/gui/textures/pass_white.edds");
        }
    }

    protected void TerrRefreshMarksOnMap(MapWidget targetMap, bool chainMode)
    {
        if (!targetMap || !s_Panel) return;
        targetMap.ClearUserMarks();
        TFLTerritoryChainDto editChain = null;
        if (chainMode && m_SelectedChainId != "") editChain = ChainById(m_SelectedChainId);
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory) continue;
            int color = ARGB(255, 210, 180, 70);
            if (territory.owner_side_id == TFLFactionSide.VSRF) color = ARGB(255, 80, 170, 240);
            else if (territory.owner_side_id == TFLFactionSide.TERRORISTS) color = ARGB(255, 220, 75, 70);

            string markerText = territory.name + " // " + territory.status;
            if (chainMode)
            {
                int chainIndex = -1;
                if (editChain) chainIndex = editChain.territory_ids.Find(territory.id);
                if (chainIndex >= 0)
                {
                    markerText = "[" + (chainIndex + 1).ToString() + "]  " + territory.name;
                    color = ARGB(255, 185, 220, 95);
                }
                else markerText = "[ + ]  " + territory.name;
            }
            else if (territory.chain_id != "")
                markerText = "[" + territory.chain_index.ToString() + "/" + territory.chain_total.ToString() + "] " + markerText;

            targetMap.AddUserMark(Vector(territory.x, territory.y, territory.z), markerText, color, "TFL_Pasport/gui/textures/seals/pass_seal_rad.edds");
        }

        if (chainMode && editChain && editChain.territory_ids.Count() > 1)
        {
            int lineColor = ARGB(255, 185, 220, 95);
            for (int c = 0; c < editChain.territory_ids.Count() - 1; c++)
            {
                TFLTerritoryDto fromTerr = TerrById(editChain.territory_ids.Get(c));
                TFLTerritoryDto toTerr = TerrById(editChain.territory_ids.Get(c + 1));
                if (!fromTerr || !toTerr) continue;
                TerrAddChainSegmentOnMap(targetMap, fromTerr, toTerr, lineColor);
                vector mid = Vector((fromTerr.x + toTerr.x) * 0.5, (fromTerr.y + toTerr.y) * 0.5, (fromTerr.z + toTerr.z) * 0.5);
                targetMap.AddUserMark(mid, "→ " + (c + 2).ToString(), lineColor, "TFL_Pasport/gui/textures/seals/pass_seal_rad.edds");
            }
        }
    }

    protected void TerrRefreshMarks()
    {
        if (m_ChainEditMode && m_ChainPanel && m_ChainPanel.IsVisible()) TerrRefreshMarksOnMap(m_ChainMap, true);
        else TerrRefreshMarksOnMap(m_TerrMap, false);
    }

    protected void TerrRebuildRows()
    {
        if (!m_TerrListHost || !s_Panel)
            return;
        ClearWidgets(m_TerritoryRows);
        int count = s_Panel.territories.Count();
        float content = count * 0.16;
        if (content < 1.0)
            content = 1.0;
        m_TerrListHost.SetSize(1, content);
        float step = 0.16 / content;
        for (int i = 0; i < count; i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/territory_admin_row.layout", m_TerrListHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_TerritoryRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget meta = TextWidget.Cast(row.FindAnyWidget("RowMeta"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name) name.SetText(territory.name);
            if (meta)
            {
                string info = territory.status + " // ";
                if (territory.chain_id != "") info = info + territory.chain_name + " [" + territory.chain_index.ToString() + "/" + territory.chain_total.ToString() + "]";
                else info = info + "БЕЗ ЦЕПИ";
                meta.SetText(info);
            }
            if (mark)
                mark.Show(territory.id == m_TerrSelectedId);
        }
    }

    protected void TerrRebuildRewards(TFLTerritoryDto territory)
    {
        if (!m_TerrRewardHost)
            return;
        ClearWidgets(m_TerrRewardRows);
        if (!territory)
            return;
        int count = territory.rewards.Count();
        float content = count * 0.18;
        if (content < 1.0)
            content = 1.0;
        m_TerrRewardHost.SetSize(1, content);
        float step = 0.18 / content;
        for (int i = 0; i < count; i++)
        {
            TFLTerritoryRewardDto reward = territory.rewards.Get(i);
            if (!reward)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_faction_row.layout", m_TerrRewardHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_TerrRewardRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget countText = TextWidget.Cast(row.FindAnyWidget("RowCount"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(reward.class_name);
            if (countText)
                countText.SetText("x" + reward.count.ToString());
            if (mark)
                mark.Show(i == m_TerrRewardSelected);
        }
    }

    protected void TerrRefreshEditor()
    {
        TFLTerritoryDto territory = TerrSelected();
        if (!territory)
        {
            TerrRebuildRewards(null);
            return;
        }

        if (m_TerrId)
            m_TerrId.SetText(territory.id);
        if (m_TerrName)
            m_TerrName.SetText(territory.name);
        if (m_TerrRadius)
            m_TerrRadius.SetText(territory.radius.ToString());
        if (m_TerrMinPlayers)
            m_TerrMinPlayers.SetText(territory.min_players.ToString());
        if (m_TerrCaptureMin)
            m_TerrCaptureMin.SetText((territory.capture_seconds / 60).ToString());
        if (m_TerrPrepMin)
            m_TerrPrepMin.SetText((territory.preparation_seconds / 60).ToString());
        if (m_TerrCooldownMin)
            m_TerrCooldownMin.SetText((territory.cooldown_seconds / 60).ToString());
        if (m_TerrParticipation)
            m_TerrParticipation.SetText(territory.participation_percent.ToString());
        if (m_TerrStartHour)
            m_TerrStartHour.SetText(territory.start_hour.ToString());
        if (m_TerrEndHour)
            m_TerrEndHour.SetText(territory.end_hour.ToString());
        if (m_TerrMapPoint)
            m_TerrMapPoint.SetText(territory.name);
        if (m_TerrDirectCapture)
        {
            string directMark = "[ ]";
            if (territory.direct_capture == 1) directMark = "ВКЛ";
            else directMark = "ВЫКЛ";
            m_TerrDirectCapture.SetText(directMark);
        }

        // Daily capture window (day-of-week / start-end hour) has no widget in the
        // redesigned point editor; m_TerrDays stays empty, this loop is now a no-op.
        for (int d = 0; d < m_TerrDays.Count(); d++)
        {
            ButtonWidget dayButton = m_TerrDays.Get(d);
            if (!dayButton)
                continue;
            string mark = "[ ] ";
            if (TerrDayAllowed(territory, d + 1))
                mark = "[X] ";
            dayButton.SetText(mark + TerrDayName(d));
        }

        if (m_TerrChainName)
        {
            if (territory.chain_id != "") m_TerrChainName.SetText(territory.chain_name);
            else m_TerrChainName.SetText("БЕЗ ЦЕПОЧКИ");
        }
        if (m_TerrChainPosition)
        {
            if (territory.chain_id != "") m_TerrChainPosition.SetText(territory.chain_index.ToString() + " / " + territory.chain_total.ToString());
            else m_TerrChainPosition.SetText("-");
        }
        if (m_TerrChainPrevious)
        {
            if (territory.chain_prev_name != "") m_TerrChainPrevious.SetText(territory.chain_prev_name);
            else m_TerrChainPrevious.SetText("-");
        }
        if (m_TerrChainNext)
        {
            if (territory.chain_next_name != "") m_TerrChainNext.SetText(territory.chain_next_name);
            else m_TerrChainNext.SetText("-");
        }
        if (m_TerrRewardNameText)
        {
            if (territory.reward_preset_name != "") m_TerrRewardNameText.SetText(territory.reward_preset_name);
            else m_TerrRewardNameText.SetText("НЕ НАЗНАЧЕНА");
        }
    }

    protected void TerrRefresh()
    {
        if (!s_Panel || !M_SUB_WIDGET)
            return;
        if (!m_TerrMap)
            return;
        if (m_TerrSelectedId == "" && s_Panel.territories.Count() > 0)
            m_TerrSelectedId = s_Panel.territories.Get(0).id;
        if (m_TerrSelectedId != "" && TerrSelected())
            m_TerrHaveMapPoint = false;
        if (m_TerrDailyLimit && s_Panel.territory_settings)
            m_TerrDailyLimit.SetText(s_Panel.territory_settings.daily_capture_limit.ToString());
        TerrRebuildRows();
        TerrRefreshMarks();
        TerrRefreshEditor();
    }

    protected void TerrSelectNearestOnMap(MapWidget sourceMap, int x, int y, bool chainMode)
    {
        if (!sourceMap || !s_Panel) return;
        float best = 6400.0;
        string bestId = "";
        float mapX, mapY;
        sourceMap.GetScreenPos(mapX, mapY);
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory) continue;
            vector screen = sourceMap.MapToScreen(Vector(territory.x, territory.y, territory.z));
            float dxAbs = screen[0] - x;
            float dyAbs = screen[1] - y;
            float distanceAbs = dxAbs * dxAbs + dyAbs * dyAbs;
            float localMarkerX = screen[0] - mapX;
            float localMarkerY = screen[1] - mapY;
            float dxLocal = localMarkerX - x;
            float dyLocal = localMarkerY - y;
            float distanceLocal = dxLocal * dxLocal + dyLocal * dyLocal;
            float distanceSq = distanceAbs;
            if (distanceLocal < distanceSq) distanceSq = distanceLocal;
            if (distanceSq < best)
            {
                best = distanceSq;
                bestId = territory.id;
            }
        }
        if (bestId != "")
        {
            if (chainMode && m_SelectedChainId != "")
            {
                QueueOrSendChainPoint(bestId);
                return;
            }
            if (chainMode) return;
            m_TerrSelectedId = bestId;
            m_TerrHaveMapPoint = false;
            m_TerrRewardSelected = -1;
            TerrRefresh();
            return;
        }
        if (chainMode) return;
        m_TerrClickPosition = sourceMap.ScreenToMap(Vector(x, y, 0));
        float terrainY = GetGame().SurfaceY(m_TerrClickPosition[0], m_TerrClickPosition[2]);
        float roadY = GetGame().SurfaceRoadY(m_TerrClickPosition[0], m_TerrClickPosition[2]);
        if (roadY > terrainY) terrainY = roadY;
        m_TerrClickPosition[1] = terrainY;
        m_TerrHaveMapPoint = true;
        m_TerrSelectedId = "";
        m_TerrRewardSelected = -1;
        if (m_TerrId) m_TerrId.SetText("");
        if (m_TerrName) m_TerrName.SetText("");
        if (m_TerrMapPoint) m_TerrMapPoint.SetText("НОВАЯ ТОЧКА: " + m_TerrClickPosition[0].ToString() + " / " + m_TerrClickPosition[2].ToString());
        TerrRebuildRows();
        TerrRebuildRewards(null);
    }

    protected void TerrSelectNearest(int x, int y)
    {
        TerrSelectNearestOnMap(m_TerrMap, x, y, false);
    }

    protected void TerrCreateTerritory()
    {
        if (!m_TerrHaveMapPoint)
        {
            if (m_TerrMapPoint)
                m_TerrMapPoint.SetText("СНАЧАЛА КЛИКНИТЕ ПО КАРТЕ");
            return;
        }
        if (!m_TerrId || !m_TerrName)
            return;
        string safeId = TFLFactionText.SafeId(m_TerrId.GetText());
        string territoryName = TFLPassText.Safe(m_TerrName.GetText(), 64);
        if (safeId == "")
        {
            if (m_TerrMapPoint)
                m_TerrMapPoint.SetText("ОШИБКА: УКАЖИТЕ ID ТЕРРИТОРИИ");
            return;
        }
        if (territoryName == "")
        {
            if (m_TerrMapPoint)
                m_TerrMapPoint.SetText("ОШИБКА: УКАЖИТЕ НАЗВАНИЕ");
            return;
        }
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = safeId;
        dto.territory_name = territoryName;
        dto.x = m_TerrClickPosition[0];
        dto.y = m_TerrClickPosition[1];
        dto.z = m_TerrClickPosition[2];
        SendCommand(TFLFactionCommand.ADMIN_TERRITORY_CREATE, dto);
        m_TerrSelectedId = safeId;
        if (m_TerrMapPoint)
            m_TerrMapPoint.SetText("СОЗДАНИЕ: " + safeId + " // ожидание сервера");
    }

    protected void TerrSaveTerritory()
    {
        if (m_TerrSelectedId == "")
            return;
        TFLTerritoryDto selected = TerrSelected();
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_TerrSelectedId;
        if (m_TerrName)
            dto.territory_name = m_TerrName.GetText();
        if (selected)
        {
            dto.x = selected.x;
            dto.y = selected.y;
            dto.z = selected.z;
        }
        if (m_TerrRadius)
            dto.radius = m_TerrRadius.GetText().ToFloat();
        if (m_TerrMinPlayers)
            dto.min_players = m_TerrMinPlayers.GetText().ToInt();
        if (m_TerrCaptureMin)
            dto.capture_seconds = m_TerrCaptureMin.GetText().ToInt() * 60;
        if (m_TerrPrepMin)
            dto.preparation_seconds = m_TerrPrepMin.GetText().ToInt() * 60;
        if (m_TerrCooldownMin)
            dto.cooldown_seconds = m_TerrCooldownMin.GetText().ToInt() * 60;
        if (m_TerrParticipation)
            dto.participation_percent = m_TerrParticipation.GetText().ToInt();
        if (m_TerrStartHour)
            dto.start_hour = m_TerrStartHour.GetText().ToInt();
        if (m_TerrEndHour)
            dto.end_hour = m_TerrEndHour.GetText().ToInt();
        SendCommand(TFLFactionCommand.ADMIN_TERRITORY_SAVE, dto);
    }


    protected TFLTerritoryRewardPresetDto RewardPresetById(string id)
    {
        if (!s_Panel) return null;
        for (int i = 0; i < s_Panel.reward_presets.Count(); i++)
        {
            TFLTerritoryRewardPresetDto preset = s_Panel.reward_presets.Get(i);
            if (preset && preset.id == id) return preset;
        }
        return null;
    }

    protected void RewardPresetOpen()
    {
        if (!m_RewardPresetPanel) return;
        TerrSetRewardMode();
        if (m_SelectedRewardPresetId == "" && m_TerrSelectedId != "")
        {
            TFLTerritoryDto terr = TerrSelected();
            if (terr) m_SelectedRewardPresetId = terr.reward_preset_id;
        }
        RewardPresetRefresh();
    }

    protected void RewardPresetClose()
    {
        if (m_RewardPresetPanel) m_RewardPresetPanel.Show(false);
        RewardPreviewClear();
        if (m_TerritoriesPanel)
        {
            TerrSetPointMode();
            TerrRefresh();
        }
    }

    protected void RewardPresetRefresh()
    {
        if (!s_Panel) return;
        ClearWidgets(m_RewardPresetRows);
        ClearWidgets(m_RewardContentRows);

        int presetCount = s_Panel.reward_presets.Count();
        if (m_RewardPresetList)
        {
            float presetContent = presetCount * 0.13;
            if (presetContent < 1.0) presetContent = 1.0;
            m_RewardPresetList.SetSize(1, presetContent);
            float presetStep = 0.13 / presetContent;
            for (int i = 0; i < presetCount; i++)
            {
                TFLTerritoryRewardPresetDto p = s_Panel.reward_presets.Get(i);
                if (!p) continue;
                Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_RewardPresetRow.layout", m_RewardPresetList);
                if (!row) continue;
                row.SetPos(0, i * presetStep);
                row.SetSize(1, presetStep);
                Widget mark = row.FindAnyWidget("RowMark");
                TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
                TextWidget meta = TextWidget.Cast(row.FindAnyWidget("RowMeta"));
                if (mark) mark.Show(p.id == m_SelectedRewardPresetId);
                if (name) name.SetText(p.name);
                if (meta) meta.SetText(p.id + " // " + p.items.Count().ToString() + " ПОЗ.");
                BindDynamicRow(row);
                m_RewardPresetRows.Insert(row);
            }
        }

        TFLTerritoryRewardPresetDto preset = RewardPresetById(m_SelectedRewardPresetId);
        if (m_RewardPresetId)
        {
            if (preset) m_RewardPresetId.SetText(preset.id);
            else if (m_SelectedRewardPresetId == "") m_RewardPresetId.SetText("");
        }
        if (m_RewardPresetName)
        {
            if (preset) m_RewardPresetName.SetText(preset.name);
            else if (m_SelectedRewardPresetId == "") m_RewardPresetName.SetText("");
        }

        if (m_RewardPresetItems && preset)
        {
            int itemCount = preset.items.Count();
            float content = itemCount * 0.24;
            if (content < 1.0) content = 1.0;
            m_RewardPresetItems.SetSize(1, content);
            float step = 0.24 / content;
            for (int r = 0; r < itemCount; r++)
            {
                TFLTerritoryRewardDto item = preset.items.Get(r);
                if (!item) continue;
                Widget itemRow = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_RewardContentRow.layout", m_RewardPresetItems);
                if (!itemRow) continue;
                itemRow.SetPos(0, r * step);
                itemRow.SetSize(1, step);
                TextWidget rowName = TextWidget.Cast(itemRow.FindAnyWidget("RowName"));
                TextWidget rowMeta = TextWidget.Cast(itemRow.FindAnyWidget("RowMeta"));
                TextWidget rowQty = TextWidget.Cast(itemRow.FindAnyWidget("RowQty"));
                if (rowName) rowName.SetText(ConfigDisplayName(item.class_name));
                if (rowMeta) rowMeta.SetText(item.class_name);
                if (rowQty) rowQty.SetText("x" + item.count.ToString());
                itemRow.SetHandler(this);
                Widget rowBtn = itemRow.FindAnyWidget("RowBtn");
                Widget removeBtn = itemRow.FindAnyWidget("RewardContentRemoveButton");
                if (rowBtn) rowBtn.SetHandler(this);
                if (removeBtn) removeBtn.SetHandler(this);
                m_RewardContentRows.Insert(itemRow);
            }
        }
    }

    protected bool TrySelectRewardPresetFromWidget(Widget w)
    {
        if (!w || !s_Panel) return false;
        for (int i = 0; i < m_RewardPresetRows.Count(); i++)
        {
            Widget row = m_RewardPresetRows.Get(i);
            if (!row || !WidgetInsideRow(w, row)) continue;
            if (i >= s_Panel.reward_presets.Count()) return true;
            TFLTerritoryRewardPresetDto preset = s_Panel.reward_presets.Get(i);
            if (preset) m_SelectedRewardPresetId = preset.id;
            m_SelectedRewardItemIndex = -1;
            RewardPresetRefresh();
            return true;
        }
        return false;
    }

    protected void RewardPresetCreate()
    {
        if (!m_RewardPresetId || !m_RewardPresetName) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.reward_preset_id = TFLFactionText.SafeId(m_RewardPresetId.GetText());
        dto.reward_preset_name = TFLPassText.Safe(m_RewardPresetName.GetText(), 64);
        if (dto.reward_preset_id == "")
        {
            dto.reward_preset_id = "reward_" + Math.RandomInt(1000, 9999).ToString();
            m_RewardPresetId.SetText(dto.reward_preset_id);
        }
        if (dto.reward_preset_name == "") dto.reward_preset_name = dto.reward_preset_id;
        m_SelectedRewardPresetId = dto.reward_preset_id;
        SendCommand(TFLFactionCommand.ADMIN_REWARD_PRESET_CREATE, dto);
    }

    protected void RewardPresetSave()
    {
        if (!m_RewardPresetId || !m_RewardPresetName) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.reward_preset_id = TFLFactionText.SafeId(m_RewardPresetId.GetText());
        dto.reward_preset_name = TFLPassText.Safe(m_RewardPresetName.GetText(), 64);
        if (dto.reward_preset_id == "") return;
        m_SelectedRewardPresetId = dto.reward_preset_id;
        SendCommand(TFLFactionCommand.ADMIN_REWARD_PRESET_SAVE, dto);
    }

    protected void RewardPresetDelete()
    {
        if (m_SelectedRewardPresetId == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.reward_preset_id = m_SelectedRewardPresetId;
        SendCommand(TFLFactionCommand.ADMIN_REWARD_PRESET_DELETE, dto);
        m_SelectedRewardPresetId = "";
        m_SelectedRewardItemIndex = -1;
    }

    protected void RewardPresetAssign()
    {
        if (m_TerrSelectedId == "" || m_SelectedRewardPresetId == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_TerrSelectedId;
        dto.reward_preset_id = m_SelectedRewardPresetId;
        SendCommand(TFLFactionCommand.ADMIN_TERRITORY_SET_REWARD_PRESET, dto);
    }

    protected void RewardItemSearchConfig(string root, string query)
    {
        if (m_RewardSearchClasses.Count() >= 120) return;
        int count = GetGame().ConfigGetChildrenCount(root);
        for (int i = 0; i < count; i++)
        {
            if (m_RewardSearchClasses.Count() >= 120) return;
            string className;
            GetGame().ConfigGetChildName(root, i, className);
            if (className == "") continue;
            string display = ConfigDisplayName(className);
            string hay = className + " " + display;
            hay.ToLower();
            if (query != "" && hay.IndexOf(query) < 0) continue;
            m_RewardSearchClasses.Insert(className);
        }
    }

    protected void RewardItemSearch()
    {
        ClearWidgets(m_RewardSearchRows);
        m_RewardSearchClasses.Clear();
        string query = "";
        if (m_RewardItemSearch) query = m_RewardItemSearch.GetText();
        query.ToLower();
        RewardItemSearchConfig("CfgVehicles", query);
        RewardItemSearchConfig("CfgWeapons", query);
        RewardItemSearchConfig("CfgMagazines", query);
        if (!m_RewardSearchResults) return;
        int count = m_RewardSearchClasses.Count();
        float content = count * 0.25;
        if (content < 1.0) content = 1.0;
        m_RewardSearchResults.SetSize(1, content);
        float step = 0.25 / content;
        for (int i = 0; i < count; i++)
        {
            string className = m_RewardSearchClasses.Get(i);
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_RewardSearchRow.layout", m_RewardSearchResults);
            if (!row) continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget meta = TextWidget.Cast(row.FindAnyWidget("RowMeta"));
            if (name) name.SetText(ConfigDisplayName(className));
            if (meta) meta.SetText(className);
            BindDynamicRow(row);
            m_RewardSearchRows.Insert(row);
        }
    }

    protected void RewardPreviewClear()
    {
        if (m_RewardItemPreview) m_RewardItemPreview.SetItem(null);
        if (m_RewardPreviewEntity)
        {
            GetGame().ObjectDelete(m_RewardPreviewEntity);
            m_RewardPreviewEntity = null;
        }
    }

    protected void RewardPreviewClass(string className)
    {
        RewardPreviewClear();
        m_RewardSelectedClassName = className;
        if (className == "") return;
        if (m_RewardItemClass) m_RewardItemClass.SetText("Classname: " + className);
        if (m_RewardItemPreviewName) m_RewardItemPreviewName.SetText(ConfigDisplayName(className));
        if (!GetGame().ConfigIsExisting("CfgVehicles " + className) && !GetGame().ConfigIsExisting("CfgWeapons " + className) && !GetGame().ConfigIsExisting("CfgMagazines " + className)) return;
        Object previewObject = GetGame().CreateObjectEx(className, "0 0 0", ECE_LOCAL | ECE_NOLIFETIME | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN);
        m_RewardPreviewEntity = EntityAI.Cast(previewObject);
        if (m_RewardItemPreview && m_RewardPreviewEntity)
        {
            m_RewardItemPreview.SetItem(m_RewardPreviewEntity);
            m_RewardItemPreview.SetView(m_RewardPreviewEntity.GetViewIndex());
            m_RewardItemPreview.SetModelPosition(Vector(0, 0, 0.5));
        }
    }

    protected bool TrySelectRewardSearchFromWidget(Widget w)
    {
        if (!w) return false;
        for (int i = 0; i < m_RewardSearchRows.Count(); i++)
        {
            Widget row = m_RewardSearchRows.Get(i);
            if (!row || !WidgetInsideRow(w, row)) continue;
            if (i < m_RewardSearchClasses.Count()) RewardPreviewClass(m_RewardSearchClasses.Get(i));
            return true;
        }
        return false;
    }

    protected bool TryHandleRewardContentWidget(Widget w)
    {
        if (!w) return false;
        TFLTerritoryRewardPresetDto preset = RewardPresetById(m_SelectedRewardPresetId);
        if (!preset) return false;
        for (int i = 0; i < m_RewardContentRows.Count(); i++)
        {
            Widget row = m_RewardContentRows.Get(i);
            if (!row || !WidgetInsideRow(w, row)) continue;
            if (i >= preset.items.Count()) return true;
            m_SelectedRewardItemIndex = i;
            TFLTerritoryRewardDto item = preset.items.Get(i);
            if (item) RewardPreviewClass(item.class_name);
            Widget removeBtn = row.FindAnyWidget("RewardContentRemoveButton");
            if (removeBtn && WidgetInsideRow(w, removeBtn)) RewardItemRemoveFromPreset();
            return true;
        }
        return false;
    }

    protected void RewardItemAddToPreset()
    {
        if (m_SelectedRewardPresetId == "" || m_RewardSelectedClassName == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.reward_preset_id = m_SelectedRewardPresetId;
        dto.class_name = m_RewardSelectedClassName;
        dto.reward_count = 1;
        if (m_RewardItemCount) dto.reward_count = m_RewardItemCount.GetText().ToInt();
        if (dto.reward_count < 1) dto.reward_count = 1;
        SendCommand(TFLFactionCommand.ADMIN_REWARD_PRESET_ADD_ITEM, dto);
    }

    protected void RewardItemRemoveFromPreset()
    {
        if (m_SelectedRewardPresetId == "" || m_SelectedRewardItemIndex < 0) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.reward_preset_id = m_SelectedRewardPresetId;
        dto.int_value = m_SelectedRewardItemIndex;
        SendCommand(TFLFactionCommand.ADMIN_REWARD_PRESET_REMOVE_ITEM, dto);
        m_SelectedRewardItemIndex = -1;
    }

    protected TFLTerritoryChainDto ChainById(string id)
    {
        if (!s_Panel) return null;
        for (int i = 0; i < s_Panel.territory_chains.Count(); i++)
        {
            TFLTerritoryChainDto chain = s_Panel.territory_chains.Get(i);
            if (chain && chain.id == id) return chain;
        }
        return null;
    }

    protected void ChainOpen()
    {
        if (!m_ChainPanel) return;
        RewardPreviewClear();
        TerrSetChainMode();
        if (m_SelectedChainId == "" && m_TerrSelectedId != "")
        {
            TFLTerritoryDto terr = TerrSelected();
            if (terr) m_SelectedChainId = terr.chain_id;
        }
        ChainRefresh();
        TerrRefreshMarks();
    }

    protected void ChainCloseEditor()
    {
        if (m_ChainPanel) m_ChainPanel.Show(false);
        TerrSetPointMode();
        TerrRefresh();
    }

    protected void ChainRefresh()
    {
        if (!s_Panel) return;
        if (m_ChainList)
        {
            m_ChainList.ClearItems();
            for (int i = 0; i < s_Panel.territory_chains.Count(); i++)
            {
                TFLTerritoryChainDto c = s_Panel.territory_chains.Get(i);
                if (!c) continue;
                m_ChainList.AddItem(c.name + "   //   " + c.side_a_name + " → " + c.side_b_name + "   // " + c.territory_ids.Count().ToString() + " ТОЧЕК", null, 0);
            }
        }
        TFLTerritoryChainDto chain = ChainById(m_SelectedChainId);
        if (chain)
        {
            m_ChainSideAId = chain.side_a;
            m_ChainSideBId = chain.side_b;
            if (m_ChainId) m_ChainId.SetText(chain.id);
            if (m_ChainName) m_ChainName.SetText(chain.name);
        }
        else
        {
            if (m_ChainId && m_SelectedChainId == "") m_ChainId.SetText("");
            if (m_ChainName && m_SelectedChainId == "") m_ChainName.SetText("");
        }
        if (m_ChainSideAText) m_ChainSideAText.SetText(TFLFactionSide.Display(m_ChainSideAId));
        if (m_ChainSideBText) m_ChainSideBText.SetText(TFLFactionSide.Display(m_ChainSideBId));
        if (m_ChainSideA) m_ChainSideA.SetText("...");
        if (m_ChainSideB) m_ChainSideB.SetText("...");

        ClearWidgets(m_ChainPointRows);
        string path = "КЛИКАЙТЕ ТОЧКИ НА КАРТЕ ПО ПОРЯДКУ";
        if (chain && m_ChainPointList)
        {
            int count = chain.territory_ids.Count();
            float content = count * 0.28;
            if (content < 1.0) content = 1.0;
            m_ChainPointList.SetSize(1, content);
            float step = 0.28 / content;
            path = "";
            for (int p = 0; p < count; p++)
            {
                if (p > 0) path = path + "  →  ";
                string pointName = chain.territory_names.Get(p);
                path = path + (p + 1).ToString() + ". " + pointName;
                Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryChainPointRow.layout", m_ChainPointList);
                if (!row) continue;
                row.SetPos(0, p * step);
                row.SetSize(1, step);
                TextWidget idx = TextWidget.Cast(row.FindAnyWidget("ChainPointIndex"));
                TextWidget name = TextWidget.Cast(row.FindAnyWidget("ChainPointName"));
                TextWidget owner = TextWidget.Cast(row.FindAnyWidget("ChainPointOwner"));
                if (idx) idx.SetText((p + 1).ToString());
                if (name) name.SetText(pointName);
                TFLTerritoryDto terr = TerrById(chain.territory_ids.Get(p));
                if (owner && terr) owner.SetText(terr.owner_side_name);
                row.SetHandler(this);
                Widget rowBtn = row.FindAnyWidget("RowBtn");
                Widget up = row.FindAnyWidget("ChainPointUpButton");
                Widget down = row.FindAnyWidget("ChainPointDownButton");
                Widget remove = row.FindAnyWidget("ChainPointRemoveButton");
                if (rowBtn) rowBtn.SetHandler(this);
                if (up) up.SetHandler(this);
                if (down) down.SetHandler(this);
                if (remove) remove.SetHandler(this);
                m_ChainPointRows.Insert(row);
            }
            if (path == "") path = "ЦЕПОЧКА ПУСТА // КЛИКАЙТЕ МАРКЕРЫ НА КАРТЕ";
        }
        if (m_ChainPath) m_ChainPath.SetText(path);
    }

    protected void ChainSelectRow()
    {
        if (!m_ChainList || !s_Panel) return;
        int row = m_ChainList.GetSelectedRow();
        if (row < 0 || row >= s_Panel.territory_chains.Count()) return;
        TFLTerritoryChainDto chain = s_Panel.territory_chains.Get(row);
        if (!chain) return;
        m_SelectedChainId = chain.id;
        m_SelectedChainPointIndex = -1;
        ChainRefresh();
        TerrRefreshMarks();
    }

    protected void ChainNewEditor()
    {
        m_ChainSideAId = TFLFactionSide.VSRF;
        m_ChainSideBId = TFLFactionSide.TERRORISTS;
        m_SelectedChainPointIndex = -1;
        if (m_PendingChainPoints) m_PendingChainPoints.Clear();
        string draftId = "front_" + Math.RandomInt(100, 999).ToString();
        string draftName = "Новая линия фронта";
        m_SelectedChainId = draftId;
        if (m_ChainId) m_ChainId.SetText(draftId);
        if (m_ChainName) m_ChainName.SetText(draftName);
        if (m_ChainSideAText) m_ChainSideAText.SetText(TFLFactionSide.Display(m_ChainSideAId));
        if (m_ChainSideBText) m_ChainSideBText.SetText(TFLFactionSide.Display(m_ChainSideBId));
        ClearWidgets(m_ChainPointRows);
        if (m_ChainPath) m_ChainPath.SetText("ЦЕПОЧКА СОЗДАНА // КЛИКАЙТЕ ТОЧКИ НА КАРТЕ ПО ПОРЯДКУ");

        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.chain_id = draftId;
        dto.chain_name = draftName;
        dto.chain_side_a = m_ChainSideAId;
        dto.chain_side_b = m_ChainSideBId;
        SendCommand(TFLFactionCommand.ADMIN_CHAIN_CREATE, dto);
    }

    protected void ChainCreateOrSave(bool create)
    {
        if (!m_ChainId || !m_ChainName) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.chain_id = TFLFactionText.SafeId(m_ChainId.GetText());
        dto.chain_name = TFLPassText.Safe(m_ChainName.GetText(), 64);
        dto.chain_side_a = m_ChainSideAId;
        dto.chain_side_b = m_ChainSideBId;
        if (dto.chain_id == "") return;
        m_SelectedChainId = dto.chain_id;
        if (create) SendCommand(TFLFactionCommand.ADMIN_CHAIN_CREATE, dto);
        else SendCommand(TFLFactionCommand.ADMIN_CHAIN_SAVE, dto);
    }

    protected void ChainDeleteSelected()
    {
        if (m_SelectedChainId == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.chain_id = m_SelectedChainId;
        SendCommand(TFLFactionCommand.ADMIN_CHAIN_DELETE, dto);
        m_SelectedChainId = "";
        m_SelectedChainPointIndex = -1;
    }

    protected void ChainCycleSide(bool first)
    {
        if (first)
        {
            m_ChainSideAId = TFLFactionSide.Enemy(m_ChainSideAId);
            if (m_ChainSideAId == m_ChainSideBId) m_ChainSideBId = TFLFactionSide.Enemy(m_ChainSideAId);
        }
        else
        {
            m_ChainSideBId = TFLFactionSide.Enemy(m_ChainSideBId);
            if (m_ChainSideBId == m_ChainSideAId) m_ChainSideAId = TFLFactionSide.Enemy(m_ChainSideBId);
        }
        if (m_ChainSideAText) m_ChainSideAText.SetText(TFLFactionSide.Display(m_ChainSideAId));
        if (m_ChainSideBText) m_ChainSideBText.SetText(TFLFactionSide.Display(m_ChainSideBId));
    }

    protected void ChainPointMove(int delta)
    {
        TFLTerritoryChainDto chain = ChainById(m_SelectedChainId);
        if (!chain || m_SelectedChainPointIndex < 0 || m_SelectedChainPointIndex >= chain.territory_ids.Count()) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.chain_id = chain.id;
        dto.target_territory_id = chain.territory_ids.Get(m_SelectedChainPointIndex);
        dto.int_value = delta;
        SendCommand(TFLFactionCommand.ADMIN_CHAIN_MOVE_POINT, dto);
    }

    protected void ChainPointRemoveSelected()
    {
        TFLTerritoryChainDto chain = ChainById(m_SelectedChainId);
        if (!chain || m_SelectedChainPointIndex < 0 || m_SelectedChainPointIndex >= chain.territory_ids.Count()) return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.chain_id = chain.id;
        dto.target_territory_id = chain.territory_ids.Get(m_SelectedChainPointIndex);
        SendCommand(TFLFactionCommand.ADMIN_CHAIN_REMOVE_POINT, dto);
        m_SelectedChainPointIndex = -1;
    }

    protected bool TryHandleChainPointWidget(Widget w)
    {
        if (!w) return false;
        for (int i = 0; i < m_ChainPointRows.Count(); i++)
        {
            Widget row = m_ChainPointRows.Get(i);
            if (!row || !WidgetInsideRow(w, row)) continue;
            m_SelectedChainPointIndex = i;
            Widget up = row.FindAnyWidget("ChainPointUpButton");
            Widget down = row.FindAnyWidget("ChainPointDownButton");
            Widget remove = row.FindAnyWidget("ChainPointRemoveButton");
            if (up && WidgetInsideRow(w, up)) ChainPointMove(-1);
            else if (down && WidgetInsideRow(w, down)) ChainPointMove(1);
            else if (remove && WidgetInsideRow(w, remove)) ChainPointRemoveSelected();
            return true;
        }
        return false;
    }

    protected void TerrSendOnly(int command)
    {
        if (m_TerrSelectedId == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_TerrSelectedId;
        SendCommand(command, dto);
    }

    override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
    {
        super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
        if (w == m_RankMemberList)
        {
            int selected = m_RankMemberList.GetSelectedRow();
            if (s_Panel && selected >= 0 && selected < s_Panel.members.Count())
            {
                TFLFactionMemberDto member = s_Panel.members.Get(selected);
                if (member) m_SelectedMember = member.uid;
                RefreshRankEditor();
            }
            return true;
        }
        if (w == m_ChainList) { ChainSelectRow(); return true; }
        return false;
    }

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
    {
        super.OnMouseButtonDown(w, x, y, button);
        if (button == MouseState.LEFT && TrySelectFactionFromWidget(w)) return true;
        if (w == m_TerrMap && button == MouseState.LEFT)
        {
            TerrSelectNearestOnMap(m_TerrMap, x, y, false);
            return true;
        }
        if (w == m_ChainMap && button == MouseState.LEFT)
        {
            TerrSelectNearestOnMap(m_ChainMap, x, y, true);
            return true;
        }
        return false;
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (w == m_SpawnYawSlider)
        {
            if (m_SpawnYawValue) m_SpawnYawValue.SetText("YAW " + Math.Round(m_SpawnYawSlider.GetCurrent()).ToString() + "°");
            return true;
        }
        if (w == m_LaptopYawSlider)
        {
            if (m_LaptopYawValue) m_LaptopYawValue.SetText("YAW " + Math.Round(m_LaptopYawSlider.GetCurrent()).ToString() + "°");
            return true;
        }
        return super.OnChange(w, x, y, finished);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!w) return false;
        if (TrySelectFactionFromWidget(w)) return true;
        if (TrySelectRewardPresetFromWidget(w)) return true;
        if (TrySelectRewardSearchFromWidget(w)) return true;
        if (TryHandleRewardContentWidget(w)) return true;
        if (TryHandleChainPointWidget(w)) return true;

        if (w == m_BtnLicenses)
        {
            ShowLicensesTab();
            return true;
        }
        if (w == m_BtnTerritories)
        {
            ShowTerritoriesTab();
            return true;
        }
        if (w == m_BtnSide)
        {
            CycleSide();
            return true;
        }
        if (w == m_BtnFactionTab)
        {
            ShowFactionTab();
            return true;
        }
        if (w == m_BtnMembersTab)
        {
            ShowMembersTab();
            if (s_Panel)
            {
                TFLFactionCommandDto refreshDto = new TFLFactionCommandDto();
                refreshDto.faction_id = s_Panel.selected_id;
                SendCommand(TFLFactionCommand.ADMIN_SELECT, refreshDto);
            }
            return true;
        }
        if (w == m_BtnRolesTab)
        {
            ShowRolesTab();
            return true;
        }
        if (w == m_BtnRanksTab)
        {
            ShowRanksTab();
            return true;
        }
        if (w == m_AdminBtnNewRank)
        {
            m_NewRank = true;
            m_SelectedRank = "";
            RebuildRanks();
            RefreshRankEditor();
            return true;
        }
        if (w == m_AdminBtnSaveRank)
        {
            SaveRankEditor();
            return true;
        }
        if (w == m_AdminBtnDeleteRank)
        {
            DeleteSelectedRank();
            return true;
        }
        if (w == m_RankAssignSelected)
        {
            AssignSelectedRank();
            return true;
        }
        if (w == m_RankCurRub) { m_RankCurrencyId = TFLFactionCurrency.RUBLE; RefreshRankCurrencyButtons(); return true; }
        if (w == m_RankCurUsd) { m_RankCurrencyId = TFLFactionCurrency.DOLLAR; RefreshRankCurrencyButtons(); return true; }
        if (w == m_RankCurEur) { m_RankCurrencyId = TFLFactionCurrency.EURO; RefreshRankCurrencyButtons(); return true; }
        if (w == m_RankTabCombat) { RefreshSalarySettings(); return true; }
        if (w == m_RankTabCivilian) { RefreshSalarySettings(); return true; }
        if (w == m_CivilianBenefitEnabled)
        {
            m_CivilianBenefitEditEnabled = 1 - m_CivilianBenefitEditEnabled;
            if (m_CivilianBenefitEditEnabled == 1) m_CivilianBenefitEnabled.SetText("[X] ПОСОБИЕ ВКЛЮЧЕНО");
            else m_CivilianBenefitEnabled.SetText("[ ] ПОСОБИЕ ВКЛЮЧЕНО");
            return true;
        }
        if (w == m_BenefitCurRub) { m_CivilianBenefitCurrencyId = TFLFactionCurrency.RUBLE; RefreshBenefitCurrencyButtons(); return true; }
        if (w == m_BenefitCurUsd) { m_CivilianBenefitCurrencyId = TFLFactionCurrency.DOLLAR; RefreshBenefitCurrencyButtons(); return true; }
        if (w == m_BenefitCurEur) { m_CivilianBenefitCurrencyId = TFLFactionCurrency.EURO; RefreshBenefitCurrencyButtons(); return true; }
        if (w == m_CivilianBenefitSave)
        {
            SaveSalarySettings();
            return true;
        }
        if (w == m_SalaryEnabled)
        {
            m_SalaryEditEnabled = 1 - m_SalaryEditEnabled;
            if (m_SalaryEditEnabled == 1) m_SalaryEnabled.SetText("[X] ВЫПЛАТЫ");
            else m_SalaryEnabled.SetText("[ ] ВЫПЛАТЫ");
            return true;
        }
        if (w == m_SalarySaveSettings)
        {
            SaveSalarySettings();
            return true;
        }

        if (w == m_LicActive)
        {
            if (m_LicEditActive == 1)
                m_LicEditActive = 0;
            else
                m_LicEditActive = 1;
            LicRefreshActiveButton();
            return true;
        }
        if (w == m_LicNew)
        {
            LicBeginNew();
            return true;
        }
        if (w == m_LicSave)
        {
            LicSaveLicense();
            return true;
        }
        if (w == m_LicDelete)
        {
            LicDeleteLicense();
            return true;
        }
        if (w == m_LicToggleFaction)
        {
            LicToggleIssuer();
            return true;
        }

        if (w == m_TerrMap)
        {
            TerrSelectNearestOnMap(m_TerrMap, x, y, false);
            return true;
        }
        if (w == m_ChainMap)
        {
            TerrSelectNearestOnMap(m_ChainMap, x, y, true);
            return true;
        }
        if (w == m_TerrCreate)
        {
            TerrCreateTerritory();
            return true;
        }
        if (w == m_TerrDelete)
        {
            TerrSendOnly(TFLFactionCommand.ADMIN_TERRITORY_DELETE);
            m_TerrSelectedId = "";
            m_TerrRewardSelected = -1;
            return true;
        }
        if (w == m_TerrSave)
        {
            TerrSaveTerritory();
            return true;
        }
        if (w == m_TerrDirectCapture)
        {
            TerrSendOnly(TFLFactionCommand.ADMIN_TERRITORY_TOGGLE_DIRECT);
            return true;
        }
        if (w == m_TerrSaveLimit)
        {
            if (m_TerrDailyLimit)
            {
                TFLFactionCommandDto limitDto = new TFLFactionCommandDto();
                limitDto.daily_capture_limit = m_TerrDailyLimit.GetText().ToInt();
                SendCommand(TFLFactionCommand.ADMIN_TERRITORY_SETTINGS_SAVE, limitDto);
            }
            return true;
        }
        for (int terrDayClick = 0; terrDayClick < m_TerrDays.Count(); terrDayClick++)
        {
            if (w == m_TerrDays.Get(terrDayClick))
            {
                if (m_TerrSelectedId != "")
                {
                    TFLFactionCommandDto dayDto = new TFLFactionCommandDto();
                    dayDto.territory_id = m_TerrSelectedId;
                    dayDto.day_index = terrDayClick + 1;
                    SendCommand(TFLFactionCommand.ADMIN_TERRITORY_TOGGLE_DAY, dayDto);
                }
                return true;
            }
        }
        if (w == m_TerrBtnRewardEditor) { RewardPresetOpen(); return true; }
        if (w == m_TerrBtnPickReward) { RewardPresetOpen(); return true; }
        if (w == m_TerrBtnChainEditor) { ChainOpen(); return true; }
        if (w == m_ModeBtnPoints) { TerrSetPointMode(); TerrRefresh(); return true; }
        if (w == m_ModeBtnChains) { ChainOpen(); return true; }
        if (w == m_ModeBtnRewards) { RewardPresetOpen(); return true; }
        if (w == m_RewardPresetClose || w == m_RewardPresetWindowClose) { RewardPresetClose(); return true; }
        if (w == m_RewardPresetNew)
        {
            m_SelectedRewardPresetId = "";
            if (m_RewardPresetId) m_RewardPresetId.SetText("reward_" + Math.RandomInt(1000, 9999).ToString());
            if (m_RewardPresetName) m_RewardPresetName.SetText("Новый пресет награды");
            ClearWidgets(m_RewardContentRows);
            return true;
        }
        if (w == m_RewardPresetSave)
        {
            if (m_SelectedRewardPresetId == "") RewardPresetCreate(); else RewardPresetSave();
            return true;
        }
        if (w == m_RewardPresetDelete) { RewardPresetDelete(); return true; }
        if (w == m_RewardPresetAssign) { RewardPresetAssign(); return true; }
        if (w == m_RewardItemSearchBtn) { RewardItemSearch(); return true; }
        if (w == m_RewardItemAdd) { RewardItemAddToPreset(); return true; }

        if (w == m_ChainClose || w == m_ChainWindowClose) { ChainCloseEditor(); return true; }
        if (w == m_ChainNew) { ChainNewEditor(); return true; }
        if (w == m_ChainSave)
        {
            if (ChainById(m_SelectedChainId)) ChainCreateOrSave(false); else ChainCreateOrSave(true);
            return true;
        }
        if (w == m_ChainDelete) { ChainDeleteSelected(); return true; }
        if (w == m_ChainSideA) { ChainCycleSide(true); return true; }
        if (w == m_ChainSideB) { ChainCycleSide(false); return true; }

        if (w == m_TerrRewardAdd)
        {
            if (m_TerrSelectedId != "" && m_TerrRewardClass && m_TerrRewardCount)
            {
                TFLFactionCommandDto rewardDto = new TFLFactionCommandDto();
                rewardDto.territory_id = m_TerrSelectedId;
                rewardDto.class_name = m_TerrRewardClass.GetText();
                rewardDto.reward_count = m_TerrRewardCount.GetText().ToInt();
                SendCommand(TFLFactionCommand.ADMIN_TERRITORY_REWARD_ADD, rewardDto);
            }
            return true;
        }
        if (w == m_TerrRewardRemove)
        {
            if (m_TerrSelectedId != "" && m_TerrRewardSelected >= 0)
            {
                TFLFactionCommandDto removeDto = new TFLFactionCommandDto();
                removeDto.territory_id = m_TerrSelectedId;
                removeDto.int_value = m_TerrRewardSelected;
                SendCommand(TFLFactionCommand.ADMIN_TERRITORY_REWARD_REMOVE, removeDto);
                m_TerrRewardSelected = -1;
            }
            return true;
        }
        if (w == m_BtnCreateFaction)
        {
            CreateFaction();
            return true;
        }
        if (w == m_BtnSaveName)
        {
            SaveFactionName();
            return true;
        }
        if (w == m_BtnSpawnHere)
        {
            SendSimple(TFLFactionCommand.ADMIN_SPAWN_HERE);
            return true;
        }
        if (w == m_BtnSpawnYawApply)
        {
            if (m_SpawnYawSlider)
            {
                TFLFactionCommandDto yawDto = new TFLFactionCommandDto();
                yawDto.int_value = Math.Round(m_SpawnYawSlider.GetCurrent());
                SendCommand(TFLFactionCommand.ADMIN_SPAWN_SET_YAW, yawDto);
            }
            return true;
        }
        if (w == m_BtnSpawnClearPoints)
        {
            SendSimple(TFLFactionCommand.ADMIN_CLEAR_SPAWN_POINTS);
            return true;
        }
        if (w == m_BtnLaptopHere)
        {
            SendSimple(TFLFactionCommand.ADMIN_LAPTOP_HERE);
            return true;
        }
        if (w == m_BtnLaptopYawApply)
        {
            if (m_LaptopYawSlider)
            {
                TFLFactionCommandDto laptopYawDto = new TFLFactionCommandDto();
                laptopYawDto.int_value = Math.Round(m_LaptopYawSlider.GetCurrent());
                SendCommand(TFLFactionCommand.ADMIN_LAPTOP_SET_YAW, laptopYawDto);
            }
            return true;
        }
        if (w == m_BtnChestHere)
        {
            SendSimple(TFLFactionCommand.ADMIN_ADD_CHEST_HERE);
            return true;
        }
        if (w == m_BtnPresetMe)
        {
            SendSimple(TFLFactionCommand.ADMIN_PRESET_ME);
            return true;
        }
        if (w == m_BtnPresetClear)
        {
            SendSimple(TFLFactionCommand.ADMIN_CLEAR_PRESET);
            return true;
        }
        if (w == m_BtnPresetClearInventory)
        {
            SendSimple(TFLFactionCommand.ADMIN_PRESET_TOGGLE_CLEAR);
            return true;
        }
        if (w == m_BtnPresetAdd)
        {
            PresetAdd();
            return true;
        }
        if (w == m_BtnPresetRemove)
        {
            PresetRemove();
            return true;
        }
        if (w == m_AdminBtnAddRolePrev)
        {
            CycleAddRole(-1);
            return true;
        }
        if (w == m_AdminBtnAddRoleNext)
        {
            CycleAddRole(1);
            return true;
        }
        if (w == m_AdminBtnAddSelected)
        {
            if (m_SelectedPerson != "")
                AddMemberByUid(m_SelectedPerson);
            return true;
        }
        if (w == m_AdminBtnAddUid)
        {
            if (m_AdminAddUidEdit)
                AddMemberByUid(m_AdminAddUidEdit.GetText());
            return true;
        }
        if (w == m_AdminBtnPromote)
        {
            MemberCommand(TFLFactionCommand.ADMIN_PROMOTE);
            return true;
        }
        if (w == m_AdminBtnDemote)
        {
            MemberCommand(TFLFactionCommand.ADMIN_DEMOTE);
            return true;
        }
        if (w == m_AdminBtnKick)
        {
            MemberCommand(TFLFactionCommand.ADMIN_KICK);
            return true;
        }
        if (w == m_AdminBtnNewRole)
        {
            m_NewRole = true;
            m_SelectedRole = "";
            RefreshRoleEditor();
            RebuildRoles();
            return true;
        }
        if (w == m_AdminBtnSaveRole)
        {
            SaveRole();
            return true;
        }

        if (w == m_AdminPermUseLaptop || w == m_AdminPermInvite)
        {
            TogglePermission(ButtonWidget.Cast(w));
            return true;
        }
        if (w == m_AdminPermKick || w == m_AdminPermPromote)
        {
            TogglePermission(ButtonWidget.Cast(w));
            return true;
        }
        if (w == m_AdminPermDemote || w == m_AdminPermManageRoles)
        {
            TogglePermission(ButtonWidget.Cast(w));
            return true;
        }
        if (w == m_AdminPermEditRights || w == m_AdminPermIssueLicenses)
        {
            TogglePermission(ButtonWidget.Cast(w));
            return true;
        }


        for (int licRowIndex = 0; licRowIndex < m_LicLicenseRows.Count(); licRowIndex++)
        {
            Widget licRow = m_LicLicenseRows.Get(licRowIndex);
            if (!licRow)
                continue;
            if (!WidgetInsideRow(w, licRow))
                continue;
            if (!s_Panel || licRowIndex >= s_Panel.licenses.Count())
                return true;
            TFLLicenseAdminDto clickedLicense = s_Panel.licenses.Get(licRowIndex);
            if (clickedLicense)
            {
                m_LicSelectedLicenseId = clickedLicense.id;
                m_LicSelectedFactionId = "";
                m_LicNewMode = false;
                LicRefresh();
            }
            return true;
        }

        for (int issuerRowIndex = 0; issuerRowIndex < m_LicIssuerRows.Count(); issuerRowIndex++)
        {
            Widget issuerRow = m_LicIssuerRows.Get(issuerRowIndex);
            if (!issuerRow)
                continue;
            if (!WidgetInsideRow(w, issuerRow))
                continue;
            if (!s_Panel || issuerRowIndex >= s_Panel.factions.Count())
                return true;
            TFLFactionSummaryDto clickedFaction = s_Panel.factions.Get(issuerRowIndex);
            if (clickedFaction)
            {
                m_LicSelectedFactionId = clickedFaction.id;
                LicRebuildIssuerRows();
            }
            return true;
        }

        for (int terrRowIndex = 0; terrRowIndex < m_TerritoryRows.Count(); terrRowIndex++)
        {
            Widget territoryRow = m_TerritoryRows.Get(terrRowIndex);
            if (!territoryRow)
                continue;
            if (!WidgetInsideRow(w, territoryRow))
                continue;
            if (!s_Panel || terrRowIndex >= s_Panel.territories.Count())
                return true;
            TFLTerritoryDto clickedTerritory = s_Panel.territories.Get(terrRowIndex);
            if (clickedTerritory)
            {
                m_TerrSelectedId = clickedTerritory.id;
                m_TerrHaveMapPoint = false;
                m_TerrRewardSelected = -1;
                TerrRefresh();
            }
            return true;
        }

        for (int rewardRowIndex = 0; rewardRowIndex < m_TerrRewardRows.Count(); rewardRowIndex++)
        {
            Widget rewardRow = m_TerrRewardRows.Get(rewardRowIndex);
            if (!rewardRow)
                continue;
            if (!WidgetInsideRow(w, rewardRow))
                continue;
            m_TerrRewardSelected = rewardRowIndex;
            TerrRebuildRewards(TerrSelected());
            return true;
        }

        for (int i = 0; i < m_FactionRows.Count(); i++)
        {
            Widget row = m_FactionRows.Get(i);
            if (!row)
                continue;
            if (!WidgetInsideRow(w, row))
                continue;
            if (i >= s_Panel.factions.Count())
                return true;
            TFLFactionSummaryDto faction = s_Panel.factions.Get(i);
            if (!faction) return true;
            SelectFactionById(faction.id);
            return true;
        }

        for (int m = 0; m < m_MemberRows.Count(); m++)
        {
            Widget memberRow = m_MemberRows.Get(m);
            if (!memberRow)
                continue;
            if (!WidgetInsideRow(w, memberRow))
                continue;
            if (m >= s_Panel.members.Count())
                return true;
            TFLFactionMemberDto member = s_Panel.members.Get(m);
            if (member)
                m_SelectedMember = member.uid;
            RebuildMembers();
            RefreshMember();
            return true;
        }

        for (int pidx = 0; pidx < m_PeopleRows.Count(); pidx++)
        {
            Widget personRow = m_PeopleRows.Get(pidx);
            if (!personRow)
                continue;
            if (!WidgetInsideRow(w, personRow))
                continue;
            if (pidx >= s_Panel.people.Count())
                return true;
            TFLFactionPassportPersonDto person = s_Panel.people.Get(pidx);
            if (person)
                m_SelectedPerson = person.uid;
            RebuildPeople();
            RefreshAddPanel();
            return true;
        }

        for (int r = 0; r < m_RoleRows.Count(); r++)
        {
            Widget roleRow = m_RoleRows.Get(r);
            if (!roleRow)
                continue;
            if (!WidgetInsideRow(w, roleRow))
                continue;
            if (r >= s_Panel.roles.Count())
                return true;
            TFLFactionRoleDto roleDto = s_Panel.roles.Get(r);
            if (roleDto)
            {
                m_NewRole = false;
                m_SelectedRole = roleDto.id;
            }
            RebuildRoles();
            RefreshRoleEditor();
            return true;
        }

        for (int rk = 0; rk < m_RankRows.Count(); rk++)
        {
            Widget rankRow = m_RankRows.Get(rk);
            if (!rankRow) continue;
            if (!WidgetInsideRow(w, rankRow)) continue;
            if (rk >= s_Panel.ranks.Count()) return true;
            TFLFactionRankDto rankDto = s_Panel.ranks.Get(rk);
            if (rankDto)
            {
                m_NewRank = false;
                m_SelectedRank = rankDto.id;
            }
            RebuildRanks();
            RefreshRankEditor();
            return true;
        }

        for (int p = 0; p < m_PresetRows.Count(); p++)
        {
            Widget presetRow = m_PresetRows.Get(p);
            if (!presetRow)
                continue;
            if (!WidgetInsideRow(w, presetRow))
                continue;
            if (p >= s_Panel.preset_items.Count())
                return true;
            m_SelectedPreset = s_Panel.preset_items.Get(p);
            if (m_PresetClassEdit)
                m_PresetClassEdit.SetText(m_SelectedPreset);
            RebuildPreset();
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
