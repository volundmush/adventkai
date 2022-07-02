from adventkai.exceptions import CommandError
from .conn_base import ConnParser
from enum import IntEnum


class State(IntEnum):
    MAIN = 0
    PASSWORD = 1
    CONFPASS = 2
    NAME = 3
    CONFNAME = 4
    EMAIL = 5
    CONFEMAIL = 6
    DELETE = 7
    CONFDELETE = 8


class AccountMenu(ConnParser):

    def __init__(self, conn):
        super().__init__(conn)
        self.account = conn.account

    async def display_user_menu(self):
        await self.send("USER MENU HERE!")

    async def start(self):
        self.state = State.MAIN
        await self.display_user_menu()

