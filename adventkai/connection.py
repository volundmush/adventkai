from mudforge.net.game_conn import GameConnection as OldConn
from adventkai.parsers.conn_login import LoginParser
from adventkai.parsers.conn_account import AccountMenu


class GameConnection(OldConn):

    def __init__(self, conn):
        super().__init__(conn)

        self.account = None
        self.parser = None
        self.session = None

    async def start(self):
        await self.on_start()

    async def on_start(self):
        await self.set_parser(LoginParser(self))

    async def set_parser(self, new_parser):
        if self.parser:
            await self.parser.close()
        self.parser = new_parser
        await self.parser.start()

    async def process_input_text(self, data: str):
        if self.parser:
            await self.parser.parse(data)

    async def login_as(self, account):
        self.account = account
        await self.set_parser(AccountMenu(self))