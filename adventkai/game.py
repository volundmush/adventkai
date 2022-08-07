import logging
from pathlib import Path
from snekmud.db.accounts.models import Account
from snekmud.game import GameService as OldGame
import snekmud

class GameService(OldGame):

    async def at_post_module_initial_load(self):
        legacy = snekmud.MODULES["legacy"]

        if not Account.objects.count():
            logging.info("loading legacy player data...")
            await legacy.load_userdata()

        print(f"CMDHANDLERS: {snekmud.CMDHANDLERS}")
        print(f"COMMANDS: {snekmud.COMMANDS}")

    async def game_loop(self):
        pass
