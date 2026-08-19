class TFLVPPAdminPlugin extends PluginBase
{
    protected static ref TFLVPPAdminPlugin s_Instance;
    protected bool m_RPCBound;

    static TFLVPPAdminPlugin Ensure()
    {
        if (!s_Instance)
            s_Instance = new TFLVPPAdminPlugin();
        return s_Instance;
    }

    protected static const string PERMISSION_MENU = "MenuTFLAdmin";
    protected static const string PERMISSION_WRITE = "MenuTFLAdmin:Write";

    protected bool m_PermissionsRegistered;

    void TFLVPPAdminPlugin()
    {
        s_Instance = this;
        RegisterPermissions();
        BindRPC();
    }

    // VerifyPermission первым делом отклоняет имя права, которого нет в пуле
    // VPPAdminTools, — причём для всех, включая Super Admin. Если плагин
    // создался раньше, чем PermissionManager, регистрация в конструкторе тихо
    // пропускалась, и оба права оставались неизвестными: панель не открывалась
    // ни у кого, а MenuTFLAdmin:Write вдобавок негде было назначить, потому что
    // в редакторе прав он не появлялся. Поэтому регистрируем лениво и
    // повторяем попытку, пока менеджер не появится.
    protected void RegisterPermissions()
    {
        if (m_PermissionsRegistered)
            return;
        PermissionManager manager = GetPermissionManager();
        if (!manager)
            return;

        ref array<string> permissions = new array<string>;
        permissions.Insert(PERMISSION_MENU);
        permissions.Insert(PERMISSION_WRITE);
        manager.AddPermissionType(permissions);
        m_PermissionsRegistered = true;
        Print("[TFL/VPP] Permissions registered: " + PERMISSION_MENU + ", " + PERMISSION_WRITE);
    }

    protected void BindRPC()
    {
        if (m_RPCBound)
            return;
        m_RPCBound = true;
        GetRPCManager().AddRPC("RPC_TFLVPPAdmin", "RequestPanel", this, SingeplayerExecutionType.Server);
        GetRPCManager().AddRPC("RPC_TFLVPPAdmin", "Command", this, SingeplayerExecutionType.Server);
        Print("[TFL/VPP] Admin RPC backend registered through CF RPCManager.");
    }

    protected bool HasPermission(PlayerIdentity sender, string permission)
    {
        if (!sender)
            return false;
        RegisterPermissions();
        PermissionManager manager = GetPermissionManager();
        if (!manager)
            return false;
        return manager.VerifyPermission(sender.GetPlainId(), permission, "", false);
    }

    // Super Admin определяем родным API VPPAdminTools: PermissionManager хранит
    // список Steam64 из SuperAdmins.txt, а GetPlainId() как раз и даёт Steam64.
    // Личность берём из самого RPC на сервере — клиент о своих правах не
    // сообщает и повлиять на решение не может.
    protected bool IsVPPSuperAdmin(PlayerIdentity sender)
    {
        if (!sender)
            return false;
        PermissionManager manager = GetPermissionManager();
        if (!manager)
            return false;
        return manager.IsSuperAdmin(sender.GetPlainId());
    }

    // ЕДИНАЯ проверка чтения: открыть панель и листать вкладки.
    protected bool TFL_CanAdminRead(PlayerIdentity sender)
    {
        if (IsVPPSuperAdmin(sender))
            return true;
        if (HasPermission(sender, PERMISSION_MENU))
            return true;
        return HasPermission(sender, PERMISSION_WRITE);
    }

    // ЕДИНАЯ проверка записи: любое создание, изменение и удаление данных TFL.
    // Через неё проходят все write-команды без исключения — фракции, состав,
    // роли, звания и пособия, лицензии, территории, цепочки и пресеты наград.
    protected bool TFL_CanAdminWrite(PlayerIdentity sender)
    {
        if (IsVPPSuperAdmin(sender))
            return true;
        return HasPermission(sender, PERMISSION_WRITE);
    }

    protected PlayerBase FindPlayer(PlayerIdentity sender)
    {
        if (!sender)
            return null;
        return TFLFactionManager.Get().FindPlayer(sender.GetPlainId());
    }

    protected void SendClient(PlayerIdentity recipient, string functionName, Param payload)
    {
        if (!recipient)
            return;
        GetRPCManager().SendRPC("RPC_TFLVPPAdminClient", functionName, payload, true, recipient);
    }

    protected void Message(PlayerIdentity recipient, string text)
    {
        SendClient(recipient, "OnVPPMessage", new Param1<string>(text));
    }

    protected void SendPanel(PlayerIdentity recipient, PlayerBase player, string selectedFaction)
    {
        if (!recipient || !player)
            return;
        TFLFactionAdminPanelDto panel = TFLFactionManager.Get().BuildVPPAdminPanel(player, selectedFaction);
        if (!panel)
        {
            Message(recipient, "TFL: не удалось сформировать административную панель.");
            return;
        }
        Print("[TFL/VPP] Panel ready // factions=" + panel.factions.Count().ToString() + " // licenses=" + panel.licenses.Count().ToString() + " // territories=" + panel.territories.Count().ToString());
        string payload;
        string err;
        if (!JsonFileLoader<TFLFactionAdminPanelDto>.MakeData(panel, payload, err, false))
        {
            Message(recipient, "TFL: ошибка сериализации административной панели.");
            Print("[TFL/VPP] panel serialization failed: " + err);
            return;
        }
        int chunkSize = TFLPassLimits.CHUNK;
        int len = payload.Length();
        int total = len / chunkSize;
        if ((len % chunkSize) != 0) total = total + 1;
        if (total <= 0) total = 1;
        for (int i = 0; i < total; i++)
        {
            int from = i * chunkSize;
            int take = chunkSize;
            if (from + take > len) take = len - from;
            SendClient(recipient, "OnVPPPanelChunk", new Param3<int, int, string>(i, total, payload.Substring(from, take)));
        }
    }

    void RequestPanel(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender)
            return;
        if (!TFL_CanAdminRead(sender))
        {
            Message(sender, "TFL: нет доступа к TFL Admin — нужно право MenuTFLAdmin в VPPAdminTools.");
            Print("[TFL/VPP] RequestPanel denied for " + sender.GetPlainId());
            return;
        }
        Param1<string> request;
        if (!ctx.Read(request) || !request)
            request = new Param1<string>("");
        PlayerBase player = FindPlayer(sender);
        if (!player)
            return;
        Print("[TFL/VPP] RequestPanel from " + sender.GetPlainId() + " // faction=" + request.param1);
        SendPanel(sender, player, request.param1);
    }

    // Команда приходит кусками (см. SendCommand на клиенте): целиком её JSON не
    // помещается в строку RPC-параметра и портится по дороге. Копим куски по
    // отправителю и выполняем, когда пришёл последний.
    protected ref map<string, string> m_CommandBuffer = new map<string, string>;

    void Command(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender)
            return;
        Param4<int, int, int, string> packet;
        if (!ctx.Read(packet) || !packet)
            return;

        int command = packet.param1;
        int index = packet.param2;
        int total = packet.param3;

        if (command < TFLFactionCommand.ADMIN_SELECT || command > TFLFactionCommand.ADMIN_TERRITORY_SET_REWARD_PRESET)
            return;
        if (total <= 0 || index < 0 || index >= total)
            return;

        // Права проверяем на первом куске: иначе чужой клиент мог бы копить
        // в буфере сервера что угодно, ни разу не пройдя проверку.
        if (index == 0)
        {
            if (command == TFLFactionCommand.ADMIN_SELECT)
            {
                if (!TFL_CanAdminRead(sender)) return;
            }
            else if (!TFL_CanAdminWrite(sender))
            {
                Message(sender, "TFL: нет прав на изменение — нужно право MenuTFLAdmin:Write в VPPAdminTools (Super Admin имеет его автоматически).");
                return;
            }
        }

        string senderId = sender.GetPlainId();
        string buffered = "";
        if (index == 0)
            m_CommandBuffer.Set(senderId, packet.param4);
        else
        {
            if (!m_CommandBuffer.Find(senderId, buffered))
                return;
            m_CommandBuffer.Set(senderId, buffered + packet.param4);
        }

        if (index < total - 1)
            return;

        string assembled = "";
        m_CommandBuffer.Find(senderId, assembled);
        m_CommandBuffer.Remove(senderId);

        // Повторная проверка перед самим выполнением: между первым и последним
        // куском права могли отозвать.
        if (command == TFLFactionCommand.ADMIN_SELECT)
        {
            if (!TFL_CanAdminRead(sender)) return;
        }
        else if (!TFL_CanAdminWrite(sender))
        {
            Message(sender, "TFL: нет прав на изменение — нужно право MenuTFLAdmin:Write в VPPAdminTools (Super Admin имеет его автоматически).");
            return;
        }

        ref TFLFactionCommandDto dto = new TFLFactionCommandDto();
        if (assembled != "")
        {
            TFLFactionCommandDto loaded;
            string err;
            if (!JsonFileLoader<TFLFactionCommandDto>.LoadData(assembled, loaded, err) || !loaded)
            {
                Message(sender, "TFL: команда не разобрана.");
                Print("[TFL/VPP] command parse failed: " + err);
                return;
            }
            dto = loaded;
        }

        PlayerBase player = FindPlayer(sender);
        if (!player)
            return;

        TFLFactionManager.Get().HandleVPPAdminCommand(player, command, dto);
        SendPanel(sender, player, "");
    }
}
