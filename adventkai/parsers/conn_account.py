from adventkai.exceptions import CommandError
from .conn_base import ConnParser
from enum import IntEnum
import adventkai
from adventkai import components as cm
from mudforge.net.basic import DisconnectReason
from mudforge import GAME


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

    def get_characters(self):
        entities = list()
        for c in self.account.characters.all():
            m = adventkai.MODULES[c.prototype.module.name]
            ent = m.entities[c.ent_id]
            entities.append(ent)
        return entities

    async def display_user_menu(self):
        self.send_circle("                 @RUser Menu@n");
        self.send_circle("@D=============================================@n")
        self.send_circle(f"@D|@gUser Account  @D: @w{self.account.username:<27}@D|@n")
        self.send_circle(f"@D|@gEmail Address @D: @w{self.account.email.replace('@', '@@'):<27}@D|@n")
        self.send_circle(f"@D|@gMax Characters@D: @w{self.account.max_slots:<27}@D|@n")
        self.send_circle(f"@D|@gRP Points     @D: @w{self.account.rpp_current:<27}@D|@n")
        self.send_circle("@D=============================================@n")
        self.send_circle("\r\n      @D[@y----@YSelect A Character Slot@y----@D]@n")

        count = 0
        characters = self.get_characters()

        for c in characters:
            count += 1
            n = adventkai.WORLD.component_for_entity(c, cm.Name)
            self.send_circle(f"                @B(@W{count}@B) @C{n.color}")

        diff = self.account.max_slots - len(characters)
        if diff > 0:
            for i in range(diff):
                count += 1
                self.send_circle(f"                @B(@W{count}@B) @C@n\n")

        self.send_circle("\r\n      @D[@y---- @YSelect Another Option @y----@D]@n")
        self.send_circle("                @B(@WB@B) @CBuy New C. Slot @D(@R15 RPP@D)@n")
        self.send_circle("                @B(@WC@B) @CUser's Customs")
        self.send_circle("                @B(@WD@B) @RDelete User@n")
        self.send_circle("                @B(@WE@B) @CEmail@n")
        self.send_circle("                @B(@WP@B) @CNew Password@n")
        self.send_circle("                @B(@WQ@B) @CQuit@n")
        self.send_circle("\r\nMake your choice:")


    async def start(self):
        self.state = State.MAIN
        await self.display_user_menu()

    async def parse(self, s: str):
        await getattr(self, f"parse_{self.state.name.lower()}")(s)

    async def parse_main(self, s):
        match s.upper():
            case "Q":
                self.send("Thanks for visiting!")
                GAME.pending_disconnections[self.conn.conn_id] = DisconnectReason.QUIT
            case "P":
                self.send("Enter new Password, or M for menu:")
                self.state = State.PASSWORD
            case "C":
                self.send("Not implemented yet.")
            case "E":
                self.send("Enter New Email, or M for menu:")
                self.state = State.EMAIL
            case "D":
                self.send("Are you sure you want to delete your user file and all its characters? Yes or No:")
                self.state = State.DELETE
            case "B":
                if self.account.max_slots >= 5:
                    await self.display_user_menu()
                    self.send("You are at the maximum amount of character slots.")
                    return
                elif self.account.rpp_current < 15:
                    await self.display_user_menu()
                    self.send("You cannot afford the 15 RPP required.")
                    return
                self.account.rpp_current -= 15
                self.account.max_slots += 1
                self.account.save()

            case _:
                if s.isdigit():
                    choice = int(s)
                    if choice < 1 or choice > self.account.max_slots:
                        await self.display_user_menu()
                        self.send("Unrecognized character slot.")
                        return
                    characters = self.get_characters()
                    if choice-1 <= len(characters):
                        c = characters[choice-1]
                        await self.conn.select_character(c)
                        return
                    await self.conn.enter_chargen()
                    return

                else:
                    await self.display_user_menu()
                    self.send("Unrecognized input.")

    async def parse_password(self, s):
        pass

    async def parse_confpass(self, s):
        pass

    async def parse_name(self, s):
        pass

    async def parse_confname(self, s):
        pass

    async def parse_email(self, s):
        pass

    async def parse_confemail(self, s):
        pass

    async def parse_delete(self, s):
        pass

    async def parse_confdelete(self, s):
        pass