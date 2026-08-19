class TFLFactionLaptopMenu extends UIScriptedMenu
{
    protected static TFLFactionLaptopMenu s_Menu;
    protected static ref TFLFactionLeaderPanelDto s_Panel;
    protected static ref TFLPassOfficeDto s_OfficeData;

    protected Widget m_Root;
    protected TextWidget m_TitleText;
    protected TextWidget m_OperatorText;
    protected ButtonWidget m_BtnClose;
    protected ButtonWidget m_BtnExit;
    protected ButtonWidget m_BtnMembersTab;
    protected ButtonWidget m_BtnRolesTab;
    protected ButtonWidget m_BtnTerritory;
    protected ButtonWidget m_BtnPassportOffice;
    protected Widget m_MembersPanel;
    protected Widget m_RolesPanel;
    protected Widget m_TerritoryPanel;
    protected Widget m_PassportOfficePanel;

    protected Widget m_MemberHost;
    protected ScrollWidget m_MemberScroll;
    protected TextWidget m_MemberName;
    protected TextWidget m_MemberRole;
    protected TextWidget m_MemberRank;
    protected TextWidget m_MemberUid;
    protected TextWidget m_MemberStatus;
    protected ButtonWidget m_BtnPromote;
    protected ButtonWidget m_BtnDemote;
    protected ButtonWidget m_BtnKick;

    protected Widget m_RoleHost;
    protected ScrollWidget m_RoleScroll;
    protected TextWidget m_RoleNameDisplay;
    protected Widget m_PermHost;
    protected ScrollWidget m_PermScroll;
    protected TextWidget m_RoleDisabledHint;

    protected ref array<Widget> m_MemberRows;
    protected ref array<Widget> m_RoleRows;
    protected ref array<Widget> m_PermRows;
    protected ref array<string> m_PermKeys;
    protected string m_SelectedMember;
    protected string m_SelectedRole;
    protected ref TFLFactionRolePermissions m_EditPermissions;

    protected TextListboxWidget m_OfficePeople;
    protected TextListboxWidget m_OfficeLicenses;
    protected TextWidget m_OfficeSelected;
    protected TextWidget m_OfficeSerial;
    protected MultilineTextWidget m_OfficeDescription;
    protected TextWidget m_OfficeStatus;
    protected EditBoxWidget m_OfficeNote;
    protected ButtonWidget m_OfficeGrant;
    protected ButtonWidget m_OfficeRevoke;
    protected ButtonWidget m_OfficeAddNote;
    protected ButtonWidget m_OfficeRefresh;
    protected string m_OfficeSelectedUid;
    protected string m_OfficeSelectedLicense;

    protected MapWidget m_LeaderTerrMap;
    protected Widget m_LeaderTerrCardHost;
    protected Widget m_LeaderTerrCard;
    protected Widget m_LeaderTerrRequestHost;
    protected Widget m_LeaderTerrRequestCard;
    protected TextWidget m_LeaderTerrName;
    protected TextWidget m_LeaderTerrOwner;
    protected TextWidget m_LeaderTerrStatusText;
    protected TextWidget m_LeaderTerrStatusReason;
    protected TextWidget m_LeaderTerrChainName;
    protected TextWidget m_LeaderTerrChainPosition;
    protected TextWidget m_LeaderTerrPrevious;
    protected TextWidget m_LeaderTerrNext;
    protected TextWidget m_LeaderTerrPlayers;
    protected TextWidget m_LeaderTerrPrepare;
    protected TextWidget m_LeaderTerrCapture;
    protected Widget m_LeaderTerrRewardHost;
    protected ScrollWidget m_LeaderTerrRewardScroll;
    protected ref array<Widget> m_LeaderTerrRewardRows;
    protected ref array<EntityAI> m_LeaderTerrPreviewItems;
    protected ButtonWidget m_LeaderTerrStart;
    protected ButtonWidget m_LeaderTerrAccept;
    protected ButtonWidget m_LeaderTerrDecline;
    protected ButtonWidget m_LeaderTerrClose;
    protected TextWidget m_LeaderTerrRequestFaction;
    protected TextWidget m_LeaderTerrRequestTerritory;
    protected int m_LeaderTerrActionCommand;
    protected string m_LeaderTerrSelectedId;
    protected string m_LeaderTerrRequestId;
    protected float m_LeaderTerrCardX;
    protected int m_LeaderTerrCardAnim;

    void TFLFactionLaptopMenu()
    {
        m_MemberRows = new array<Widget>();
        m_RoleRows = new array<Widget>();
        m_PermRows = new array<Widget>();
        m_PermKeys = new array<string>();
        m_SelectedMember = "";
        m_SelectedRole = "";
        m_EditPermissions = new TFLFactionRolePermissions();
        m_LeaderTerrSelectedId = "";
        m_LeaderTerrRequestId = "";
        m_LeaderTerrCardX = 1.02;
        m_LeaderTerrCardAnim = 0;
        m_LeaderTerrActionCommand = -1;
        m_LeaderTerrRewardRows = new array<Widget>();
        m_LeaderTerrPreviewItems = new array<EntityAI>();
        m_OfficeSelectedUid = "";
        m_OfficeSelectedLicense = "";
    }

    static void OnPanel(TFLFactionLeaderPanelDto panel)
    {
        if (!panel)
            return;
        s_Panel = panel;
        if (!s_Menu)
            s_Menu = TFLFactionLaptopMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(TFLFactionMenuIds.LAPTOP, null));
        else
        {
            s_Menu.RefreshAll();
            s_Menu.LeaderTerrAutoOpenUrgent();
        }
    }

    static bool CanReceiveOffice()
    {
        return s_Menu != null && s_Panel != null && s_Panel.can_issue_licenses == 1;
    }

    static void OnOffice(TFLPassOfficeDto data)
    {
        if (!data)
            return;
        s_OfficeData = data;
        if (s_Menu)
            s_Menu.OfficeRefresh();
    }

    static void SetOfficeStatus(string text)
    {
        if (s_Menu && s_Menu.m_OfficeStatus)
            s_Menu.m_OfficeStatus.SetText(text);
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_laptop_v2.layout");
        m_Root = layoutRoot;
        if (!layoutRoot)
        {
            TFLFactionLog.Error("Не загружен faction_laptop_v2.layout");
            return null;
        }

        m_TitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopTitleText"));
        m_OperatorText = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopFactionText"));
        m_BtnClose = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnClose"));
        m_BtnExit = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopBtnExit"));
        m_BtnMembersTab = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopTabMembers"));
        m_BtnRolesTab = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopTabRoles"));
        m_BtnTerritory = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopTabTerritory"));
        m_BtnPassportOffice = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopTabPassport"));
        m_MembersPanel = layoutRoot.FindAnyWidget("LaptopMembersPanel");
        m_RolesPanel = layoutRoot.FindAnyWidget("LaptopRolesPanel");
        m_TerritoryPanel = layoutRoot.FindAnyWidget("LaptopTerritoryPanel");
        m_PassportOfficePanel = layoutRoot.FindAnyWidget("LaptopPassportPanel");

        m_MemberHost = layoutRoot.FindAnyWidget("LaptopMemberHost");
        m_MemberScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("LaptopMemberScroll"));
        m_MemberName = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopMemberName"));
        m_MemberRole = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopMemberRole"));
        m_MemberRank = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopMemberRank"));
        m_MemberUid = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopMemberUid"));
        m_MemberStatus = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopMemberHint"));
        m_BtnPromote = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopBtnPromote"));
        m_BtnDemote = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopBtnDemote"));
        m_BtnKick = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LaptopBtnKick"));

        m_RoleHost = layoutRoot.FindAnyWidget("LaptopRoleHost");
        m_RoleScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("LaptopRoleScroll"));
        m_RoleNameDisplay = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopRoleName"));
        m_PermHost = layoutRoot.FindAnyWidget("LaptopPermHost");
        m_PermScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("LaptopPermScroll"));
        m_RoleDisabledHint = TextWidget.Cast(layoutRoot.FindAnyWidget("LaptopRoleDisabledHint"));

        m_OfficePeople = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("FOfficePeopleList"));
        m_OfficeLicenses = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("FOfficeLicenseList"));
        m_OfficeSelected = TextWidget.Cast(layoutRoot.FindAnyWidget("FOfficeSelectedPerson"));
        m_OfficeSerial = TextWidget.Cast(layoutRoot.FindAnyWidget("FOfficeSelectedSerial"));
        m_OfficeDescription = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("FOfficeLicenseDescription"));
        m_OfficeStatus = TextWidget.Cast(layoutRoot.FindAnyWidget("FOfficeStatus"));
        m_OfficeNote = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("FOfficeNoteEdit"));
        m_OfficeGrant = ButtonWidget.Cast(layoutRoot.FindAnyWidget("FOfficeBtnGrant"));
        m_OfficeRevoke = ButtonWidget.Cast(layoutRoot.FindAnyWidget("FOfficeBtnRevoke"));
        m_OfficeAddNote = ButtonWidget.Cast(layoutRoot.FindAnyWidget("FOfficeBtnNote"));
        m_OfficeRefresh = ButtonWidget.Cast(layoutRoot.FindAnyWidget("FOfficeBtnRefresh"));

        m_LeaderTerrMap = MapWidget.Cast(layoutRoot.FindAnyWidget("LaptopTerritoryMap"));
        m_LeaderTerrCardHost = layoutRoot.FindAnyWidget("LaptopCardHost");
        m_LeaderTerrRequestHost = layoutRoot.FindAnyWidget("LaptopRequestCardHost");
        if (m_LeaderTerrCardHost)
        {
            m_LeaderTerrCard = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryLeaderCard.layout", m_LeaderTerrCardHost);
            if (m_LeaderTerrCard)
            {
                m_LeaderTerrCard.Show(false);
                m_LeaderTerrName = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryName"));
                m_LeaderTerrOwner = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryOwner"));
                m_LeaderTerrStatusText = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryStatus"));
                m_LeaderTerrStatusReason = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryStatusReason"));
                m_LeaderTerrChainName = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryChainName"));
                m_LeaderTerrChainPosition = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryChainPosition"));
                m_LeaderTerrPrevious = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryPrevious"));
                m_LeaderTerrNext = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryNext"));
                m_LeaderTerrPlayers = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryPlayers"));
                m_LeaderTerrPrepare = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryPrepareTime"));
                m_LeaderTerrCapture = TextWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryCaptureTime"));
                m_LeaderTerrRewardHost = m_LeaderTerrCard.FindAnyWidget("TerritoryRewardList");
                m_LeaderTerrRewardScroll = ScrollWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryRewardScroll"));
                m_LeaderTerrStart = ButtonWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryCaptureButton"));
                m_LeaderTerrClose = ButtonWidget.Cast(m_LeaderTerrCard.FindAnyWidget("TerritoryCardClose"));
            }
        }
        if (m_LeaderTerrRequestHost)
        {
            m_LeaderTerrRequestCard = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryRequestCard.layout", m_LeaderTerrRequestHost);
            if (m_LeaderTerrRequestCard)
            {
                m_LeaderTerrRequestCard.Show(false);
                m_LeaderTerrRequestFaction = TextWidget.Cast(m_LeaderTerrRequestCard.FindAnyWidget("TerritoryRequestFaction"));
                m_LeaderTerrRequestTerritory = TextWidget.Cast(m_LeaderTerrRequestCard.FindAnyWidget("TerritoryRequestTerritory"));
                m_LeaderTerrAccept = ButtonWidget.Cast(m_LeaderTerrRequestCard.FindAnyWidget("TerritoryRequestAccept"));
                m_LeaderTerrDecline = ButtonWidget.Cast(m_LeaderTerrRequestCard.FindAnyWidget("TerritoryRequestDecline"));
            }
        }
        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        s_Menu = this;
        SetFocus(layoutRoot);
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);
        TFLPassHud.Hide(true);
        LeaderTerrInitMapView();
        ShowMembersTab();
        RefreshAll();
    }

    override void OnHide()
    {
        LeaderTerrClearRewards();
        super.OnHide();
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor(false);
        TFLPassHud.Hide(false);
        TFLPassClientRPC.Send(TFLPassRPC.OFFICE_CLOSE, new Param1<int>(0));
        s_OfficeData = null;
        s_Menu = null;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        LeaderTerrUpdateCardAnimation(timeslice);
        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
            Close();
    }

    protected void HideAllTabs()
    {
        if (m_MembersPanel)
            m_MembersPanel.Show(false);
        if (m_RolesPanel)
            m_RolesPanel.Show(false);
        if (m_TerritoryPanel)
            m_TerritoryPanel.Show(false);
        if (m_PassportOfficePanel)
            m_PassportOfficePanel.Show(false);
    }

    protected void ShowMembersTab()
    {
        if (m_TitleText) m_TitleText.SetText("СОСТАВ");
        HideAllTabs();
        if (m_MembersPanel)
            m_MembersPanel.Show(true);
    }

    protected void ShowRolesTab()
    {
        if (m_TitleText) m_TitleText.SetText("РОЛИ И ПРАВА");
        HideAllTabs();
        if (m_RolesPanel)
            m_RolesPanel.Show(true);
    }

    protected void ShowTerritoryTab()
    {
        if (m_TitleText) m_TitleText.SetText("ЗАХВАТ ТЕРРИТОРИЙ");
        if (!IsLocalLeader())
            return;
        HideAllTabs();
        if (m_TerritoryPanel)
            m_TerritoryPanel.Show(true);
        LeaderTerrInitMapView();
        LeaderTerrRefresh();
    }

    protected void ShowPassportOfficeTab()
    {
        if (m_TitleText) m_TitleText.SetText("ПАСПОРТНЫЙ СТОЛ");
        if (!s_Panel || s_Panel.can_issue_licenses != 1)
            return;
        HideAllTabs();
        if (m_PassportOfficePanel)
            m_PassportOfficePanel.Show(true);
        if (m_OfficeStatus)
            m_OfficeStatus.SetText("ОБНОВЛЕНИЕ // игроки в радиусе 3 м");
        TFLPassClientRPC.Send(TFLPassRPC.OFFICE_REFRESH, new Param1<int>(0));
        OfficeRefresh();
    }

    protected TFLPassOfficePersonDto OfficeFindPerson(string uid)
    {
        if (!s_OfficeData)
            return null;
        for (int i = 0; i < s_OfficeData.people.Count(); i++)
        {
            TFLPassOfficePersonDto person = s_OfficeData.people.Get(i);
            if (person && person.uid == uid)
                return person;
        }
        return null;
    }

    protected TFLLicenseDto OfficeFindLicense(string id)
    {
        if (!s_OfficeData)
            return null;
        for (int i = 0; i < s_OfficeData.licenses.Count(); i++)
        {
            TFLLicenseDto license = s_OfficeData.licenses.Get(i);
            if (license && license.id == id)
                return license;
        }
        return null;
    }

    protected void OfficeRefreshSelection()
    {
        TFLPassOfficePersonDto person = OfficeFindPerson(m_OfficeSelectedUid);
        if (m_OfficeSelected)
        {
            if (person)
                m_OfficeSelected.SetText(person.name);
            else
                m_OfficeSelected.SetText("ВЫБЕРИТЕ ЧЕЛОВЕКА В РАДИУСЕ 3 М");
        }
        if (m_OfficeSerial)
        {
            if (person)
                m_OfficeSerial.SetText("ПАСПОРТ: " + person.serial);
            else
                m_OfficeSerial.SetText("ПАСПОРТ: -");
        }
        TFLLicenseDto license = OfficeFindLicense(m_OfficeSelectedLicense);
        if (m_OfficeDescription)
        {
            if (license)
                m_OfficeDescription.SetText(license.description);
            else
                m_OfficeDescription.SetText("");
        }
    }

    protected void OfficeRefresh()
    {
        if (!s_OfficeData || !m_OfficePeople || !m_OfficeLicenses)
            return;
        if (m_OfficeSelectedUid != "" && !OfficeFindPerson(m_OfficeSelectedUid))
            m_OfficeSelectedUid = "";
        if (m_OfficeSelectedLicense != "" && !OfficeFindLicense(m_OfficeSelectedLicense))
            m_OfficeSelectedLicense = "";

        m_OfficePeople.ClearItems();
        for (int i = 0; i < s_OfficeData.people.Count(); i++)
        {
            TFLPassOfficePersonDto person = s_OfficeData.people.Get(i);
            if (person)
                m_OfficePeople.AddItem(person.name + " // " + person.serial, null, 0);
        }
        m_OfficeLicenses.ClearItems();
        TFLPassOfficePersonDto selected = OfficeFindPerson(m_OfficeSelectedUid);
        for (int l = 0; l < s_OfficeData.licenses.Count(); l++)
        {
            TFLLicenseDto license = s_OfficeData.licenses.Get(l);
            if (!license)
                continue;
            string mark = "[ ] ";
            if (selected && selected.license_ids.Find(license.id) != -1)
                mark = "[X] ";
            m_OfficeLicenses.AddItem(mark + license.name, null, 0);
        }
        if (m_OfficeStatus && s_OfficeData.people.Count() == 0)
            m_OfficeStatus.SetText("НЕТ ИГРОКОВ С ПАСПОРТОМ В РАДИУСЕ 3 М");
        OfficeRefreshSelection();
    }

    protected void OfficeSelectPerson()
    {
        if (!s_OfficeData || !m_OfficePeople)
            return;
        int row = m_OfficePeople.GetSelectedRow();
        if (row < 0 || row >= s_OfficeData.people.Count())
            return;
        TFLPassOfficePersonDto person = s_OfficeData.people.Get(row);
        if (!person)
            return;
        m_OfficeSelectedUid = person.uid;
        OfficeRefresh();
    }

    protected void OfficeSelectLicense()
    {
        if (!s_OfficeData || !m_OfficeLicenses)
            return;
        int row = m_OfficeLicenses.GetSelectedRow();
        if (row < 0 || row >= s_OfficeData.licenses.Count())
            return;
        TFLLicenseDto license = s_OfficeData.licenses.Get(row);
        if (!license)
            return;
        m_OfficeSelectedLicense = license.id;
        OfficeRefreshSelection();
    }

    protected void OfficeSendLicense(bool grant)
    {
        if (m_OfficeSelectedUid == "" || m_OfficeSelectedLicense == "")
        {
            SetOfficeStatus("Выберите человека и лицензию.");
            return;
        }
        int value = 0;
        if (grant)
            value = 1;
        TFLPassClientRPC.Send(TFLPassRPC.OFFICE_LICENSE, new Param3<string,string,int>(m_OfficeSelectedUid, m_OfficeSelectedLicense, value));
    }

    protected bool IsLocalLeader()
    {
        if (!s_Panel)
            return false;
        if (s_Panel.is_leader == 1)
            return true;

        Man man = GetGame().GetPlayer();
        if (!man || !man.GetIdentity())
            return false;
        string uid = man.GetIdentity().GetPlainId();
        for (int i = 0; i < s_Panel.members.Count(); i++)
        {
            TFLFactionMemberDto member = s_Panel.members.Get(i);
            if (member && member.uid == uid && member.role_id == "leader")
                return true;
        }
        return false;
    }

    void RefreshAll()
    {
        if (!s_Panel)
            return;

        if (m_BtnTerritory)
        {
            bool territoryVisible = IsLocalLeader();
            m_BtnTerritory.Show(territoryVisible);
        }
        if (m_BtnPassportOffice)
            m_BtnPassportOffice.Show(s_Panel.can_issue_licenses == 1);
        if (m_OperatorText)
            m_OperatorText.SetText(s_Panel.faction_name + " // ЛИДЕР");
        RebuildMembers();
        RebuildRoles();
        RefreshSelectedMember();
        RefreshRoleEditor();
        LeaderTerrRefresh();
        OfficeRefresh();
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

    protected void BindDynamicRow(Widget row)
    {
        if (!row) return;
        // UIScriptedMenu receives events from widgets in its menu tree automatically.
        // UIScriptedMenu is not a ScriptedWidgetEventHandler; explicit handler registration is invalid here.
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

    protected void RebuildMembers()
    {
        if (!m_MemberHost)
            return;
        ClearWidgets(m_MemberRows);
        int count = s_Panel.members.Count();
        float content = count * 0.085;
        if (content < 1.0)
            content = 1.0;
        m_MemberHost.SetSize(1, content);
        float step = 0.085 / content;

        for (int i = 0; i < count; i++)
        {
            TFLFactionMemberDto member = s_Panel.members.Get(i);
            if (!member)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_member_row.layout", m_MemberHost);
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
                string line = member.role_name;
                if (member.rank_name != "") line = member.rank_name + " // " + member.role_name;
                role.SetText(line);
            }
            if (mark)
                mark.Show(member.uid == m_SelectedMember);
        }
        if (m_MemberScroll)
            m_MemberScroll.VScrollToPos01(0);
    }

    protected void RebuildRoles()
    {
        if (!m_RoleHost)
            return;
        ClearWidgets(m_RoleRows);
        int count = s_Panel.roles.Count();
        float content = count * 0.085;
        if (content < 1.0)
            content = 1.0;
        m_RoleHost.SetSize(1, content);
        float step = 0.085 / content;

        for (int i = 0; i < count; i++)
        {
            TFLFactionRoleDto roleDto = s_Panel.roles.Get(i);
            if (!roleDto)
                continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/faction_role_row.layout", m_RoleHost);
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
        if (m_RoleScroll)
            m_RoleScroll.VScrollToPos01(0);
    }

    protected TFLFactionMemberDto FindMember(string uid)
    {
        if (!s_Panel)
            return null;
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

    protected TFLFactionRoleDto FindRole(string id)
    {
        if (!s_Panel)
            return null;
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

    protected void RefreshSelectedMember()
    {
        TFLFactionMemberDto member = FindMember(m_SelectedMember);
        if (!member)
        {
            m_SelectedMember = "";
            if (m_MemberName)
                m_MemberName.SetText("-");
            if (m_MemberRole)
                m_MemberRole.SetText("-");
            if (m_MemberRank)
                m_MemberRank.SetText("-");
            if (m_MemberUid)
                m_MemberUid.SetText("UID: -");
            return;
        }
        if (m_MemberName)
            m_MemberName.SetText(member.name);
        if (m_MemberRole) m_MemberRole.SetText(member.role_name);
        if (m_MemberRank)
        {
            string rankText = member.rank_name;
            if (rankText == "") rankText = "-";
            if (member.rank_name != "") rankText = rankText + " // " + member.salary.ToString() + " " + TFLFactionCurrency.Symbol(member.currency_id) + "/ч";
            m_MemberRank.SetText(rankText);
        }
        if (m_MemberUid)
            m_MemberUid.SetText("UID: " + member.uid);
    }

    // Role creation/renaming stays an Admin-only function (VPP TFL Faction Control).
    // The redesigned laptop Roles tab only lets the leader view a role and flip its
    // permission switches, so the editor here works off the already-selected role's
    // own id/name/level rather than free-text edit boxes.
    protected void RefreshRoleEditor()
    {
        TFLFactionRoleDto role = FindRole(m_SelectedRole);
        if (!role)
        {
            if (m_RoleNameDisplay)
                m_RoleNameDisplay.SetText("-");
            m_EditPermissions = new TFLFactionRolePermissions();
            RebuildPermissionRows();
            return;
        }
        if (m_RoleNameDisplay)
            m_RoleNameDisplay.SetText(role.name);
        m_EditPermissions = new TFLFactionRolePermissions();
        if (role.permissions)
            m_EditPermissions = role.permissions.Copy();
        RebuildPermissionRows();
    }

    protected string PermissionLabel(string key)
    {
        if (key == "use_laptop") return "НОУТБУК";
        if (key == "invite") return "ПРИГЛАШАТЬ";
        if (key == "kick") return "ИСКЛЮЧАТЬ";
        if (key == "promote") return "ПОВЫШАТЬ";
        if (key == "demote") return "ПОНИЖАТЬ";
        if (key == "manage_roles") return "СОЗДАВАТЬ РОЛИ";
        if (key == "edit_role_permissions") return "ПРАВА РОЛЕЙ";
        if (key == "issue_licenses") return "ЛИЦЕНЗИИ";
        return key;
    }

    protected int PermissionValue(string key)
    {
        if (!m_EditPermissions)
            return 0;
        if (key == "use_laptop") return m_EditPermissions.use_laptop;
        if (key == "invite") return m_EditPermissions.invite;
        if (key == "kick") return m_EditPermissions.kick;
        if (key == "promote") return m_EditPermissions.promote;
        if (key == "demote") return m_EditPermissions.demote;
        if (key == "manage_roles") return m_EditPermissions.manage_roles;
        if (key == "edit_role_permissions") return m_EditPermissions.edit_role_permissions;
        if (key == "issue_licenses") return m_EditPermissions.issue_licenses;
        return 0;
    }

    protected void TogglePermissionKey(string key)
    {
        if (!m_EditPermissions)
            m_EditPermissions = new TFLFactionRolePermissions();
        if (key == "use_laptop") m_EditPermissions.use_laptop = 1 - m_EditPermissions.use_laptop;
        if (key == "invite") m_EditPermissions.invite = 1 - m_EditPermissions.invite;
        if (key == "kick") m_EditPermissions.kick = 1 - m_EditPermissions.kick;
        if (key == "promote") m_EditPermissions.promote = 1 - m_EditPermissions.promote;
        if (key == "demote") m_EditPermissions.demote = 1 - m_EditPermissions.demote;
        if (key == "manage_roles") m_EditPermissions.manage_roles = 1 - m_EditPermissions.manage_roles;
        if (key == "edit_role_permissions") m_EditPermissions.edit_role_permissions = 1 - m_EditPermissions.edit_role_permissions;
        if (key == "issue_licenses") m_EditPermissions.issue_licenses = 1 - m_EditPermissions.issue_licenses;
    }

    protected void RebuildPermissionRows()
    {
        if (!m_PermHost)
            return;
        ClearWidgets(m_PermRows);
        m_PermKeys.Clear();
        m_PermKeys.Insert("use_laptop");
        m_PermKeys.Insert("invite");
        m_PermKeys.Insert("kick");
        m_PermKeys.Insert("promote");
        m_PermKeys.Insert("demote");
        m_PermKeys.Insert("manage_roles");
        m_PermKeys.Insert("edit_role_permissions");
        m_PermKeys.Insert("issue_licenses");

        int count = m_PermKeys.Count();
        float content = count * 0.095;
        if (content < 1.0)
            content = 1.0;
        m_PermHost.SetSize(1, content);
        float step = 0.095 / content;

        for (int i = 0; i < count; i++)
        {
            string key = m_PermKeys.Get(i);
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_LaptopPermRow.layout", m_PermHost);
            if (!row)
                continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_PermRows.Insert(row);
            BindDynamicRow(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RowName"));
            TextWidget state = TextWidget.Cast(row.FindAnyWidget("RowLevel"));
            Widget mark = row.FindAnyWidget("RowMark");
            if (name)
                name.SetText(PermissionLabel(key));
            if (state)
                state.SetText(TFLFactionUiText.OnOff(PermissionValue(key)));
            if (mark)
                mark.Show(PermissionValue(key) == 1);
        }
        if (m_PermScroll)
            m_PermScroll.VScrollToPos01(0);

        bool canEdit = s_Panel && s_Panel.can_edit_role_permissions == 1;
        if (m_RoleDisabledHint)
            m_RoleDisabledHint.Show(!canEdit);
    }

    // Applied immediately on click: the redesigned panel has no separate Save button,
    // so a toggle is the save action. Same server command and gating as before.
    protected void TogglePermission(string key)
    {
        if (!s_Panel)
            return;
        if (s_Panel.can_edit_role_permissions != 1)
            return;
        TFLFactionRoleDto role = FindRole(m_SelectedRole);
        if (!role)
            return;

        TogglePermissionKey(key);
        RebuildPermissionRows();

        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.role_id = role.id;
        dto.role_name = role.name;
        dto.role_level = role.level;
        dto.permissions = m_EditPermissions.Copy();
        TFLFactionClientRPC.Send(TFLFactionCommand.SAVE_ROLE, dto);
    }

    protected void MemberCommand(int command)
    {
        if (m_SelectedMember == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.target_uid = m_SelectedMember;
        TFLFactionClientRPC.Send(command, dto);
    }


    protected string LeaderTerrShortTime(int seconds)
    {
        if (seconds < 0) seconds = 0;
        int minutes = seconds / 60;
        if (minutes > 0) return minutes.ToString() + " мин";
        return seconds.ToString() + " сек";
    }

    protected void LeaderTerrPositionWidget(Widget widget, TFLTerritoryDto territory, float width, float height)
    {
        if (!territory || !widget || !m_LeaderTerrMap || !m_TerritoryPanel) return;
        vector screen = m_LeaderTerrMap.MapToScreen(Vector(territory.x, territory.y, territory.z));
        float panelX, panelY, panelW, panelH;
        float mapX, mapY, mapW, mapH;
        m_TerritoryPanel.GetScreenPos(panelX, panelY);
        m_TerritoryPanel.GetScreenSize(panelW, panelH);
        m_LeaderTerrMap.GetScreenPos(mapX, mapY);
        m_LeaderTerrMap.GetScreenSize(mapW, mapH);
        if (panelW <= 0 || panelH <= 0 || mapW <= 0 || mapH <= 0) return;
        float localX = (screen[0] - panelX) / panelW;
        float localY = (screen[1] - panelY) / panelH;
        float markerMapX = (screen[0] - mapX) / mapW;
        float cardX;
        if (markerMapX > 0.5) cardX = localX - width - 0.018;
        else cardX = localX + 0.018;
        float cardY = localY - height * 0.5;
        if (cardX < 0.01) cardX = 0.01;
        if (cardX > 0.99 - width) cardX = 0.99 - width;
        if (cardY < 0.01) cardY = 0.01;
        if (cardY > 0.99 - height) cardY = 0.99 - height;
        widget.SetPos(cardX, cardY);
    }

    protected void LeaderTerrPositionCard(TFLTerritoryDto territory)
    {
        // Карточка живёт фиксированной колонкой справа (позиция задана в
        // раскладке), а не всплывает у маркера: у маркера она загораживала
        // карту и упиралась в края экрана.
    }

    protected void LeaderTerrOpenCard()
    {
        if (!m_LeaderTerrCard) return;
        // The new layout keeps LaptopCardHost hidden by default (empty state); the
        // host has to be shown along with the card it hosts, or a visible child
        // under a hidden parent still renders as nothing.
        if (m_LeaderTerrCardHost) m_LeaderTerrCardHost.Show(true);
        m_LeaderTerrCardAnim = 0;
        m_LeaderTerrCard.SetAlpha(1.0);
        m_LeaderTerrCard.Show(true);
        TFLTerritoryDto territory = LeaderTerrSelected();
        if (territory) LeaderTerrPositionCard(territory);
    }

    protected void LeaderTerrCloseCard()
    {
        if (!m_LeaderTerrCard) return;
        m_LeaderTerrCardAnim = 0;
        m_LeaderTerrCard.Show(false);
        if (m_LeaderTerrCardHost) m_LeaderTerrCardHost.Show(false);
    }

    protected void LeaderTerrUpdateCardAnimation(float timeslice)
    {
    }

    protected void LeaderTerrAutoOpenUrgent()
    {
        if (!s_Panel) return;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.can_accept_ally == 1)
            {
                m_LeaderTerrSelectedId = territory.id;
                ShowTerritoryTab();
                LeaderTerrRefreshMarks();
                LeaderTerrRefreshCard();
                LeaderTerrOpenCard();
                return;
            }
        }
    }

    protected void LeaderTerrInitMapView()
    {
        if (!m_LeaderTerrMap) return;
        string worldName = GetGame().GetWorldName();
        vector center = GetGame().ConfigGetVector("CfgWorlds " + worldName + " centerPosition");
        if (center[0] == 0 && center[2] == 0)
        {
            int worldSize = GetGame().GetWorld().GetWorldSize();
            center = Vector(worldSize * 0.5, 0, worldSize * 0.5);
        }
        m_LeaderTerrMap.SetScale(0.33);
        m_LeaderTerrMap.SetMapPos(center);
    }

    protected TFLTerritoryDto LeaderTerrSelected()
    {
        if (!s_Panel) return null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.id == m_LeaderTerrSelectedId) return territory;
        }
        return null;
    }

    protected string LeaderTerrTimeText(int seconds)
    {
        if (seconds < 0) seconds = 0;
        int min = seconds / 60;
        int sec = seconds % 60;
        return TFLPassText.Pad2(min) + ":" + TFLPassText.Pad2(sec);
    }

    protected string LeaderTerrFactionNames(array<string> names)
    {
        if (!names || names.Count() == 0) return "-";
        string result = "";
        for (int i = 0; i < names.Count(); i++)
        {
            if (i > 0) result = result + ", ";
            result = result + names.Get(i);
        }
        return result;
    }

    protected TFLTerritoryDto LeaderTerrByChainIndex(string chainId, int chainIndex)
    {
        if (!s_Panel || chainId == "") return null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.chain_id == chainId && territory.chain_index == chainIndex) return territory;
        }
        return null;
    }

    protected void LeaderTerrAddChainSegment(TFLTerritoryDto fromTerr, TFLTerritoryDto toTerr, int color)
    {
        if (!m_LeaderTerrMap || !fromTerr || !toTerr) return;
        int pieces = 10;
        for (int i = 1; i < pieces; i++)
        {
            float k = (i * 1.0) / pieces;
            vector dot = Vector(fromTerr.x + (toTerr.x - fromTerr.x) * k, fromTerr.y + (toTerr.y - fromTerr.y) * k, fromTerr.z + (toTerr.z - fromTerr.z) * k);
            m_LeaderTerrMap.AddUserMark(dot, "", color, "TFL_Pasport/gui/textures/pass_white.edds");
        }
    }

    protected void LeaderTerrRefreshMarks()
    {
        if (!m_LeaderTerrMap || !s_Panel) return;
        m_LeaderTerrMap.ClearUserMarks();
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory) continue;
            int color = ARGB(255, 190, 190, 150);
            if (territory.status == "ПОДГОТОВКА") color = ARGB(255, 240, 180, 60);
            else if (territory.status == "ЗАХВАТ" || territory.status == "ОСПАРИВАЕТСЯ") color = ARGB(255, 220, 80, 70);
            else if (territory.owner_side_id == s_Panel.side_id) color = ARGB(255, 90, 190, 90);
            string markerText = territory.name + " // " + territory.status;
            if (territory.chain_id != "") markerText = territory.chain_index.ToString() + "/" + territory.chain_total.ToString() + "  " + markerText;
            m_LeaderTerrMap.AddUserMark(Vector(territory.x, territory.y, territory.z), markerText, color, "TFL_Pasport/gui/textures/seals/pass_seal_rad.edds");
        }
        TFLTerritoryDto selected = LeaderTerrSelected();
        if (selected && selected.chain_id != "" && selected.chain_total > 1)
        {
            int lineColor = ARGB(210, 127, 145, 82);
            for (int c = 1; c < selected.chain_total; c++)
            {
                TFLTerritoryDto fromTerr = LeaderTerrByChainIndex(selected.chain_id, c);
                TFLTerritoryDto toTerr = LeaderTerrByChainIndex(selected.chain_id, c + 1);
                if (!fromTerr || !toTerr) continue;
                LeaderTerrAddChainSegment(fromTerr, toTerr, lineColor);
                vector mid = Vector((fromTerr.x + toTerr.x) * 0.5, (fromTerr.y + toTerr.y) * 0.5, (fromTerr.z + toTerr.z) * 0.5);
                m_LeaderTerrMap.AddUserMark(mid, "→", lineColor, "TFL_Pasport/gui/textures/seals/pass_seal_rad.edds");
            }
        }
    }

    protected string LeaderTerrConfigName(string className)
    {
        if (className == "") return "";
        string cfgPath = "CfgVehicles " + className + " displayName";
        if (GetGame().ConfigIsExisting(cfgPath))
        {
            string displayName;
            GetGame().ConfigGetText(cfgPath, displayName);
            if (displayName != "") return displayName;
        }
        return className;
    }

    protected void LeaderTerrClearRewards()
    {
        if (!m_LeaderTerrRewardRows) return;
        for (int i = 0; i < m_LeaderTerrRewardRows.Count(); i++)
        {
            Widget row = m_LeaderTerrRewardRows.Get(i);
            if (row) row.Unlink();
        }
        m_LeaderTerrRewardRows.Clear();
        if (m_LeaderTerrPreviewItems) m_LeaderTerrPreviewItems.Clear();
    }

    protected void LeaderTerrBuildRewards(TFLTerritoryDto territory)
    {
        LeaderTerrClearRewards();
        if (!m_LeaderTerrRewardHost || !territory || !territory.rewards) return;
        int count = territory.rewards.Count();
        int shown = count;
        if (shown > 4) shown = 4;
        float content = shown * 0.30;
        if (content < 1.0) content = 1.0;
        m_LeaderTerrRewardHost.SetSize(1, content);
        float step = 0.30 / content;
        for (int i = 0; i < shown; i++)
        {
            TFLTerritoryRewardDto reward = territory.rewards.Get(i);
            if (!reward) continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryLeaderRewardRow.layout", m_LeaderTerrRewardHost);
            if (!row) continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_LeaderTerrRewardRows.Insert(row);
            TextWidget nameWidget = TextWidget.Cast(row.FindAnyWidget("RewardName"));
            TextWidget qtyWidget = TextWidget.Cast(row.FindAnyWidget("RewardQty"));
            TextWidget classWidget = TextWidget.Cast(row.FindAnyWidget("RewardClass"));
            if (nameWidget) nameWidget.SetText(LeaderTerrConfigName(reward.class_name));
            if (qtyWidget) qtyWidget.SetText("x" + reward.count.ToString());
            if (classWidget) classWidget.SetText(reward.class_name);
        }
        if (m_LeaderTerrRewardScroll) m_LeaderTerrRewardScroll.VScrollToPos01(0);
    }

    protected void LeaderTerrRefreshAction(TFLTerritoryDto territory)
    {
        m_LeaderTerrActionCommand = -1;
        if (!m_LeaderTerrStart || !territory) return;
        if (territory.can_start == 1)
        {
            m_LeaderTerrActionCommand = TFLFactionCommand.TERRITORY_START;
            m_LeaderTerrStart.SetText("НАЧАТЬ ЗАХВАТ");
            m_LeaderTerrStart.SetColor(ARGB(255, 41, 56, 26));
        }
        else if (territory.can_join_attack == 1)
        {
            m_LeaderTerrActionCommand = TFLFactionCommand.TERRITORY_JOIN_ATTACK;
            m_LeaderTerrStart.SetText("ПРИСОЕДИНИТЬСЯ К АТАКЕ");
            m_LeaderTerrStart.SetColor(ARGB(255, 41, 56, 26));
        }
        else if (territory.can_join_defense == 1)
        {
            m_LeaderTerrActionCommand = TFLFactionCommand.TERRITORY_JOIN_DEFENSE;
            m_LeaderTerrStart.SetText("ПРИСОЕДИНИТЬСЯ К ОБОРОНЕ");
            m_LeaderTerrStart.SetColor(ARGB(255, 41, 56, 26));
        }
        else
        {
            m_LeaderTerrStart.SetText("ЗАХВАТ НЕДОСТУПЕН");
            m_LeaderTerrStart.SetColor(ARGB(255, 31, 31, 28));
        }
    }

    protected void LeaderTerrRefreshRequest()
    {
        if (!m_LeaderTerrRequestCard || !s_Panel) return;
        TFLTerritoryDto requestTerritory = null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.can_accept_ally == 1)
            {
                requestTerritory = territory;
                break;
            }
        }
        if (!requestTerritory)
        {
            m_LeaderTerrRequestId = "";
            m_LeaderTerrRequestCard.Show(false);
            if (m_LeaderTerrRequestHost) m_LeaderTerrRequestHost.Show(false);
            return;
        }
        m_LeaderTerrRequestId = requestTerritory.id;
        if (m_LeaderTerrRequestFaction) m_LeaderTerrRequestFaction.SetText(requestTerritory.pending_ally_faction_name);
        if (m_LeaderTerrRequestTerritory) m_LeaderTerrRequestTerritory.SetText(requestTerritory.name);
        if (m_LeaderTerrRequestHost) m_LeaderTerrRequestHost.Show(true);
        m_LeaderTerrRequestCard.Show(true);
    }

    protected void LeaderTerrRefreshCard()
    {
        TFLTerritoryDto territory = LeaderTerrSelected();
        if (!territory)
        {
            if (m_LeaderTerrCard) m_LeaderTerrCard.Show(false);
            LeaderTerrClearRewards();
            m_LeaderTerrActionCommand = -1;
            LeaderTerrRefreshRequest();
            return;
        }
        if (m_LeaderTerrCard && !m_LeaderTerrCard.IsVisible()) LeaderTerrOpenCard();
        LeaderTerrPositionCard(territory);
        if (m_LeaderTerrName) m_LeaderTerrName.SetText(territory.name);
        if (m_LeaderTerrOwner)
        {
            string owner = territory.owner_side_name;
            if (owner == "") owner = "НЕЙТРАЛЬНАЯ";
            m_LeaderTerrOwner.SetText(owner);
        }
        if (m_LeaderTerrStatusText) m_LeaderTerrStatusText.SetText(territory.status);
        if (m_LeaderTerrStatusReason)
        {
            string reason = "";
            if (territory.cooldown_left > 0) reason = "Кулдаун: " + LeaderTerrShortTime(territory.cooldown_left);
            else if (territory.chain_prev_name != "" && territory.can_start == 0 && territory.status != "ЗАХВАТ") reason = "Нужно захватить: " + territory.chain_prev_name;
            m_LeaderTerrStatusReason.SetText(reason);
        }
        if (m_LeaderTerrChainName)
        {
            string chainName = territory.chain_name;
            if (chainName == "") chainName = "БЕЗ ЦЕПОЧКИ";
            m_LeaderTerrChainName.SetText(chainName);
        }
        if (m_LeaderTerrChainPosition)
        {
            if (territory.chain_id != "") m_LeaderTerrChainPosition.SetText(territory.chain_index.ToString() + " / " + territory.chain_total.ToString());
            else m_LeaderTerrChainPosition.SetText("-");
        }
        if (m_LeaderTerrPrevious)
        {
            string prev = territory.chain_prev_name;
            if (prev == "") prev = "-";
            m_LeaderTerrPrevious.SetText(prev);
        }
        if (m_LeaderTerrNext)
        {
            string next = territory.chain_next_name;
            if (next == "") next = "-";
            m_LeaderTerrNext.SetText(next);
        }
        if (m_LeaderTerrPlayers) m_LeaderTerrPlayers.SetText(territory.min_players.ToString());
        if (m_LeaderTerrPrepare) m_LeaderTerrPrepare.SetText(LeaderTerrShortTime(territory.preparation_seconds));
        if (m_LeaderTerrCapture) m_LeaderTerrCapture.SetText(LeaderTerrShortTime(territory.capture_seconds));
        LeaderTerrBuildRewards(territory);
        LeaderTerrRefreshAction(territory);
        LeaderTerrRefreshRequest();
    }

    protected void LeaderTerrRefresh()
    {
        if (!s_Panel || !layoutRoot)
            return;
        if (!m_LeaderTerrMap)
            return;
        LeaderTerrRefreshMarks();
        LeaderTerrRefreshCard();
        LeaderTerrRefreshRequest();
    }

    protected void LeaderTerrSelectNearest(int x, int y)
    {
        if (!m_LeaderTerrMap || !s_Panel)
            return;
        float best = 6400.0;
        string bestId = "";
        float mapX, mapY;
        m_LeaderTerrMap.GetScreenPos(mapX, mapY);
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory) continue;
            vector screen = m_LeaderTerrMap.MapToScreen(Vector(territory.x, territory.y, territory.z));
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
            m_LeaderTerrSelectedId = bestId;
            LeaderTerrRefreshMarks();
            LeaderTerrRefreshCard();
            LeaderTerrOpenCard();
        }
    }

    protected void LeaderTerrSend(int command)
    {
        if (m_LeaderTerrSelectedId == "")
            return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_LeaderTerrSelectedId;
        TFLFactionClientRPC.Send(command, dto);
    }

    protected void LeaderTerrSendRequest(int command)
    {
        if (m_LeaderTerrRequestId == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_LeaderTerrRequestId;
        TFLFactionClientRPC.Send(command, dto);
    }

    override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
    {
        super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
        if (w == m_OfficePeople)
        {
            OfficeSelectPerson();
            return true;
        }
        if (w == m_OfficeLicenses)
        {
            OfficeSelectLicense();
            return true;
        }
        return false;
    }

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
    {
        super.OnMouseButtonDown(w, x, y, button);
        if (w == m_LeaderTerrMap && button == MouseState.LEFT)
        {
            LeaderTerrSelectNearest(x, y);
            return true;
        }
        return false;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);
        if (!w)
            return false;

        if (w == m_BtnClose || w == m_BtnExit)
        {
            Close();
            return true;
        }
        if (w == m_BtnTerritory)
        {
            ShowTerritoryTab();
            return true;
        }
        if (w == m_BtnPassportOffice)
        {
            ShowPassportOfficeTab();
            return true;
        }
        if (w == m_BtnMembersTab)
        {
            ShowMembersTab();
            return true;
        }
        if (w == m_BtnRolesTab)
        {
            ShowRolesTab();
            return true;
        }

        if (w == m_OfficePeople)
        {
            OfficeSelectPerson();
            return true;
        }
        if (w == m_OfficeLicenses)
        {
            OfficeSelectLicense();
            return true;
        }
        if (w == m_OfficeGrant)
        {
            OfficeSendLicense(true);
            return true;
        }
        if (w == m_OfficeRevoke)
        {
            OfficeSendLicense(false);
            return true;
        }
        if (w == m_OfficeAddNote)
        {
            if (m_OfficeSelectedUid == "")
            {
                SetOfficeStatus("Выберите человека.");
                return true;
            }
            string noteText = "";
            if (m_OfficeNote)
                noteText = m_OfficeNote.GetText();
            TFLPassClientRPC.Send(TFLPassRPC.OFFICE_NOTE, new Param2<string,string>(m_OfficeSelectedUid, noteText));
            if (m_OfficeNote)
                m_OfficeNote.SetText("");
            return true;
        }
        if (w == m_OfficeRefresh)
        {
            TFLPassClientRPC.Send(TFLPassRPC.OFFICE_REFRESH, new Param1<int>(0));
            return true;
        }

        if (w == m_LeaderTerrMap)
        {
            LeaderTerrSelectNearest(x, y);
            return true;
        }
        if (w == m_LeaderTerrClose)
        {
            m_LeaderTerrSelectedId = "";
            LeaderTerrClearRewards();
            LeaderTerrCloseCard();
            return true;
        }
        if (w == m_LeaderTerrStart)
        {
            if (m_LeaderTerrActionCommand >= 0) LeaderTerrSend(m_LeaderTerrActionCommand);
            return true;
        }
        if (w == m_LeaderTerrAccept)
        {
            LeaderTerrSendRequest(TFLFactionCommand.TERRITORY_ACCEPT_ALLY);
            if (m_LeaderTerrRequestCard) m_LeaderTerrRequestCard.Show(false);
            return true;
        }
        if (w == m_LeaderTerrDecline)
        {
            LeaderTerrSendRequest(TFLFactionCommand.TERRITORY_DECLINE_ALLY);
            if (m_LeaderTerrRequestCard) m_LeaderTerrRequestCard.Show(false);
            return true;
        }
        if (w == m_BtnPromote)
        {
            MemberCommand(TFLFactionCommand.PROMOTE_MEMBER);
            return true;
        }
        if (w == m_BtnDemote)
        {
            MemberCommand(TFLFactionCommand.DEMOTE_MEMBER);
            return true;
        }
        if (w == m_BtnKick)
        {
            MemberCommand(TFLFactionCommand.KICK_MEMBER);
            return true;
        }
        for (int i = 0; i < m_MemberRows.Count(); i++)
        {
            Widget memberRow = m_MemberRows.Get(i);
            if (!memberRow)
                continue;
            if (!WidgetInsideRow(w, memberRow))
                continue;
            if (i >= s_Panel.members.Count())
                return true;
            TFLFactionMemberDto member = s_Panel.members.Get(i);
            if (member)
                m_SelectedMember = member.uid;
            RebuildMembers();
            RefreshSelectedMember();
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
                m_SelectedRole = roleDto.id;
            RebuildRoles();
            RefreshRoleEditor();
            return true;
        }

        for (int p = 0; p < m_PermRows.Count(); p++)
        {
            Widget permRow = m_PermRows.Get(p);
            if (!permRow)
                continue;
            if (!WidgetInsideRow(w, permRow))
                continue;
            if (p >= m_PermKeys.Count())
                return true;
            TogglePermission(m_PermKeys.Get(p));
            return true;
        }
        return false;
    }
}
