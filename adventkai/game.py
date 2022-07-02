from mudforge.services.game import GameService as OldGame
import mudforge
import logging
import adventkai
from .modules import Module
from pathlib import Path
from .legacy import LegacyLoader
from adventkai.db.accounts.models import Account


class GameService(OldGame):

    async def on_start(self):
        mod_path = Path("modules")
        logging.info(f"Loading game database from {mod_path}")
        save_root = Path("save")

        for p in [p for p in mod_path.iterdir() if p.is_dir()]:
            m = Module(p.name, p, save_root / p.name)
            adventkai.MODULES[m.name] = m

        logging.info(f"Discovered {len(adventkai.MODULES)} modules!")

        for k, v in adventkai.MODULES.items():
            await v.load_maps()

        for k, v in adventkai.MODULES.items():
            await v.load_prototypes()

        legacy_path = Path("legacy")
        legacy_loader = None
        if legacy_path.exists() and legacy_path.is_dir():
            logging.info("Loading legacy database assets...")
            legacy_loader = LegacyLoader(legacy_path)
            await legacy_loader.load_assets()

        if not Account.objects.count() and legacy_loader:
            logging.info("loading legacy player data...")
            await legacy_loader.load_userdata()

        for k, v in adventkai.MODULES.items():
            await v.load_entities_initial()

        for k, v in adventkai.MODULES.items():
            await v.load_entities_finalize()

    async def game_loop(self):
        pass
