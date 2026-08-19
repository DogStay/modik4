class TFLFactionManager
{
    protected static ref TFLFactionManager s_Instance;
    protected ref TFLFactionStore m_Store;
    protected ref TFLFactionPassportIndex m_Passports;
    protected ref map<string, ref TFLFactionInviteState> m_Invites;
    protected ref map<string, string> m_OpenFaction;
    protected ref map<string, int> m_OpenMode;
    protected ref map<string, Object> m_OpenLaptop;
    protected ref array<Object> m_FactionLaptops;
    protected ref array<string> m_FactionLaptopIds;
    protected ref array<Object> m_RewardChests;
    protected ref array<string> m_RewardChestFactionIds;
    protected float m_Timer;

    static const int MODE_LEADER = 1;
    static const int MODE_ADMIN = 2;
    static const int INVITE_LIFETIME_MS = 60000;
    static const float INVITE_DISTANCE = 3.0;
    static const float LAPTOP_DISTANCE = 3.0;

    static TFLFactionManager Get()
    {
        if (!s_Instance) s_Instance = new TFLFactionManager();
        return s_Instance;
    }

    void TFLFactionManager()
    {
        m_Store = new TFLFactionStore();
        m_Passports = new TFLFactionPassportIndex();
        m_Invites = new map<string, ref TFLFactionInviteState>();
        m_OpenFaction = new map<string, string>();
        m_OpenMode = new map<string, int>();
        m_OpenLaptop = new map<string, Object>();
        m_FactionLaptops = new array<Object>();
        m_FactionLaptopIds = new array<string>();
        m_RewardChests = new array<Object>();
        m_RewardChestFactionIds = new array<string>();
        m_Timer = 0;
    }

    TFLFactionStore Store() { return m_Store; }

    void Init()
    {
        m_Store.LoadAll();
        m_Passports.Reload();
        SpawnFactionLaptops();
        SpawnRewardChests();
        TFLFactionLog.Info("Система фракций инициализирована");
    }

    void OnUpdate(float timeslice)
    {
        m_Timer = m_Timer + timeslice;
        if (m_Timer < 5.0) return;
        m_Timer = 0;
        CleanupInvites();
        CleanupOpenSessions();
        SyncAllOnlineFlags();
    }

    void OnPlayerConnect(PlayerBase player)
    {
        SyncPlayerFlags(player);
    }

    void OnPassportChanged(TFLPassRecord rec)
    {
        if (!rec) return;
        m_Passports.Upsert(rec);
        m_Store.UpdateMemberNickname(rec.steamid, rec.FullName());
        BroadcastAdminPanels("");
        string factionId = m_Store.GetFactionIdByUid(rec.steamid);
        if (factionId != "") BroadcastFaction(factionId);
        TFLFactionLog.Info("Паспортный каталог обновлён: " + rec.steamid);
    }

    bool HasPermission(string uid, string permission)
    {
        TFLFactionRole role = m_Store.GetRoleForUid(uid);
        if (!role || !role.permissions) return false;
        if (permission == TFLFactionPerm.USE_LAPTOP) return role.permissions.use_laptop == 1;
        if (permission == TFLFactionPerm.INVITE) return role.permissions.invite == 1;
        if (permission == TFLFactionPerm.KICK) return role.permissions.kick == 1;
        if (permission == TFLFactionPerm.PROMOTE) return role.permissions.promote == 1;
        if (permission == TFLFactionPerm.DEMOTE) return role.permissions.demote == 1;
        if (permission == TFLFactionPerm.MANAGE_ROLES) return role.permissions.manage_roles == 1;
        if (permission == TFLFactionPerm.EDIT_ROLE_PERMISSIONS) return role.permissions.edit_role_permissions == 1;
        if (permission == TFLFactionPerm.ISSUE_LICENSES) return role.permissions.issue_licenses == 1;
        return false;
    }

    protected void SyncAllOnlineFlags()
    {
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player) SyncPlayerFlags(player);
        }
    }

    void SyncPlayerFlags(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return;
        string uid = player.GetIdentity().GetPlainId();
        bool member = m_Store.GetFactionIdByUid(uid) != "";
        bool admin = m_Store.IsAdmin(uid);
        // TFL administrator status no longer grants faction gameplay permissions.
        // Global administration lives in VPP; world actions follow the actor's faction role.
        bool invite = HasPermission(uid, TFLFactionPerm.INVITE);
        bool laptop = m_Store.IsLeaderUid(uid);
        bool issue = HasPermission(uid, TFLFactionPerm.ISSUE_LICENSES);
        bool leader = m_Store.IsLeaderUid(uid);
        player.TFLFactionSetFlags(member, admin, invite, laptop, issue, leader);
    }

    PlayerBase FindPlayer(string uid)
    {
        if (uid == "") return null;
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity()) continue;
            if (player.GetIdentity().GetPlainId() == uid) return player;
        }
        return null;
    }

    protected string FactionIdForLaptop(TFL_PassportLaptop laptop)
    {
        if (!laptop) return "";
        int idx = m_FactionLaptops.Find(laptop);
        if (idx < 0 || idx >= m_FactionLaptopIds.Count()) return "";
        return m_FactionLaptopIds.Get(idx);
    }

    protected Object CreateLaptopObject(vector pos, vector rot)
    {
        Object obj = GetGame().CreateObjectEx("TFL_PassportLaptop", pos, ECE_SETUP | ECE_UPDATEPATHGRAPH | ECE_CREATEPHYSICS | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN);
        if (!obj)
        {
            obj = GetGame().CreateObject("TFL_PassportLaptop", "0 0 0", false, false, false);
        }
        if (!obj)
        {
            TFLFactionLog.Error("TFL_PassportLaptop: CreateObject/CreateObjectEx вернули NULL");
            return null;
        }

        obj.SetPosition(pos);
        obj.SetOrientation(rot);
        obj.SetFlags(EntityFlags.STATIC, false);
        obj.Update();
        obj.SetAffectPathgraph(true, false);
        if (obj.CanAffectPathgraph())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().UpdatePathgraphRegionByObject, 100, false, obj);
        return obj;
    }

    protected bool HasFactionLaptop(string factionId)
    {
        return m_FactionLaptopIds.Find(factionId) >= 0;
    }

    void SpawnFactionLaptops()
    {
        for (int i = 0; i < m_FactionLaptops.Count(); i++)
        {
            Object old = m_FactionLaptops.Get(i);
            if (old) GetGame().ObjectDelete(old);
        }
        m_FactionLaptops.Clear();
        m_FactionLaptopIds.Clear();

        array<TFLFactionDefinition> defs;
        m_Store.GetDefinitions(defs);
        for (int k = 0; k < defs.Count(); k++)
        {
            TFLFactionDefinition def = defs.Get(k);
            if (!def || !def.laptop || def.laptop.Active != 1) continue;

            vector pos = def.laptop.Pos();
            if (pos[0] == 0 && pos[2] == 0)
            {
                TFLFactionLog.Error("Ноутбук " + def.id + ": координаты X/Z равны 0, спавн пропущен");
                continue;
            }

            // Saved XYZ is authoritative, including building floors and tables.
            Object obj = CreateLaptopObject(pos, def.laptop.Rot());
            TFL_PassportLaptop laptop = TFL_PassportLaptop.Cast(obj);
            if (!laptop)
            {
                if (obj) GetGame().ObjectDelete(obj);
                TFLFactionLog.Error("Ноутбук " + def.id + ": объект создан, но Cast<TFL_PassportLaptop> не прошёл");
                continue;
            }

            laptop.TFLFactionSetLaptop(true);
            m_FactionLaptops.Insert(obj);
            m_FactionLaptopIds.Insert(def.id);
            TFLFactionLog.Info("Фракционный ноутбук создан: " + def.id + " // " + pos.ToString());
        }
    }

    void SpawnRewardChests()
    {
        for (int i = 0; i < m_RewardChests.Count(); i++)
        {
            Object old = m_RewardChests.Get(i);
            if (old) GetGame().ObjectDelete(old);
        }
        m_RewardChests.Clear();
        m_RewardChestFactionIds.Clear();
        array<TFLFactionDefinition> defs;
        m_Store.GetDefinitions(defs);
        for (int d = 0; d < defs.Count(); d++)
        {
            TFLFactionDefinition def = defs.Get(d);
            if (!def || !def.reward_chests) continue;
            for (int c = 0; c < def.reward_chests.Count(); c++)
            {
                TFLRewardChestDef chestDef = def.reward_chests.Get(c);
                if (!chestDef || chestDef.Active != 1) continue;
                vector pos = chestDef.Pos();
                if (pos[0] == 0 && pos[2] == 0) continue;
                Object obj = GetGame().CreateObjectEx("TFL_FactionRewardChest", pos, ECE_CREATEPHYSICS);
                if (!obj) continue;
                obj.SetOrientation(chestDef.Rot());
                obj.SetPosition(pos);
                EntityAI entity = EntityAI.Cast(obj);
                if (entity) entity.SetAllowDamage(false);
                m_RewardChests.Insert(obj);
                m_RewardChestFactionIds.Insert(def.id);
            }
        }
    }

    bool OpenLaptop(PlayerBase player, TFL_PassportLaptop laptop)
    {
        if (!player || !player.GetIdentity() || !laptop) return false;
        if (vector.Distance(player.GetPosition(), laptop.GetPosition()) > LAPTOP_DISTANCE) return false;
        string uid = player.GetIdentity().GetPlainId();
        string laptopFaction = FactionIdForLaptop(laptop);
        if (!laptop.TFLFactionIsLaptop() || laptopFaction == "") return false;
        string memberFaction = m_Store.GetFactionIdByUid(uid);
        if (memberFaction == "" || memberFaction != laptopFaction) return false;
        // The physical faction laptop is gameplay-only and belongs to the faction leader.
        if (!m_Store.IsLeaderUid(uid)) return false;
        m_OpenMode.Set(uid, MODE_LEADER);
        m_OpenLaptop.Set(uid, laptop);
        m_OpenFaction.Set(uid, memberFaction);
        SendLeaderPanel(player, memberFaction);
        return true;
    }

    protected bool ValidLaptopSession(PlayerBase player, int requiredMode)
    {
        if (!player || !player.GetIdentity()) return false;
        string uid = player.GetIdentity().GetPlainId();
        if (!m_OpenMode.Contains(uid) || m_OpenMode.Get(uid) != requiredMode) return false;
        if (!m_OpenLaptop.Contains(uid)) return false;
        Object laptopObj = m_OpenLaptop.Get(uid);
        TFL_PassportLaptop laptop = TFL_PassportLaptop.Cast(laptopObj);
        if (!laptop) return false;
        if (vector.Distance(player.GetPosition(), laptop.GetPosition()) > LAPTOP_DISTANCE) return false;

        if (requiredMode == MODE_ADMIN)
            return m_Store.IsAdmin(uid);

        if (!laptop.TFLFactionIsLaptop()) return false;
        string laptopFaction = FactionIdForLaptop(laptop);
        if (laptopFaction == "") return false;
        if (m_Store.GetFactionIdByUid(uid) != laptopFaction) return false;
        return m_Store.IsLeaderUid(uid);
    }

    protected void CleanupOpenSessions()
    {
        array<string> uids = new array<string>();
        for (int i = 0; i < m_OpenMode.Count(); i++) uids.Insert(m_OpenMode.GetKey(i));
        for (int k = 0; k < uids.Count(); k++)
        {
            string uid = uids.Get(k);
            PlayerBase player = FindPlayer(uid);
            int mode = m_OpenMode.Get(uid);
            if (player && ValidLaptopSession(player, mode)) continue;
            m_OpenMode.Remove(uid);
            m_OpenFaction.Remove(uid);
            m_OpenLaptop.Remove(uid);
        }
    }

    string Invite(PlayerBase inviter, PlayerBase target)
    {
        if (!inviter || !target || !inviter.GetIdentity() || !target.GetIdentity()) return "Нет цели.";
        if (inviter == target) return "Нельзя пригласить себя.";
        if (vector.Distance(inviter.GetPosition(), target.GetPosition()) > INVITE_DISTANCE) return "Игрок слишком далеко.";
        string inviterUid = inviter.GetIdentity().GetPlainId();
        string targetUid = target.GetIdentity().GetPlainId();
        string factionId = m_Store.GetFactionIdByUid(inviterUid);
        if (factionId == "" || !HasPermission(inviterUid, TFLFactionPerm.INVITE)) return "Нет права приглашать.";
        if (factionId == "") return "Не выбрана фракция.";
        if (m_Store.GetFactionIdByUid(targetUid) != "") return "Игрок уже состоит во фракции.";
        TFLFactionDefinition def = m_Store.GetDefinition(factionId);
        if (!def) return "Фракция не найдена.";
        TFLFactionInviteState state = new TFLFactionInviteState();
        state.target_uid = targetUid;
        state.inviter_uid = inviterUid;
        state.inviter_name = inviter.GetIdentity().GetName();
        state.faction_id = factionId;
        state.expires_at_ms = GetGame().GetTime() + INVITE_LIFETIME_MS;
        m_Invites.Set(targetUid, state);
        TFLFactionInviteDto dto = new TFLFactionInviteDto();
        dto.faction_id = factionId; dto.faction_name = def.name; dto.inviter_name = state.inviter_name; dto.seconds = 60;
        SendInviteDto(target, dto);
        return "";
    }

    void AcceptInvite(PlayerBase target)
    {
        if (!target || !target.GetIdentity()) return;
        string uid = target.GetIdentity().GetPlainId();
        if (!m_Invites.Contains(uid)) { Notify(target, "Приглашение недействительно."); return; }
        TFLFactionInviteState state = m_Invites.Get(uid);
        if (!state || GetGame().GetTime() > state.expires_at_ms) { m_Invites.Remove(uid); Notify(target, "Время приглашения истекло."); return; }
        if (m_Store.GetFactionIdByUid(uid) != "") { m_Invites.Remove(uid); return; }
        bool allowed = HasPermission(state.inviter_uid, TFLFactionPerm.INVITE);
        if (!allowed) { m_Invites.Remove(uid); return; }
        if (!m_Store.AddMember(state.faction_id, uid, target.GetIdentity().GetName(), state.inviter_uid)) { Notify(target, "Не удалось вступить во фракцию."); return; }
        TFLFactionSalaryManager.Get().ResetWorkCycle(uid, "faction joined");
        m_Invites.Remove(uid);
        SyncPlayerFlags(target);
        TFLFactionDefinition def = m_Store.GetDefinition(state.faction_id);
        if (def) Notify(target, "Вы вступили во фракцию: " + def.name);
        BroadcastFaction(state.faction_id);
        BroadcastAdminPanels(state.faction_id);
    }

    void DeclineInvite(PlayerBase target)
    {
        if (!target || !target.GetIdentity()) return;
        m_Invites.Remove(target.GetIdentity().GetPlainId());
        Notify(target, "Приглашение отклонено.");
    }

    protected void CleanupInvites()
    {
        int now = GetGame().GetTime();
        array<string> expired = new array<string>();
        for (int i = 0; i < m_Invites.Count(); i++)
        {
            TFLFactionInviteState state = m_Invites.GetElement(i);
            if (!state || now > state.expires_at_ms) expired.Insert(m_Invites.GetKey(i));
        }
        for (int k = 0; k < expired.Count(); k++) m_Invites.Remove(expired.Get(k));
    }

    void HandleCommand(PlayerBase player, int command, TFLFactionCommandDto dto)
    {
        if (!player || !player.GetIdentity()) return;
        ref TFLFactionCommandDto safeDto = dto;
        if (!safeDto) safeDto = new TFLFactionCommandDto();
        string uid = player.GetIdentity().GetPlainId();

        if (command == TFLFactionCommand.ACCEPT_INVITE) { AcceptInvite(player); return; }
        if (command == TFLFactionCommand.DECLINE_INVITE) { DeclineInvite(player); return; }
        if (command == TFLFactionCommand.REQUEST_PANEL)
        {
            if (m_Store.IsAdmin(uid))
            {
                if (!ValidLaptopSession(player, MODE_ADMIN)) { Notify(player, "Сессия ADMIN-ноутбука недействительна."); return; }
                SendAdminPanel(player, safeDto.faction_id);
            }
            else
            {
                if (!ValidLaptopSession(player, MODE_LEADER)) { Notify(player, "Отойдите к своему фракционному ноутбуку и откройте его снова."); return; }
                SendLeaderPanel(player, m_Store.GetFactionIdByUid(uid));
            }
            return;
        }

        if (command >= TFLFactionCommand.TERRITORY_START && command <= TFLFactionCommand.TERRITORY_JOIN_DEFENSE)
        {
            if (!ValidLaptopSession(player, MODE_LEADER) || !m_Store.IsLeaderUid(uid))
            {
                Notify(player, "Управление захватом доступно лидеру только у фракционного ноутбука.");
                return;
            }
            if (command == TFLFactionCommand.TERRITORY_START) CommandResult(player, TFLTerritoryManager.Get().StartCapture(player, safeDto.territory_id));
            else if (command == TFLFactionCommand.TERRITORY_JOIN_ATTACK) CommandResult(player, TFLTerritoryManager.Get().RequestJoinAttack(player, safeDto.territory_id));
            else if (command == TFLFactionCommand.TERRITORY_ACCEPT_ALLY) CommandResult(player, TFLTerritoryManager.Get().ResolveAlly(player, safeDto.territory_id, true));
            else if (command == TFLFactionCommand.TERRITORY_DECLINE_ALLY) CommandResult(player, TFLTerritoryManager.Get().ResolveAlly(player, safeDto.territory_id, false));
            else if (command == TFLFactionCommand.TERRITORY_JOIN_DEFENSE) CommandResult(player, TFLTerritoryManager.Get().JoinDefense(player, safeDto.territory_id));
            return;
        }

        // Весь административный диапазон идёт только через VPP Admin Tools, где
        // отправителя проверяет TFL_CanAdminWrite. Верхней границей раньше стоял
        // ADMIN_SALARY_SETTINGS_SAVE (58), из-за чего команды цепочек и пресетов
        // наград (70..81) проскакивали мимо этой отсечки в лидерский обработчик
        // и меняли данные в обход проверки прав.
        if (command >= TFLFactionCommand.ADMIN_SELECT && command <= TFLFactionCommand.ADMIN_TERRITORY_SET_REWARD_PRESET)
        {
            Notify(player, "Администрирование TFL перенесено в VPP Admin Tools.");
            return;
        }
        if (!ValidLaptopSession(player, MODE_LEADER)) { Notify(player, "Фракционный ноутбук слишком далеко или сессия закрыта."); return; }
        HandleLeaderCommand(player, uid, command, safeDto);
    }

    protected void CommandResult(PlayerBase player, string err)
    {
        if (err != "") Notify(player, err);
    }

    protected bool SameFaction(string firstUid, string secondUid)
    {
        string a = m_Store.GetFactionIdByUid(firstUid);
        string b = m_Store.GetFactionIdByUid(secondUid);
        return a != "" && a == b;
    }

    protected bool CanManageTarget(string actorUid, string targetUid)
    {
        if (!SameFaction(actorUid, targetUid)) return false;
        TFLFactionRole a = m_Store.GetRoleForUid(actorUid);
        TFLFactionRole b = m_Store.GetRoleForUid(targetUid);
        if (!a || !b) return false;
        return a.level > b.level;
    }

    protected void HandleLeaderCommand(PlayerBase player, string uid, int command, TFLFactionCommandDto dto)
    {
        string factionId = m_Store.GetFactionIdByUid(uid);
        TFLFactionDefinition def = m_Store.GetDefinition(factionId);
        if (!def) { Notify(player, "Вы не состоите во фракции."); return; }

        if (command == TFLFactionCommand.PROMOTE_MEMBER || command == TFLFactionCommand.DEMOTE_MEMBER || command == TFLFactionCommand.KICK_MEMBER)
        {
            if (!CanManageTarget(uid, dto.target_uid)) { Notify(player, "Нельзя управлять этим участником."); return; }
            TFLFactionMember target = m_Store.GetMember(factionId, dto.target_uid);
            TFLFactionRole current = m_Store.GetRole(def, target.role_id);
            if (command == TFLFactionCommand.KICK_MEMBER)
            {
                if (!HasPermission(uid, TFLFactionPerm.KICK)) { Notify(player, "Нет права исключать."); return; }
                if (m_Store.RemoveMember(factionId, dto.target_uid))
                {
                    TFLFactionSalaryManager.Get().ResetWorkCycle(dto.target_uid, "faction kicked");
                    SyncOnlineByUid(dto.target_uid);
                    PlayerBase kickedPlayer = FindPlayer(dto.target_uid);
                    if (kickedPlayer) Notify(kickedPlayer, "Вы исключены из фракции. Статус: Гражданские.");
                }
            }
            else if (command == TFLFactionCommand.PROMOTE_MEMBER)
            {
                if (!HasPermission(uid, TFLFactionPerm.PROMOTE)) { Notify(player, "Нет права повышать."); return; }
                TFLFactionRole next = m_Store.NextHigher(def, current);
                TFLFactionRole actor = m_Store.GetRoleForUid(uid);
                if (next && actor && next.level < actor.level) m_Store.SetMemberRole(factionId, dto.target_uid, next.id);
                else Notify(player, "Нет доступного повышения.");
            }
            else
            {
                if (!HasPermission(uid, TFLFactionPerm.DEMOTE)) { Notify(player, "Нет права понижать."); return; }
                TFLFactionRole lower = m_Store.NextLower(def, current);
                if (lower) m_Store.SetMemberRole(factionId, dto.target_uid, lower.id);
            }
            SyncOnlineByUid(dto.target_uid);
            BroadcastFaction(factionId); BroadcastAdminPanels(factionId); return;
        }

        if (command == TFLFactionCommand.CREATE_ROLE)
        {
            if (!HasPermission(uid, TFLFactionPerm.MANAGE_ROLES)) { Notify(player, "Нет права создавать роли."); return; }
            TFLFactionRole actorRole = m_Store.GetRoleForUid(uid);
            if (actorRole && dto.role_level >= actorRole.level) { Notify(player, "Нельзя создать роль своего уровня или выше."); return; }
            if (!m_Store.AddRole(factionId, dto.role_id, dto.role_name, dto.role_level, dto.permissions)) Notify(player, "Роль не создана.");
            BroadcastFaction(factionId); BroadcastAdminPanels(factionId); return;
        }

        if (command == TFLFactionCommand.SAVE_ROLE)
        {
            if (!HasPermission(uid, TFLFactionPerm.EDIT_ROLE_PERMISSIONS)) { Notify(player, "Нет права редактировать роли."); return; }
            TFLFactionRole targetRole = m_Store.GetRole(def, dto.role_id);
            TFLFactionRole actorEditRole = m_Store.GetRoleForUid(uid);
            if (!targetRole || !actorEditRole || targetRole.level >= actorEditRole.level) { Notify(player, "Эту роль редактировать нельзя."); return; }
            if (dto.role_level >= actorEditRole.level) { Notify(player, "Уровень роли слишком высокий."); return; }
            m_Store.SaveRole(factionId, dto.role_id, dto.role_name, dto.role_level, dto.permissions);
            SyncFactionFlags(factionId); BroadcastFaction(factionId); BroadcastAdminPanels(factionId); return;
        }
    }

    protected string SelectedFaction(string uid, TFLFactionCommandDto dto)
    {
        string id = dto.faction_id;
        if (id == "" && m_OpenFaction.Contains(uid)) id = m_OpenFaction.Get(uid);
        return id;
    }

    protected bool ValidInventoryClass(string className)
    {
        if (className == "") return false;
        if (GetGame().ConfigIsExisting("CfgVehicles " + className)) return true;
        if (GetGame().ConfigIsExisting("CfgWeapons " + className)) return true;
        if (GetGame().ConfigIsExisting("CfgMagazines " + className)) return true;
        return false;
    }

    protected void HandleAdminCommand(PlayerBase player, string uid, int command, TFLFactionCommandDto dto)
    {
        string factionId = SelectedFaction(uid, dto);
        if (command == TFLFactionCommand.ADMIN_SELECT)
        {
            if (m_Store.GetDefinition(dto.faction_id)) m_OpenFaction.Set(uid, dto.faction_id);
            SendAdminPanel(player, dto.faction_id); return;
        }
        if (command == TFLFactionCommand.ADMIN_CREATE)
        {
            if (m_Store.CreateFaction(dto.faction_id, dto.faction_name))
            {
                m_OpenFaction.Set(uid, TFLFactionText.SafeId(dto.faction_id));
                Audit("ADMIN", uid + " created faction " + dto.faction_id);
            }
            SendAdminPanel(player, m_OpenFaction.Get(uid)); return;
        }
        TFLFactionDefinition def = m_Store.GetDefinition(factionId);
        bool rankCommand = command >= TFLFactionCommand.ADMIN_CREATE_RANK && command <= TFLFactionCommand.ADMIN_SET_MEMBER_RANK;
        if (!def && (command < TFLFactionCommand.ADMIN_LICENSE_CREATE || command == TFLFactionCommand.ADMIN_CLEAR_SPAWN_POINTS || rankCommand)) { Notify(player, "Сначала выберите фракцию."); return; }

        if (command == TFLFactionCommand.ADMIN_SAVE_NAME)
        {
            string shortName = dto.short_name; if (shortName == "") shortName = def.short_name;
            string sideId = dto.side_id; if (sideId == "") sideId = def.side_id;
            m_Store.SaveFactionIdentity(factionId, dto.faction_name, shortName, sideId);
        }
        else if (command == TFLFactionCommand.ADMIN_SAVE_SIDE)
        {
            string name = dto.faction_name; if (name == "") name = def.name;
            string shortName2 = dto.short_name; if (shortName2 == "") shortName2 = def.short_name;
            m_Store.SaveFactionIdentity(factionId, name, shortName2, dto.side_id);
        }
        else if (command == TFLFactionCommand.ADMIN_SPAWN_HERE)
        {
            if (def.system_civilian == 1 || factionId == TFLFactionSystem.CIVILIAN_ID)
            {
                if (m_Store.AddSpawnPoint(factionId, player.GetPosition(), player.GetOrientation()))
                    Notify(player, "Точка спавна гражданских добавлена.");
            }
            else
            {
                def.spawn.SetTransform(player.GetPosition(), player.GetOrientation());
                def.revision = def.revision + 1;
                m_Store.SaveDefinition(def);
            }
        }
        else if (command == TFLFactionCommand.ADMIN_CLEAR_SPAWN_POINTS)
        {
            if (def.system_civilian == 1 || factionId == TFLFactionSystem.CIVILIAN_ID)
            {
                m_Store.ClearSpawnPoints(factionId);
                Notify(player, "Точки спавна гражданских очищены.");
            }
        }
        else if (command == TFLFactionCommand.ADMIN_SPAWN_SET_YAW)
        {
            if (!m_Store.SetSpawnYaw(factionId, dto.int_value))
                Notify(player, "Сначала поставьте точку SPAWN.");
        }
        else if (command == TFLFactionCommand.ADMIN_LAPTOP_SET_YAW)
        {
            if (m_Store.SetLaptopYaw(factionId, dto.int_value))
                SpawnFactionLaptops();
            else
                Notify(player, "Сначала поставьте фракционный ноутбук.");
        }
        else if (command == TFLFactionCommand.ADMIN_LAPTOP_HERE)
        {
            vector laptopPos = player.GetPosition();
            vector laptopRot = player.GetOrientation();
            def.laptop.SetTransform(laptopPos, laptopRot);
            def.laptop.SnapToGround = 0;
            def.laptop.HeightOffset = 0;
            def.revision = def.revision + 1;
            m_Store.SaveDefinition(def);
            SpawnFactionLaptops();
            if (HasFactionLaptop(factionId))
                Notify(player, "Фракционный ноутбук создан. Если он под ногами — отойдите на 1-2 метра.");
            else
                Notify(player, "Не удалось создать ноутбук. Смотрите TFLFactionLog/RPT: CreateObject или Cast вернул ошибку.");
        }
        else if (command == TFLFactionCommand.ADMIN_PRESET_ME)
        {
            CopyPresetFromPlayer(player, def); m_Store.SaveDefinition(def);
        }
        else if (command == TFLFactionCommand.ADMIN_CLEAR_PRESET)
        {
            def.spawn_preset.items.Clear(); def.revision = def.revision + 1; m_Store.SaveDefinition(def);
        }
        else if (command == TFLFactionCommand.ADMIN_PRESET_ADD)
        {
            if (!ValidInventoryClass(dto.class_name))
                Notify(player, "Classname не найден в конфиге.");
            else if (def.spawn_preset.items.Find(dto.class_name) == -1)
            {
                def.spawn_preset.items.Insert(dto.class_name);
                def.revision = def.revision + 1;
                m_Store.SaveDefinition(def);
            }
        }
        else if (command == TFLFactionCommand.ADMIN_PRESET_REMOVE)
        {
            int idx = def.spawn_preset.items.Find(dto.class_name); if (idx != -1) def.spawn_preset.items.Remove(idx); def.revision = def.revision + 1; m_Store.SaveDefinition(def);
        }
        else if (command == TFLFactionCommand.ADMIN_PRESET_TOGGLE_CLEAR)
        {
            if (def.spawn_preset.clear_inventory == 1) def.spawn_preset.clear_inventory = 0; else def.spawn_preset.clear_inventory = 1; def.revision = def.revision + 1; m_Store.SaveDefinition(def);
        }
        else if (command == TFLFactionCommand.ADMIN_ADD_CHEST_HERE)
        {
            if (m_Store.AddRewardChest(factionId, player.GetPosition(), player.GetOrientation())) SpawnRewardChests();
        }
        else if (command == TFLFactionCommand.ADMIN_CLEAR_CHESTS)
        {
            if (m_Store.ClearRewardChests(factionId)) SpawnRewardChests();
        }
        else if (command == TFLFactionCommand.ADMIN_ADD_MEMBER)
        {
            if (def.system_civilian == 1 || factionId == TFLFactionSystem.CIVILIAN_ID)
            {
                Notify(player, "Гражданские назначаются автоматически и не имеют списка участников.");
            }
            else
            {
            string addName = dto.target_uid;
            TFLFactionPassportPersonDto person = m_Passports.Get(dto.target_uid);
            if (person) addName = person.name;
            if (!m_Store.AddMemberWithRole(factionId, dto.target_uid, addName, dto.role_id, uid))
                Notify(player, "Игрок не добавлен.");
            else
                TFLFactionSalaryManager.Get().ResetWorkCycle(dto.target_uid, "faction assigned");
            SyncOnlineByUid(dto.target_uid);
            }
        }
        else if (command == TFLFactionCommand.ADMIN_PROMOTE || command == TFLFactionCommand.ADMIN_DEMOTE || command == TFLFactionCommand.ADMIN_KICK)
        {
            if (def.system_civilian == 1 || factionId == TFLFactionSystem.CIVILIAN_ID)
            {
                Notify(player, "У системной фракции Гражданские нет списка участников.");
            }
            else
            {
            TFLFactionMember member = m_Store.GetMember(factionId, dto.target_uid);
            if (member)
            {
                TFLFactionRole current = m_Store.GetRole(def, member.role_id);
                if (command == TFLFactionCommand.ADMIN_KICK)
                {
                    if (m_Store.RemoveMember(factionId, dto.target_uid))
                    {
                        TFLFactionSalaryManager.Get().ResetWorkCycle(dto.target_uid, "faction kicked by admin");
                        PlayerBase kickedAdminTarget = FindPlayer(dto.target_uid);
                        if (kickedAdminTarget) Notify(kickedAdminTarget, "Вы исключены из фракции. Статус: Гражданские.");
                    }
                }
                else if (command == TFLFactionCommand.ADMIN_PROMOTE)
                {
                    TFLFactionRole higher = m_Store.NextHigher(def, current); if (higher) m_Store.SetMemberRole(factionId, dto.target_uid, higher.id);
                }
                else
                {
                    TFLFactionRole lower = m_Store.NextLower(def, current); if (lower) m_Store.SetMemberRole(factionId, dto.target_uid, lower.id);
                }
                SyncOnlineByUid(dto.target_uid);
            }
            }
        }
        else if (command == TFLFactionCommand.ADMIN_CREATE_ROLE)
            m_Store.AddRole(factionId, dto.role_id, dto.role_name, dto.role_level, dto.permissions);
        else if (command == TFLFactionCommand.ADMIN_SAVE_ROLE)
            m_Store.SaveRole(factionId, dto.role_id, dto.role_name, dto.role_level, dto.permissions);
        else if (command == TFLFactionCommand.ADMIN_CREATE_RANK)
        {
            if (!m_Store.AddRank(factionId, dto.rank_id, dto.rank_name, dto.rank_level, dto.salary, dto.currency_id))
                Notify(player, "Звание не создано. Проверьте ID, уровень и зарплату.");
        }
        else if (command == TFLFactionCommand.ADMIN_SAVE_RANK)
        {
            if (!m_Store.SaveRank(factionId, dto.rank_id, dto.rank_name, dto.rank_level, dto.salary, dto.currency_id))
                Notify(player, "Звание не сохранено.");
        }
        else if (command == TFLFactionCommand.ADMIN_DELETE_RANK)
        {
            TFLFactionMembersFile rankMembers = m_Store.GetMembers(factionId);
            array<string> resetRankUids = new array<string>();
            if (rankMembers)
            {
                for (int rm = 0; rm < rankMembers.members.Count(); rm++)
                {
                    TFLFactionMember rankMember = rankMembers.members.Get(rm);
                    if (rankMember && rankMember.rank_id == dto.rank_id) resetRankUids.Insert(rankMember.steamid);
                }
            }
            if (!m_Store.DeleteRank(factionId, dto.rank_id))
                Notify(player, "Нельзя удалить это звание. Звание по умолчанию защищено.");
            else
            {
                for (int rr = 0; rr < resetRankUids.Count(); rr++)
                    TFLFactionSalaryManager.Get().ResetWorkCycle(resetRankUids.Get(rr), "rank deleted");
            }
        }
        else if (command == TFLFactionCommand.ADMIN_SET_MEMBER_RANK)
        {
            if (m_Store.SetMemberRank(factionId, dto.target_uid, dto.rank_id))
            {
                TFLFactionSalaryManager.Get().ResetWorkCycle(dto.target_uid, "rank changed");
                SyncOnlineByUid(dto.target_uid);
                TFLFactionRank assignedRank = m_Store.GetRank(def, dto.rank_id);
                string assignedName = dto.rank_id;
                if (assignedRank) assignedName = assignedRank.name;
                Notify(player, "Звание назначено: " + assignedName + ".");
                PlayerBase rankedPlayer = FindPlayer(dto.target_uid);
                if (rankedPlayer) Notify(rankedPlayer, "Ваше звание изменено: " + assignedName + ".");
            }
            else
                Notify(player, "Звание игроку не назначено: проверьте выбранного игрока и звание.");
        }
        else if (command == TFLFactionCommand.ADMIN_SALARY_SETTINGS_SAVE)
        {
            if (!TFLFactionSalaryManager.Get().SaveSettingsFromCommand(dto))
                Notify(player, "Настройки выплат не сохранены. Проверьте банк, суммы, валюты и интервалы.");
        }
        else if (command == TFLFactionCommand.ADMIN_LICENSE_CREATE)
            TFLPassManager.Get().Licenses().Create(dto.license_id, dto.license_name, dto.description);
        else if (command == TFLFactionCommand.ADMIN_LICENSE_SAVE)
            TFLPassManager.Get().Licenses().SaveDefinition(dto.license_id, dto.license_name, dto.description, dto.int_value);
        else if (command == TFLFactionCommand.ADMIN_LICENSE_TOGGLE_FACTION)
            TFLPassManager.Get().Licenses().ToggleFaction(dto.license_id, dto.target_faction_id);
        else if (command == TFLFactionCommand.ADMIN_LICENSE_DELETE)
            TFLPassManager.Get().Licenses().Delete(dto.license_id);
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_CREATE)
        {
            string newTerritoryId = TFLFactionText.SafeId(dto.territory_id);
            if (newTerritoryId == "")
            {
                Notify(player, "Территория не создана: укажите корректный ID.");
            }
            else if (dto.territory_name == "")
            {
                Notify(player, "Территория не создана: укажите название.");
            }
            else if (TFLTerritoryManager.Get().Store().Get(newTerritoryId))
            {
                Notify(player, "Территория с ID " + newTerritoryId + " уже существует.");
            }
            else
            {
                vector createPos = Vector(dto.x, dto.y, dto.z);
                float terrainY = GetGame().SurfaceY(createPos[0], createPos[2]);
                float roadY = GetGame().SurfaceRoadY(createPos[0], createPos[2]);
                if (roadY > terrainY)
                    terrainY = roadY;
                createPos[1] = terrainY;
                if (TFLTerritoryManager.Get().Store().Create(newTerritoryId, dto.territory_name, createPos))
                    Notify(player, "Территория создана: " + dto.territory_name + " // " + newTerritoryId);
                else
                    Notify(player, "Территория не создана. Проверьте ID и точку на карте.");
            }
        }
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_SAVE)
        {
            if (!TFLTerritoryManager.Get().Store().SaveDefinition(dto))
                Notify(player, "Территория не сохранена: запись не найдена.");
        }
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_DELETE)
            TFLTerritoryManager.Get().DeleteTerritory(dto.territory_id);
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_REWARD_ADD)
        {
            if (!ValidInventoryClass(dto.class_name))
                Notify(player, "Classname награды не найден в конфиге.");
            else
                TFLTerritoryManager.Get().Store().AddReward(dto.territory_id, dto.class_name, dto.reward_count);
        }
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_REWARD_REMOVE)
            TFLTerritoryManager.Get().Store().RemoveReward(dto.territory_id, dto.int_value);
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_TOGGLE_DAY)
            TFLTerritoryManager.Get().Store().ToggleDay(dto.territory_id, dto.day_index);
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_TOGGLE_DIRECT)
            TFLTerritoryManager.Get().Store().ToggleDirectCapture(dto.territory_id);
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_SETTINGS_SAVE)
            TFLTerritoryManager.Get().Store().SaveDailyLimit(dto.daily_capture_limit);
        else if (command == TFLFactionCommand.ADMIN_CHAIN_CREATE)
        {
            if (!TFLTerritoryManager.Get().Store().CreateChain(dto.chain_id, dto.chain_name, dto.chain_side_a, dto.chain_side_b))
                Notify(player, "Цепочка не создана. Проверьте ID.");
        }
        else if (command == TFLFactionCommand.ADMIN_CHAIN_SAVE)
        {
            if (!TFLTerritoryManager.Get().Store().SaveChain(dto.chain_id, dto.chain_name, dto.chain_side_a, dto.chain_side_b))
                Notify(player, "Цепочка не сохранена.");
        }
        else if (command == TFLFactionCommand.ADMIN_CHAIN_DELETE)
        {
            if (!TFLTerritoryManager.Get().Store().DeleteChain(dto.chain_id))
                Notify(player, "Цепочка не удалена.");
        }
        else if (command == TFLFactionCommand.ADMIN_CHAIN_ADD_POINT)
        {
            if (!TFLTerritoryManager.Get().Store().ChainAddPoint(dto.chain_id, dto.target_territory_id))
                Notify(player, "Точка не добавлена в цепочку.");
        }
        else if (command == TFLFactionCommand.ADMIN_CHAIN_REMOVE_POINT)
        {
            if (!TFLTerritoryManager.Get().Store().ChainRemovePoint(dto.chain_id, dto.target_territory_id))
                Notify(player, "Точка не удалена из цепочки.");
        }
        else if (command == TFLFactionCommand.ADMIN_CHAIN_MOVE_POINT)
        {
            if (!TFLTerritoryManager.Get().Store().ChainMovePoint(dto.chain_id, dto.target_territory_id, dto.int_value))
                Notify(player, "Порядок точки не изменён.");
        }
        else if (command == TFLFactionCommand.ADMIN_REWARD_PRESET_CREATE)
        {
            if (!TFLTerritoryManager.Get().Store().CreateRewardPreset(dto.reward_preset_id, dto.reward_preset_name))
                Notify(player, "Пресет награды не создан. Проверьте ID.");
        }
        else if (command == TFLFactionCommand.ADMIN_REWARD_PRESET_SAVE)
        {
            if (!TFLTerritoryManager.Get().Store().SaveRewardPreset(dto.reward_preset_id, dto.reward_preset_name))
                Notify(player, "Пресет награды не сохранён.");
        }
        else if (command == TFLFactionCommand.ADMIN_REWARD_PRESET_DELETE)
        {
            if (!TFLTerritoryManager.Get().Store().DeleteRewardPreset(dto.reward_preset_id))
                Notify(player, "Пресет награды не удалён.");
        }
        else if (command == TFLFactionCommand.ADMIN_REWARD_PRESET_ADD_ITEM)
        {
            if (!ValidInventoryClass(dto.class_name))
                Notify(player, "Classname предмета не найден в конфиге.");
            else if (!TFLTerritoryManager.Get().Store().AddRewardPresetItem(dto.reward_preset_id, dto.class_name, dto.reward_count))
                Notify(player, "Предмет не добавлен в пресет.");
        }
        else if (command == TFLFactionCommand.ADMIN_REWARD_PRESET_REMOVE_ITEM)
        {
            if (!TFLTerritoryManager.Get().Store().RemoveRewardPresetItem(dto.reward_preset_id, dto.int_value))
                Notify(player, "Предмет не удалён из пресета.");
        }
        else if (command == TFLFactionCommand.ADMIN_TERRITORY_SET_REWARD_PRESET)
        {
            if (!TFLTerritoryManager.Get().Store().AssignRewardPreset(dto.territory_id, dto.reward_preset_id))
                Notify(player, "Пресет награды не назначен территории.");
        }

        string auditText = "cmd=" + command.ToString();
        auditText = auditText + " actor=" + uid;
        auditText = auditText + " faction=" + factionId;
        auditText = auditText + " target=" + dto.target_uid;
        auditText = auditText + " license=" + dto.license_id;
        auditText = auditText + " territory=" + dto.territory_id;
        Audit("ADMIN", auditText);

        SyncFactionFlags(factionId);
        BroadcastFaction(factionId);
        BroadcastAdminPanels(factionId);
        BroadcastTerritoryPanels();
        SendAdminPanel(player, factionId);
    }

    protected void CopyPresetFromPlayer(PlayerBase player, TFLFactionDefinition def)
    {
        if (!player || !def) return;
        def.spawn_preset.items.Clear();
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        for (int i = 0; i < items.Count(); i++)
        {
            EntityAI entity = items.Get(i);
            if (!entity || entity == player) continue;
            string type = entity.GetType();
            if (type != "" && def.spawn_preset.items.Find(type) == -1) def.spawn_preset.items.Insert(type);
        }
        def.revision = def.revision + 1;
    }

    void ApplySpawnPreset(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return;
        string factionId = m_Store.GetEffectiveFactionIdByUid(player.GetIdentity().GetPlainId());
        TFLFactionDefinition def = m_Store.GetDefinition(factionId);
        if (!def) return;

        TFLFactionSpawnDef chosenSpawn = null;
        if ((def.system_civilian == 1 || factionId == TFLFactionSystem.CIVILIAN_ID) && def.spawn_points && def.spawn_points.Count() > 0)
        {
            array<ref TFLFactionSpawnDef> activePoints = new array<ref TFLFactionSpawnDef>();
            for (int sp = 0; sp < def.spawn_points.Count(); sp++)
            {
                TFLFactionSpawnDef candidate = def.spawn_points.Get(sp);
                if (candidate && candidate.Active == 1) activePoints.Insert(candidate);
            }
            if (activePoints.Count() > 0) chosenSpawn = activePoints.Get(Math.RandomInt(0, activePoints.Count()));
        }
        else if (def.spawn && def.spawn.Active == 1)
        {
            chosenSpawn = def.spawn;
        }

        if (chosenSpawn)
        {
            vector pos = chosenSpawn.Pos();
            if (pos[0] != 0 || pos[2] != 0)
            {
                player.SetPosition(pos);
                player.SetOrientation(chosenSpawn.Rot());
            }
        }
        if (!def.spawn_preset) return;
        if (def.spawn_preset.clear_inventory == 1) player.RemoveAllItems();
        for (int i = 0; i < def.spawn_preset.items.Count(); i++)
        {
            string className = def.spawn_preset.items.Get(i);
            if (className != "") player.GetInventory().CreateInInventory(className);
        }
    }

    protected TFLFactionMemberDto MemberDto(TFLFactionMember member, TFLFactionDefinition def)
    {
        TFLFactionMemberDto dto = new TFLFactionMemberDto();
        dto.uid = member.steamid;
        TFLFactionPassportPersonDto person = m_Passports.Get(member.steamid);
        if (person) dto.name = person.name; else dto.name = member.nickname;
        TFLFactionRole role = m_Store.GetRole(def, member.role_id);
        dto.role_id = member.role_id;
        if (role) { dto.role_name = role.name; dto.role_level = role.level; }
        TFLFactionRank rank = m_Store.GetRank(def, member.rank_id);
        dto.rank_id = member.rank_id;
        if (rank)
        {
            dto.rank_name = rank.name;
            dto.rank_level = rank.level;
            dto.salary = rank.salary;
            dto.currency_id = TFLFactionCurrency.Normalize(rank.currency_id);
        }
        return dto;
    }

    protected TFLFactionRankDto RankDto(TFLFactionRank rank)
    {
        TFLFactionRankDto dto = new TFLFactionRankDto();
        dto.id = rank.id;
        dto.name = rank.name;
        dto.level = rank.level;
        dto.salary = rank.salary;
        dto.currency_id = TFLFactionCurrency.Normalize(rank.currency_id);
        return dto;
    }

    protected TFLFactionRoleDto RoleDto(TFLFactionRole role)
    {
        TFLFactionRoleDto dto = new TFLFactionRoleDto();
        dto.id = role.id; dto.name = role.name; dto.level = role.level;
        dto.permissions = role.permissions.Copy();
        return dto;
    }

    void SendLeaderPanel(PlayerBase player, string factionId)
    {
        if (!player || !player.GetIdentity()) return;
        string uid = player.GetIdentity().GetPlainId();
        TFLFactionDefinition def = m_Store.GetDefinition(factionId);
        TFLFactionMembersFile members = m_Store.GetMembers(factionId);
        if (!def || !members) return;
        TFLFactionLeaderPanelDto panel = new TFLFactionLeaderPanelDto();
        panel.faction_id = def.id; panel.faction_name = def.name; panel.faction_short_name = def.short_name;
        panel.side_id = def.side_id; panel.side_name = TFLFactionSide.Display(def.side_id); panel.revision = def.revision + members.revision;
        panel.is_leader = m_Store.IsLeaderUid(uid);
        panel.can_invite = HasPermission(uid, TFLFactionPerm.INVITE);
        panel.can_kick = HasPermission(uid, TFLFactionPerm.KICK);
        panel.can_promote = HasPermission(uid, TFLFactionPerm.PROMOTE);
        panel.can_demote = HasPermission(uid, TFLFactionPerm.DEMOTE);
        panel.can_manage_roles = HasPermission(uid, TFLFactionPerm.MANAGE_ROLES);
        panel.can_edit_role_permissions = HasPermission(uid, TFLFactionPerm.EDIT_ROLE_PERMISSIONS);
        panel.can_issue_licenses = HasPermission(uid, TFLFactionPerm.ISSUE_LICENSES);
        for (int i = 0; i < members.members.Count(); i++) if (members.members.Get(i)) panel.members.Insert(MemberDto(members.members.Get(i), def));
        for (int r = 0; r < def.roles.Count(); r++) if (def.roles.Get(r)) panel.roles.Insert(RoleDto(def.roles.Get(r)));
        if (panel.is_leader == 1 && def.side_id != TFLFactionSide.NEUTRAL) TFLTerritoryManager.Get().BuildAllDtos(uid, false, panel.territories);
        SendLeaderPanelDto(player, panel);
    }

    protected TFLFactionAdminPanelDto BuildAdminPanelDto(PlayerBase player, string requestedFaction)
    {
        if (!player || !player.GetIdentity()) return null;
        string uid = player.GetIdentity().GetPlainId();
        m_Passports.Reload();
        string selected = requestedFaction;
        if (selected == "" && m_OpenFaction.Contains(uid)) selected = m_OpenFaction.Get(uid);
        array<TFLFactionDefinition> defs;
        m_Store.GetDefinitions(defs);
        if (selected == "" && defs.Count() > 0) selected = defs.Get(0).id;
        if (selected != "") m_OpenFaction.Set(uid, selected);
        TFLFactionDefinition def = m_Store.GetDefinition(selected);
        TFLFactionAdminPanelDto panel = new TFLFactionAdminPanelDto();
        panel.selected_id = selected;
        if (def)
        {
            panel.selected_name = def.name; panel.selected_short_name = def.short_name; panel.selected_side_id = def.side_id; panel.selected_side_name = TFLFactionSide.Display(def.side_id); panel.revision = def.revision;
            panel.selected_system_civilian = def.system_civilian;
            if (def.system_civilian == 1 || def.id == TFLFactionSystem.CIVILIAN_ID)
            {
                panel.spawn_active = 0;
                if (def.spawn_points)
                {
                    for (int spi = 0; spi < def.spawn_points.Count(); spi++)
                    {
                        TFLFactionSpawnDef point = def.spawn_points.Get(spi);
                        if (!point) continue;
                        TFLFactionSpawnPointDto pointDto = new TFLFactionSpawnPointDto();
                        CopyFloatArray(point.Position, pointDto.position);
                        CopyFloatArray(point.Orientation, pointDto.orientation);
                        panel.spawn_points.Insert(pointDto);
                        if (point.Active == 1) panel.spawn_active = 1;
                    }
                    if (def.spawn_points.Count() > 0 && def.spawn_points.Get(def.spawn_points.Count() - 1))
                    {
                        CopyFloatArray(def.spawn_points.Get(def.spawn_points.Count() - 1).Position, panel.spawn_position);
                        CopyFloatArray(def.spawn_points.Get(def.spawn_points.Count() - 1).Orientation, panel.spawn_orientation);
                    }
                }
            }
            else
            {
                panel.spawn_active = def.spawn.Active;
                CopyFloatArray(def.spawn.Position, panel.spawn_position);
                CopyFloatArray(def.spawn.Orientation, panel.spawn_orientation);
            }
            panel.laptop_active = def.laptop.Active; CopyFloatArray(def.laptop.Position, panel.laptop_position); CopyFloatArray(def.laptop.Orientation, panel.laptop_orientation);
            panel.clear_inventory = def.spawn_preset.clear_inventory;
            for (int p = 0; p < def.spawn_preset.items.Count(); p++) panel.preset_items.Insert(def.spawn_preset.items.Get(p));
            for (int c = 0; c < def.reward_chests.Count(); c++)
            {
                TFLRewardChestDef chest = def.reward_chests.Get(c); if (!chest) continue;
                TFLRewardChestDto cd = new TFLRewardChestDto(); cd.id = chest.id; cd.class_name = chest.class_name; CopyFloatArray(chest.Position, cd.position); CopyFloatArray(chest.Orientation, cd.orientation); panel.reward_chests.Insert(cd);
            }
            TFLFactionMembersFile members = m_Store.GetMembers(def.id);
            if (def.system_civilian != 1 && members)
                for (int m = 0; m < members.members.Count(); m++) if (members.members.Get(m)) panel.members.Insert(MemberDto(members.members.Get(m), def));
            for (int r = 0; r < def.roles.Count(); r++) if (def.roles.Get(r)) panel.roles.Insert(RoleDto(def.roles.Get(r)));
            for (int rk = 0; rk < def.ranks.Count(); rk++) if (def.ranks.Get(rk)) panel.ranks.Insert(RankDto(def.ranks.Get(rk)));
        }
        TFLFactionSalaryManager.Get().FillSettingsDto(panel.salary_settings);
        for (int f = 0; f < defs.Count(); f++)
        {
            TFLFactionDefinition fd = defs.Get(f); if (!fd) continue;
            TFLFactionSummaryDto sum = new TFLFactionSummaryDto();
            sum.id = fd.id;
            sum.name = fd.name;
            sum.short_name = fd.short_name;
            sum.side_id = fd.side_id;
            sum.side_name = TFLFactionSide.Display(fd.side_id);
            sum.roles = fd.roles.Count();
            sum.system_civilian = fd.system_civilian;
            if (fd.spawn_points) sum.spawn_points = fd.spawn_points.Count();
            if (fd.system_civilian == 1 || fd.id == TFLFactionSystem.CIVILIAN_ID)
            {
                sum.members = 0;
                if (sum.spawn_points > 0) sum.spawn_active = 1; else sum.spawn_active = 0;
            }
            else
            {
                sum.spawn_active = fd.spawn.Active;
                TFLFactionMembersFile fm = m_Store.GetMembers(fd.id); if (fm) sum.members = fm.members.Count();
            }
            sum.laptop_active = fd.laptop.Active;
            panel.factions.Insert(sum);
        }
        if (!def || def.system_civilian != 1)
        {
            array<ref TFLFactionPassportPersonDto> people;
            m_Passports.GetAll(people);
            for (int pp = 0; pp < people.Count(); pp++) if (people.Get(pp) && m_Store.GetFactionIdByUid(people.Get(pp).uid) == "") panel.people.Insert(people.Get(pp));
        }
        array<TFLLicenseDefinition> licenses;
        TFLPassManager.Get().Licenses().GetAll(licenses);
        for (int l = 0; l < licenses.Count(); l++)
        {
            TFLLicenseDefinition ld = licenses.Get(l); if (!ld) continue;
            TFLLicenseAdminDto outLic = new TFLLicenseAdminDto(); outLic.id = ld.id; outLic.name = ld.name; outLic.description = ld.description; outLic.active = ld.active;
            for (int lf = 0; lf < ld.issuer_factions.Count(); lf++) outLic.issuer_factions.Insert(ld.issuer_factions.Get(lf));
            panel.licenses.Insert(outLic);
        }
        TFLTerritoryManager.Get().BuildAllDtos(uid, true, panel.territories);

        array<TFLTerritoryRewardPresetDef> rewardPresets;
        TFLTerritoryManager.Get().Store().GetRewardPresets(rewardPresets);
        for (int rp = 0; rp < rewardPresets.Count(); rp++)
        {
            TFLTerritoryRewardPresetDef presetDef = rewardPresets.Get(rp);
            if (!presetDef) continue;
            TFLTerritoryRewardPresetDto presetDto = new TFLTerritoryRewardPresetDto();
            presetDto.id = presetDef.id;
            presetDto.name = presetDef.name;
            for (int ri = 0; ri < presetDef.items.Count(); ri++)
            {
                TFLTerritoryRewardDef itemDef = presetDef.items.Get(ri);
                if (!itemDef) continue;
                TFLTerritoryRewardDto itemDto = new TFLTerritoryRewardDto();
                itemDto.class_name = itemDef.class_name;
                itemDto.count = itemDef.count;
                presetDto.items.Insert(itemDto);
            }
            panel.reward_presets.Insert(presetDto);
        }

        array<TFLTerritoryChainDefinition> chains;
        TFLTerritoryManager.Get().Store().GetChains(chains);
        for (int ch = 0; ch < chains.Count(); ch++)
        {
            TFLTerritoryChainDefinition chainDef = chains.Get(ch);
            if (!chainDef) continue;
            TFLTerritoryChainDto chainDto = new TFLTerritoryChainDto();
            chainDto.id = chainDef.id;
            chainDto.name = chainDef.name;
            chainDto.side_a = chainDef.side_a;
            chainDto.side_a_name = TFLFactionSide.Display(chainDef.side_a);
            chainDto.side_b = chainDef.side_b;
            chainDto.side_b_name = TFLFactionSide.Display(chainDef.side_b);
            for (int cp = 0; cp < chainDef.territory_ids.Count(); cp++)
            {
                string chainTerritoryId = chainDef.territory_ids.Get(cp);
                chainDto.territory_ids.Insert(chainTerritoryId);
                TFLTerritoryDefinition chainTerritory = TFLTerritoryManager.Get().Store().Get(chainTerritoryId);
                if (chainTerritory) chainDto.territory_names.Insert(chainTerritory.name);
                else chainDto.territory_names.Insert(chainTerritoryId);
            }
            panel.territory_chains.Insert(chainDto);
        }

        TFLTerritorySettings ts = TFLTerritoryManager.Get().Store().Settings(); if (ts) { panel.territory_settings.daily_capture_limit = ts.daily_capture_limit; panel.territory_settings.revision = ts.revision; }
        return panel;
    
    }

    TFLFactionAdminPanelDto BuildVPPAdminPanel(PlayerBase player, string requestedFaction)
    {
        return BuildAdminPanelDto(player, requestedFaction);
    }

    void HandleVPPAdminCommand(PlayerBase player, int command, TFLFactionCommandDto dto)
    {
        if (!player || !player.GetIdentity()) return;
        if (command < TFLFactionCommand.ADMIN_SELECT || command > TFLFactionCommand.ADMIN_TERRITORY_SET_REWARD_PRESET) return;
        ref TFLFactionCommandDto safeDto = dto;
        if (!safeDto) safeDto = new TFLFactionCommandDto();
        HandleAdminCommand(player, player.GetIdentity().GetPlainId(), command, safeDto);
    }

    void SendAdminPanel(PlayerBase player, string requestedFaction)
    {
        if (!player || !player.GetIdentity()) return;
        string uid = player.GetIdentity().GetPlainId();
        if (!m_Store.IsAdmin(uid)) return;
        TFLFactionAdminPanelDto panel = BuildAdminPanelDto(player, requestedFaction);
        if (panel) SendAdminPanelDto(player, panel);
    }

    protected void CopyFloatArray(array<float> source, array<float> target)
    {
        if (!source || !target) return;
        for (int i = 0; i < source.Count(); i++) target.Insert(source.Get(i));
    }

    protected void SyncOnlineByUid(string uid)
    {
        PlayerBase p = FindPlayer(uid); if (p) SyncPlayerFlags(p);
    }

    protected void SyncFactionFlags(string factionId)
    {
        TFLFactionMembersFile list = m_Store.GetMembers(factionId); if (!list) return;
        for (int i = 0; i < list.members.Count(); i++) if (list.members.Get(i)) SyncOnlineByUid(list.members.Get(i).steamid);
    }

    void BroadcastFaction(string factionId)
    {
        TFLFactionMembersFile list = m_Store.GetMembers(factionId); if (!list) return;
        for (int i = 0; i < list.members.Count(); i++)
        {
            TFLFactionMember member = list.members.Get(i); if (!member) continue;
            if (!m_OpenMode.Contains(member.steamid) || m_OpenMode.Get(member.steamid) != MODE_LEADER) continue;
            PlayerBase player = FindPlayer(member.steamid);
            if (player && ValidLaptopSession(player, MODE_LEADER)) SendLeaderPanel(player, factionId);
        }
    }

    void BroadcastAdminPanels(string selectedFaction)
    {
        array<Man> players = new array<Man>(); GetGame().GetPlayers(players);
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i)); if (!player || !player.GetIdentity()) continue;
            string uid = player.GetIdentity().GetPlainId();
            if (!m_Store.IsAdmin(uid)) continue;
            if (!m_OpenMode.Contains(uid) || m_OpenMode.Get(uid) != MODE_ADMIN) continue;
            if (!ValidLaptopSession(player, MODE_ADMIN)) continue;
            SendAdminPanel(player, selectedFaction);
        }
    }

    void BroadcastTerritoryPanels()
    {
        array<Man> players = new array<Man>(); GetGame().GetPlayers(players);
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i)); if (!player || !player.GetIdentity()) continue;
            string uid = player.GetIdentity().GetPlainId();
            if (m_OpenMode.Contains(uid) && m_OpenMode.Get(uid) == MODE_ADMIN && m_Store.IsAdmin(uid) && ValidLaptopSession(player, MODE_ADMIN)) SendAdminPanel(player, "");
            else if (m_OpenMode.Contains(uid) && m_OpenMode.Get(uid) == MODE_LEADER && m_Store.IsLeaderUid(uid) && ValidLaptopSession(player, MODE_LEADER)) SendLeaderPanel(player, m_Store.GetFactionIdByUid(uid));
        }
    }

    void SendTerritoryHudToFactions(array<string> factionIds, TFLTerritoryHudDto hud)
    {
        if (!factionIds || !hud) return;
        for (int i = 0; i < factionIds.Count(); i++) SendTerritoryHudToFaction(factionIds.Get(i), hud);
    }

    protected void SendTerritoryHudToFaction(string factionId, TFLTerritoryHudDto hud)
    {
        TFLFactionMembersFile list = m_Store.GetMembers(factionId); if (!list) return;
        for (int i = 0; i < list.members.Count(); i++)
        {
            TFLFactionMember member = list.members.Get(i); if (!member) continue;
            PlayerBase player = FindPlayer(member.steamid); if (player) SendTerritoryHudDto(player, hud);
        }
    }

    void Notify(PlayerBase player, string text)
    {
        if (!player || !player.GetIdentity()) return;
        GetGame().RPCSingleParam(player, TFLFactionRPC.NOTIFY, new Param1<string>(text), true, player.GetIdentity());
    }

    void NotifyUid(string uid, string text)
    {
        PlayerBase player = FindPlayer(uid); if (player) Notify(player, text);
    }

    void NotifyFaction(string factionId, string text)
    {
        TFLFactionMembersFile list = m_Store.GetMembers(factionId); if (!list) return;
        for (int i = 0; i < list.members.Count(); i++) if (list.members.Get(i)) NotifyUid(list.members.Get(i).steamid, text);
    }

    void NotifyFactions(array<string> factionIds, string text)
    {
        if (!factionIds) return;
        for (int i = 0; i < factionIds.Count(); i++) NotifyFaction(factionIds.Get(i), text);
    }

    void GiveTerritoryReward(string factionId, array<ref TFLTerritoryRewardDef> rewards, string territoryName)
    {
        if (!rewards) return;
        Object chestObj = null;
        for (int i = 0; i < m_RewardChestFactionIds.Count(); i++)
        {
            if (m_RewardChestFactionIds.Get(i) == factionId) { chestObj = m_RewardChests.Get(i); break; }
        }
        EntityAI chest = EntityAI.Cast(chestObj);
        if (!chest) { TFLFactionLog.Error("Нет наградного ящика для " + factionId + " // " + territoryName); return; }
        for (int r = 0; r < rewards.Count(); r++)
        {
            TFLTerritoryRewardDef reward = rewards.Get(r); if (!reward || reward.class_name == "") continue;
            for (int n = 0; n < reward.count; n++) chest.GetInventory().CreateInInventory(reward.class_name);
        }
        NotifyFaction(factionId, "Награда за " + territoryName + " доставлена в фракционный ящик.");
    }

    protected string AuditStamp()
    {
        int year; int month; int day;
        int hour; int minute; int second;
        GetYearMonthDayUTC(year, month, day);
        GetHourMinuteSecondUTC(hour, minute, second);
        string stamp = year.ToString() + "-" + TFLPassText.Pad2(month) + "-" + TFLPassText.Pad2(day);
        stamp = stamp + " " + TFLPassText.Pad2(hour) + ":" + TFLPassText.Pad2(minute) + ":" + TFLPassText.Pad2(second) + "Z";
        return stamp;
    }

    void Audit(string category, string text)
    {
        MakeDirectory("$profile:TFL/Logs");
        string path = "$profile:TFL/Logs/" + category + ".log";
        FileHandle file = OpenFile(path, FileMode.APPEND);
        if (!file) return;
        FPrintln(file, AuditStamp() + " // " + text);
        CloseFile(file);
    }

    protected void SendChunked(PlayerBase player, int kind, string payload)
    {
        if (!player || !player.GetIdentity() || payload == "") return;
        int size = TFLPassLimits.CHUNK; int len = payload.Length(); int total = len / size; if ((len % size) != 0) total = total + 1; if (total <= 0) total = 1;
        for (int i = 0; i < total; i++)
        {
            int from = i * size; int take = size; if (from + take > len) take = len - from;
            GetGame().RPCSingleParam(player, TFLFactionRPC.DATA, new Param4<int,int,int,string>(kind, i, total, payload.Substring(from, take)), true, player.GetIdentity());
        }
    }

    protected void SendInviteDto(PlayerBase player, TFLFactionInviteDto dto) { string s; string e; if (JsonFileLoader<TFLFactionInviteDto>.MakeData(dto,s,e,false)) SendChunked(player,TFLFactionDataKind.INVITE,s); }
    protected void SendLeaderPanelDto(PlayerBase player, TFLFactionLeaderPanelDto dto) { string s; string e; if (JsonFileLoader<TFLFactionLeaderPanelDto>.MakeData(dto,s,e,false)) SendChunked(player,TFLFactionDataKind.LEADER_PANEL,s); }
    protected void SendAdminPanelDto(PlayerBase player, TFLFactionAdminPanelDto dto) { string s; string e; if (JsonFileLoader<TFLFactionAdminPanelDto>.MakeData(dto,s,e,false)) SendChunked(player,TFLFactionDataKind.ADMIN_PANEL,s); }
    protected void SendTerritoryHudDto(PlayerBase player, TFLTerritoryHudDto dto) { string s; string e; if (JsonFileLoader<TFLTerritoryHudDto>.MakeData(dto,s,e,false)) SendChunked(player,TFLFactionDataKind.TERRITORY_HUD,s); }
}
