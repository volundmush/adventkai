import logging
from snekmud.db.accounts.models import Account
from snekmud.db.gamesessions.models import GameSession
from snekmud.game import GameService as OldGame
import snekmud
import adventkai


class GameService(OldGame):

    async def load_middle(self):
        await super().load_middle()

        legacy = snekmud.MODULES["legacy"]

        if not Account.objects.count():
            logging.info("loading legacy player data...")
            await legacy.load_userdata()

    async def game_loop(self):
        for x in set(adventkai.DG_PAUSED):
            await x.decrement_timer(self.tick_rate)
        for x in GameSession.objects.all():
            await x.handler.update(self.tick_rate)
