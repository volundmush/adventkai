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

