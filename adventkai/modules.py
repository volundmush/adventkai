import sys
from pathlib import Path


class Module:

    def __init__(self, name: str, path: Path, save_path: Path):
        self.name = sys.intern(name)
        self.maps: dict[str, int] = dict()
        self.prototypes: dict[str, int] = dict()
        self.entities: dict[str, int] = dict()
        self.path = path
        self.save_path = save_path

    def __str__(self):
        return self.name


    async def load_maps(self):
        pass


    async def load_prototypes(self):
        pass


    async def load_entities_initial(self):
        pass


    async def load_entities_finalize(self):
        pass