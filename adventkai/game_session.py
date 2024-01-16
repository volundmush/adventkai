import asyncio
from dataclasses import dataclass, field
from rich.color import ColorType
from rich.abc import RichRenderable


@dataclass
class Capabilities:
    protocol: str = "telnet"
    session_name: str = ""
    encryption: bool = False
    client_name: str = "UNKNOWN"
    client_version: str = "UNKNOWN"
    host_address: str = "UNKNOWN"
    host_port: int = -1
    host_names: list[str, ...] = None
    encoding: str = "ascii"
    color: ColorType = ColorType.DEFAULT
    width: int = 78
    height: int = 24
    mccp2: bool = False
    mccp2_enabled: bool = False
    mccp3: bool = False
    mccp3_enabled: bool = False
    gmcp: bool = False
    msdp: bool = False
    mssp: bool = False
    mslp: bool = False
    mtts: bool = False
    naws: bool = False
    sga: bool = False
    linemode: bool = False
    force_endline: bool = False
    screen_reader: bool = False
    mouse_tracking: bool = False
    vt100: bool = False
    osc_color_palette: bool = False
    proxy: bool = False
    mnes: bool = False

    def display_client_name(self):
        if self.client_version != "UNKNOWN":
            return f"{self.client_name} (v {self.client_version})"
        return self.client_name


@dataclass
class ClientHello:
    userdata: dict[str, "Any"] = field(default_factory=dict)
    capabilities: Capabilities = field(default_factory=Capabilities)


@dataclass
class ClientCommand:
    text: str = ""

    async def at_server_receive(self, session):
        await session.handle_command(self.text)


@dataclass
class ClientGMCP:
    cmd: str
    data: "Any"

    async def at_server_receive(self, session):
        await session.receive_gmcp(self.cmd, self.data)


@dataclass
class ClientUpdate:
    capabilities: dict[str, "Any"] = field(default_factory=dict)

    async def at_server_receive(self, session):
        await session.change_capabilities(self.capabilities)


@dataclass
class ClientDisconnect:

    async def at_server_receive(self, session):
        pass


@dataclass
class ServerUserdata:
    userdata: dict[str, "Any"] = field(default_factory=dict)

    async def at_portal_receive(self, session):
        session.userdata = self.userdata


@dataclass
class ServerDisconnect:
    async def at_portal_receive(self, session):
        await session.close()


@dataclass
class ServerMSSP:
    data: dict[str, str] = field(default_factory=dict)

    async def at_portal_receive(self, session):
        await session.send_mssp(self.data)


@dataclass
class ServerText:
    data: str

    async def at_portal_receive(self, session):
        await session.send_game_text(self.data)


@dataclass
class ServerGMCP:
    cmd: str
    data: "Any"

    async def at_portal_receive(self, session):
        await session.send_gmcp(self.cmd, self.data)


class GameSession:
    def __init__(self):
        self.capabilities = Capabilities()
        self.task_group = asyncio.TaskGroup()
        self.tasks: dict[str, asyncio.Task] = {}
        self.running = True
        # This contains arbitrary data sent by the server which will be sent on a reconnect.
        self.userdata = None
        self.outgoing_queue = asyncio.Queue()
        self.core = None

    async def run(self):
        pass

    async def start(self):
        pass

    async def change_capabilities(self, changed: dict[str, "Any"]):
        pass

    async def at_capability_change(self, capability: str, value):
        pass
