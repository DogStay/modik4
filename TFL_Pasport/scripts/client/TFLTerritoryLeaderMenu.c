class TFLTerritoryLeaderMenu extends UIScriptedMenu
{
    protected static TFLTerritoryLeaderMenu s_Menu;
    protected static ref TFLFactionLeaderPanelDto s_Panel;

    protected Widget m_Shell;
    protected TextWidget m_FactionOperator;
    protected MapWidget m_Map;
    protected Widget m_CardHost;
    protected Widget m_Card;
    protected Widget m_RequestHost;
    protected Widget m_RequestCard;

    protected TextWidget m_Name;
    protected TextWidget m_Owner;
    protected TextWidget m_Status;
    protected TextWidget m_StatusReason;
    protected TextWidget m_ChainName;
    protected TextWidget m_ChainPosition;
    protected TextWidget m_Previous;
    protected TextWidget m_Next;
    protected TextWidget m_Players;
    protected TextWidget m_PrepareTime;
    protected TextWidget m_CaptureTime;
    protected Widget m_RewardHost;
    protected ScrollWidget m_RewardScroll;
    protected ref array<Widget> m_RewardRows;

    protected ButtonWidget m_Action;
    protected ButtonWidget m_Close;
    protected ButtonWidget m_CardClose;
    protected int m_ActionCommand;

    protected TextWidget m_RequestFaction;
    protected TextWidget m_RequestTerritory;
    protected ButtonWidget m_RequestAccept;
    protected ButtonWidget m_RequestDecline;

    protected string m_SelectedTerritoryId;
    protected string m_RequestTerritoryId;

    void TFLTerritoryLeaderMenu()
    {
        m_RewardRows = new array<Widget>();
        m_SelectedTerritoryId = "";
        m_RequestTerritoryId = "";
        m_ActionCommand = -1;
    }

    static void OnPanel(TFLFactionLeaderPanelDto panel)
    {
        if (!panel) return;
        s_Panel = panel;
        if (s_Menu)
        {
            s_Menu.Refresh();
            s_Menu.AutoOpenUrgent();
        }
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/territory_leader_v2.layout");
        if (!layoutRoot) return null;

        // Bound to MapArea, not the outer Shell: LeaderCardHost/LeaderRequestCardHost
        // are children of MapArea, and SetPos() below is relative to that immediate
        // parent, not to the screen or to Shell.
        m_Shell = layoutRoot.FindAnyWidget("MapArea");
        m_FactionOperator = TextWidget.Cast(layoutRoot.FindAnyWidget("FactionOperatorText"));
        m_Map = MapWidget.Cast(layoutRoot.FindAnyWidget("LeaderTerritoryMap"));
        m_Close = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnClose"));
        m_CardHost = layoutRoot.FindAnyWidget("LeaderCardHost");
        m_RequestHost = layoutRoot.FindAnyWidget("LeaderRequestCardHost");

        if (m_CardHost)
        {
            m_Card = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryLeaderCard.layout", m_CardHost);
            if (m_Card)
            {
                m_Card.Show(false);
                m_Name = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryName"));
                m_Owner = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryOwner"));
                m_Status = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryStatus"));
                m_StatusReason = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryStatusReason"));
                m_ChainName = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryChainName"));
                m_ChainPosition = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryChainPosition"));
                m_Previous = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryPrevious"));
                m_Next = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryNext"));
                m_Players = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryPlayers"));
                m_PrepareTime = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryPrepareTime"));
                m_CaptureTime = TextWidget.Cast(m_Card.FindAnyWidget("TerritoryCaptureTime"));
                m_RewardHost = m_Card.FindAnyWidget("TerritoryRewardList");
                m_RewardScroll = ScrollWidget.Cast(m_Card.FindAnyWidget("TerritoryRewardScroll"));
                m_Action = ButtonWidget.Cast(m_Card.FindAnyWidget("TerritoryCaptureButton"));
                m_CardClose = ButtonWidget.Cast(m_Card.FindAnyWidget("TerritoryCardClose"));
            }
        }

        if (m_RequestHost)
        {
            m_RequestCard = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryRequestCard.layout", m_RequestHost);
            if (m_RequestCard)
            {
                m_RequestCard.Show(false);
                m_RequestFaction = TextWidget.Cast(m_RequestCard.FindAnyWidget("TerritoryRequestFaction"));
                m_RequestTerritory = TextWidget.Cast(m_RequestCard.FindAnyWidget("TerritoryRequestTerritory"));
                m_RequestAccept = ButtonWidget.Cast(m_RequestCard.FindAnyWidget("TerritoryRequestAccept"));
                m_RequestDecline = ButtonWidget.Cast(m_RequestCard.FindAnyWidget("TerritoryRequestDecline"));
            }
        }
        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        s_Menu = this;
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);
        TFLPassHud.Hide(true);
        InitMapView();
        Refresh();
        AutoOpenUrgent();
    }

    override void OnHide()
    {
        super.OnHide();
        ClearRewards();
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor(false);
        TFLPassHud.Hide(false);
        s_Menu = null;
    }

    protected void InitMapView()
    {
        if (!m_Map) return;
        string worldName = GetGame().GetWorldName();
        vector center = GetGame().ConfigGetVector("CfgWorlds " + worldName + " centerPosition");
        if (center[0] == 0 && center[2] == 0)
        {
            int worldSize = GetGame().GetWorld().GetWorldSize();
            center = Vector(worldSize * 0.5, 0, worldSize * 0.5);
        }
        m_Map.SetScale(0.33);
        m_Map.SetMapPos(center);
    }

    protected TFLTerritoryDto Selected()
    {
        if (!s_Panel) return null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.id == m_SelectedTerritoryId) return territory;
        }
        return null;
    }

    protected TFLTerritoryDto ByChainIndex(string chainId, int chainIndex)
    {
        if (!s_Panel || chainId == "") return null;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.chain_id == chainId && territory.chain_index == chainIndex) return territory;
        }
        return null;
    }

    protected string TimeShort(int seconds)
    {
        if (seconds < 0) seconds = 0;
        int minutes = seconds / 60;
        if (minutes > 0) return minutes.ToString() + " мин";
        return seconds.ToString() + " сек";
    }

    protected string ConfigName(string className)
    {
        if (className == "") return "";
        string path = "CfgVehicles " + className + " displayName";
        if (GetGame().ConfigIsExisting(path))
        {
            string displayName;
            GetGame().ConfigGetText(path, displayName);
            if (displayName != "") return displayName;
        }
        return className;
    }

    protected void AddChainSegment(TFLTerritoryDto fromTerr, TFLTerritoryDto toTerr, int color)
    {
        if (!m_Map || !fromTerr || !toTerr) return;
        int pieces = 10;
        for (int i = 1; i < pieces; i++)
        {
            float t = (i * 1.0) / pieces;
            vector dot = Vector(fromTerr.x + (toTerr.x - fromTerr.x) * t, fromTerr.y + (toTerr.y - fromTerr.y) * t, fromTerr.z + (toTerr.z - fromTerr.z) * t);
            m_Map.AddUserMark(dot, "", color, "TFL_Pasport/gui/textures/pass_white.edds");
        }
    }

    protected void RefreshMarks()
    {
        if (!m_Map || !s_Panel) return;
        m_Map.ClearUserMarks();
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory) continue;
            int color = ARGB(255, 190, 190, 150);
            if (territory.status == "ПОДГОТОВКА") color = ARGB(255, 240, 180, 60);
            else if (territory.status == "ЗАХВАТ" || territory.status == "ОСПАРИВАЕТСЯ") color = ARGB(255, 220, 80, 70);
            else if (territory.owner_side_id == s_Panel.side_id) color = ARGB(255, 90, 190, 90);
            string label = territory.name + " // " + territory.status;
            if (territory.chain_id != "") label = territory.chain_index.ToString() + "/" + territory.chain_total.ToString() + "  " + label;
            m_Map.AddUserMark(Vector(territory.x, territory.y, territory.z), label, color, "TFL_Pasport/gui/textures/seals/pass_seal_rad.edds");
        }
        TFLTerritoryDto selected = Selected();
        if (selected && selected.chain_id != "" && selected.chain_total > 1)
        {
            int lineColor = ARGB(210, 127, 145, 82);
            for (int c = 1; c < selected.chain_total; c++)
            {
                TFLTerritoryDto fromTerr = ByChainIndex(selected.chain_id, c);
                TFLTerritoryDto toTerr = ByChainIndex(selected.chain_id, c + 1);
                if (!fromTerr || !toTerr) continue;
                AddChainSegment(fromTerr, toTerr, lineColor);
                vector mid = Vector((fromTerr.x + toTerr.x) * 0.5, (fromTerr.y + toTerr.y) * 0.5, (fromTerr.z + toTerr.z) * 0.5);
                m_Map.AddUserMark(mid, "→", lineColor, "TFL_Pasport/gui/textures/seals/pass_seal_rad.edds");
            }
        }
    }

    protected void PositionWidgetAtMarker(Widget widget, TFLTerritoryDto territory, float width, float height)
    {
        if (!widget || !territory || !m_Map || !m_Shell) return;
        vector markerScreen = m_Map.MapToScreen(Vector(territory.x, territory.y, territory.z));
        float shellX, shellY, shellW, shellH;
        float mapX, mapY, mapW, mapH;
        m_Shell.GetScreenPos(shellX, shellY);
        m_Shell.GetScreenSize(shellW, shellH);
        m_Map.GetScreenPos(mapX, mapY);
        m_Map.GetScreenSize(mapW, mapH);
        if (shellW <= 0 || shellH <= 0 || mapW <= 0 || mapH <= 0) return;

        float localX = (markerScreen[0] - shellX) / shellW;
        float localY = (markerScreen[1] - shellY) / shellH;
        float markerMapX = (markerScreen[0] - mapX) / mapW;
        float xPos;
        if (markerMapX > 0.5) xPos = localX - width - 0.018;
        else xPos = localX + 0.018;
        float yPos = localY - height * 0.5;
        if (xPos < 0.025) xPos = 0.025;
        if (xPos > 0.975 - width) xPos = 0.975 - width;
        if (yPos < 0.095) yPos = 0.095;
        if (yPos > 0.98 - height) yPos = 0.98 - height;
        widget.SetPos(xPos, yPos);
    }

    protected void ClearRewards()
    {
        if (!m_RewardRows) return;
        for (int i = 0; i < m_RewardRows.Count(); i++)
        {
            Widget row = m_RewardRows.Get(i);
            if (row) row.Unlink();
        }
        m_RewardRows.Clear();
    }

    protected void BuildRewards(TFLTerritoryDto territory)
    {
        ClearRewards();
        if (!m_RewardHost || !territory || !territory.rewards) return;
        int count = territory.rewards.Count();
        int shown = count;
        if (shown > 4) shown = 4;
        float content = shown * 0.30;
        if (content < 1.0) content = 1.0;
        m_RewardHost.SetSize(1, content);
        float step = 0.30 / content;
        for (int i = 0; i < shown; i++)
        {
            TFLTerritoryRewardDto reward = territory.rewards.Get(i);
            if (!reward) continue;
            Widget row = GetGame().GetWorkspace().CreateWidgets("TFL_Pasport/gui/layouts/TFL_TerritoryLeaderRewardRow.layout", m_RewardHost);
            if (!row) continue;
            row.SetPos(0, i * step);
            row.SetSize(1, step);
            m_RewardRows.Insert(row);
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("RewardName"));
            TextWidget qty = TextWidget.Cast(row.FindAnyWidget("RewardQty"));
            TextWidget classText = TextWidget.Cast(row.FindAnyWidget("RewardClass"));
            if (name) name.SetText(ConfigName(reward.class_name));
            if (qty) qty.SetText("x" + reward.count.ToString());
            if (classText) classText.SetText(reward.class_name);
        }
        if (m_RewardScroll) m_RewardScroll.VScrollToPos01(0);
    }

    protected void RefreshAction(TFLTerritoryDto territory)
    {
        m_ActionCommand = -1;
        if (!m_Action || !territory) return;
        if (territory.can_start == 1)
        {
            m_ActionCommand = TFLFactionCommand.TERRITORY_START;
            m_Action.SetText("НАЧАТЬ ЗАХВАТ");
            m_Action.SetColor(ARGB(255, 41, 56, 26));
        }
        else if (territory.can_join_attack == 1)
        {
            m_ActionCommand = TFLFactionCommand.TERRITORY_JOIN_ATTACK;
            m_Action.SetText("ПРИСОЕДИНИТЬСЯ К АТАКЕ");
            m_Action.SetColor(ARGB(255, 41, 56, 26));
        }
        else if (territory.can_join_defense == 1)
        {
            m_ActionCommand = TFLFactionCommand.TERRITORY_JOIN_DEFENSE;
            m_Action.SetText("ПРИСОЕДИНИТЬСЯ К ОБОРОНЕ");
            m_Action.SetColor(ARGB(255, 41, 56, 26));
        }
        else
        {
            m_Action.SetText("ЗАХВАТ НЕДОСТУПЕН");
            m_Action.SetColor(ARGB(255, 31, 31, 28));
        }
    }

    protected void RefreshCard()
    {
        TFLTerritoryDto territory = Selected();
        if (!territory)
        {
            if (m_Card) m_Card.Show(false);
            if (m_CardHost) m_CardHost.Show(false);
            ClearRewards();
            m_ActionCommand = -1;
            return;
        }

        // LeaderCardHost defaults to hidden (empty state); it has to come back
        // with the card, or a visible card under a hidden host still renders as nothing.
        if (m_CardHost) m_CardHost.Show(true);
        if (m_Card) m_Card.Show(true);
        if (m_Name) m_Name.SetText(territory.name);
        if (m_Owner)
        {
            string owner = territory.owner_side_name;
            if (owner == "") owner = "НЕЙТРАЛЬНАЯ";
            m_Owner.SetText(owner);
        }
        if (m_Status) m_Status.SetText(territory.status);
        if (m_StatusReason)
        {
            string reason = "";
            if (territory.cooldown_left > 0) reason = "Кулдаун: " + TimeShort(territory.cooldown_left);
            else if (territory.chain_prev_name != "" && territory.can_start == 0 && territory.status != "ЗАХВАТ") reason = "Нужно захватить: " + territory.chain_prev_name;
            m_StatusReason.SetText(reason);
        }
        if (m_ChainName)
        {
            string chainName = territory.chain_name;
            if (chainName == "") chainName = "БЕЗ ЦЕПОЧКИ";
            m_ChainName.SetText(chainName);
        }
        if (m_ChainPosition)
        {
            if (territory.chain_id != "") m_ChainPosition.SetText(territory.chain_index.ToString() + " / " + territory.chain_total.ToString());
            else m_ChainPosition.SetText("-");
        }
        if (m_Previous)
        {
            string previousName = territory.chain_prev_name;
            if (previousName == "") previousName = "-";
            m_Previous.SetText(previousName);
        }
        if (m_Next)
        {
            string nextName = territory.chain_next_name;
            if (nextName == "") nextName = "-";
            m_Next.SetText(nextName);
        }
        if (m_Players) m_Players.SetText(territory.min_players.ToString());
        if (m_PrepareTime) m_PrepareTime.SetText(TimeShort(territory.preparation_seconds));
        if (m_CaptureTime) m_CaptureTime.SetText(TimeShort(territory.capture_seconds));
        BuildRewards(territory);
        RefreshAction(territory);
        PositionWidgetAtMarker(m_CardHost, territory, 0.225, 0.62);
    }

    protected void RefreshRequest()
    {
        if (!m_RequestCard || !s_Panel) return;
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
            m_RequestTerritoryId = "";
            m_RequestCard.Show(false);
            if (m_RequestHost) m_RequestHost.Show(false);
            return;
        }
        m_RequestTerritoryId = requestTerritory.id;
        if (m_RequestFaction) m_RequestFaction.SetText(requestTerritory.pending_ally_faction_name);
        if (m_RequestTerritory) m_RequestTerritory.SetText(requestTerritory.name);
        if (m_RequestHost) m_RequestHost.Show(true);
        m_RequestCard.Show(true);
        PositionWidgetAtMarker(m_RequestHost, requestTerritory, 0.208, 0.34);
    }

    protected void AutoOpenUrgent()
    {
        if (!s_Panel) return;
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (territory && territory.can_accept_ally == 1)
            {
                m_SelectedTerritoryId = territory.id;
                RefreshMarks();
                RefreshCard();
                RefreshRequest();
                return;
            }
        }
    }

    void Refresh()
    {
        if (!s_Panel || !layoutRoot) return;
        if (m_FactionOperator) m_FactionOperator.SetText("ФРАКЦИЯ: " + s_Panel.faction_name + " // " + s_Panel.side_name);
        RefreshMarks();
        RefreshCard();
        RefreshRequest();
    }

    protected void SelectNearest(int x, int y)
    {
        if (!m_Map || !s_Panel) return;
        float best = 6400.0;
        string bestId = "";
        float mapX, mapY;
        m_Map.GetScreenPos(mapX, mapY);
        for (int i = 0; i < s_Panel.territories.Count(); i++)
        {
            TFLTerritoryDto territory = s_Panel.territories.Get(i);
            if (!territory) continue;
            vector screen = m_Map.MapToScreen(Vector(territory.x, territory.y, territory.z));
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
            m_SelectedTerritoryId = bestId;
            RefreshMarks();
            RefreshCard();
        }
    }

    protected void SendTerritoryCommand(int command)
    {
        if (m_SelectedTerritoryId == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_SelectedTerritoryId;
        TFLFactionClientRPC.Send(command, dto);
    }

    protected void SendRequestCommand(int command)
    {
        if (m_RequestTerritoryId == "") return;
        TFLFactionCommandDto dto = new TFLFactionCommandDto();
        dto.territory_id = m_RequestTerritoryId;
        TFLFactionClientRPC.Send(command, dto);
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (GetUApi().GetInputByID(UAUIBack).LocalPress()) Close();
    }

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
    {
        super.OnMouseButtonDown(w, x, y, button);
        if (w == m_Map && button == MouseState.LEFT)
        {
            SelectNearest(x, y);
            return true;
        }
        return false;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_Close)
        {
            Close();
            return true;
        }
        if (w == m_CardClose)
        {
            m_SelectedTerritoryId = "";
            RefreshMarks();
            RefreshCard();
            return true;
        }
        if (w == m_Map)
        {
            SelectNearest(x, y);
            return true;
        }
        if (w == m_Action)
        {
            if (m_ActionCommand >= 0) SendTerritoryCommand(m_ActionCommand);
            return true;
        }
        if (w == m_RequestAccept)
        {
            SendRequestCommand(TFLFactionCommand.TERRITORY_ACCEPT_ALLY);
            if (m_RequestCard) m_RequestCard.Show(false);
            return true;
        }
        if (w == m_RequestDecline)
        {
            SendRequestCommand(TFLFactionCommand.TERRITORY_DECLINE_ALLY);
            if (m_RequestCard) m_RequestCard.Show(false);
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
