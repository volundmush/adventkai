from adventkai.db.accounts.models import Account
from enum import IntEnum
from django.core.exceptions import ValidationError
from adventkai.exceptions import CommandError
from .conn_base import ConnParser
from django.core.validators import EmailValidator
import adventkai

_email_valid = EmailValidator()

class LoginState(IntEnum):
    NAME = 0
    CONFNAME = 1
    PASSWORD = 2
    CONFPASS = 3
    EMAIL = 4
    CONFEMAIL = 5


class LoginParser(ConnParser):

    def __init__(self, conn):
        super().__init__(conn)
        self.user_data = {}
        self.state = LoginState.NAME
        self.account = None

        self.state_map = {
            LoginState.NAME: self.parse_name,
            LoginState.CONFNAME: self.parse_confname,
            LoginState.PASSWORD: self.parse_password,
            LoginState.CONFPASS: self.parse_confpass,
            LoginState.EMAIL: self.parse_email,
            LoginState.CONFEMAIL: self.parse_confemail
        }

    async def start(self):
        await self.send_circle(adventkai.TEXT_FILES["greetansi"])
        await self.send_circle("\r\n@cEnter your desired username or the username you have already made.\r\n@CEnter Username:")
        self.user_data.clear()
        self.account = None
        self.state = LoginState.NAME

    async def parse(self, s: str):
        if s.lower == "return":
            await self.start()
            return

        try:
            await self.state_map[self.state](s)
        except CommandError as err:
            await self.send(str(err))
        except ValidationError as err:
            await self.send(str(err))

    async def parse_name(self, s):
        Account.username_validator(s)

        if account := Account.objects.filter(username__iexact=s).first():
            self.account = account
            self.user_data.clear()
            self.state = LoginState.PASSWORD
            await self.send("Please enter password or 'return' to cancel.")
            await self.send("Password:")
        else:
            self.user_data["username"] = s
            self.state = LoginState.CONFNAME
            await self.send(f"Please confirm username ({s}):")

    async def parse_confname(self, s):
        try:
            if s != self.user_data["username"]:
                raise CommandError("Usernames do not match!")

            self.state = LoginState.PASSWORD
            await self.send("Please enter password or 'return' to cancel.")
            await self.send("Password:")
        except CommandError as err:
            await self.send(str(err))
            self.user_data.clear()
            self.state = LoginState.NAME
            await self.send("Enter Username:")

    async def parse_password(self, s):
        if self.account:
            if self.account.check_password(s):
                await self.conn.login_as(self.account)
                return
            else:
                await self.send("That was not the correct password.")
                await self.send("Password:")
                return
        else:
            if not s:
                await self.send("Please enter a password.")
                await self.send("Password:")
                return
            self.user_data["password"] = s
            self.state = LoginState.CONFPASS
            await self.send("Got it. Please enter password again to verify:")


    async def parse_confpass(self, s):
        if s == self.user_data["password"]:
            self.state = LoginState.EMAIL
            await self.send("ALmost done! Now please provide an email address. Or, type 'return' to cancel.")
            await self.send("Email:")
            return

        await self.send("Passwords did not match. Let's try that again.")
        await self.send("Password:")
        self.state = LoginState.PASSWORD


    async def parse_email(self, s):
        try:
            _email_valid(s)
            self.user_data["email"] = s
            self.state = LoginState.CONFEMAIL
            await self.send("One last time, please verify that email address for us.")
            await self.send("Email:")
        except ValidationError as err:
            await self.send(str(err))
            await self.send("That doesn't look like an email address. Please try again.")
            await self.send("Email:")

    async def parse_confemail(self, s):
        if s != self.user_data["email"]:
            await self.send("Those emails didn't match. Let's try again.")
            await self.send("Email:")
            self.state = LoginState.EMAIL
            return

        acc = Account.objects.create_user(**self.user_data)
        await self.conn.login_as(acc)