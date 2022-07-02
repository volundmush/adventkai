from .base import Modifier as _BaseMod
from adventkai.typing import Entity, Vnum
from adventkai.components import Sizes


class _Race(_BaseMod):
    category = "Race"
    mod_id = -1
    pc_ok = True
    size = Sizes.MEDIUM
    abbr = "--"

    def rpp_cost(self, ent: Entity) -> int:
        return 0


class Human(_Race):
    mod_id = 0
    abbr = "Hum"


class Saiyan(_Race):
    mod_id = 1
    abbr = "Sai"

    def rpp_cost(self, ent: Entity) -> int:
        return 65


class Icer(_Race):
    mod_id = 2
    abbr = "Ice"


class Konatsu(_Race):
    mod_id = 3
    abbr = "Kon"


class Namekian(_Race):
    mod_id = 4
    abbr = "Nam"


class Mutant(_Race):
    mod_id = 5
    abbr = "Mut"


class Kanassan(_Race):
    mod_id = 6
    abbr = "Kan"


class Halfbreed(_Race):
    mod_id = 7
    abbr = "H-B"


class BioAndroid(_Race):
    mod_id = 8
    abbr = "Bio"


class Android(_Race):
    mod_id = 9
    abbr = "And"


class Demon(_Race):
    mod_id = 10
    abbr = "Dem"


class Majin(_Race):
    mod_id = 11
    abbr = "Maj"


class Kai(_Race):
    mod_id = 12
    abbr = "Kai"


class Tuffle(_Race):
    mod_id = 13
    size = Sizes.SMALL
    abbr = "Tuf"


class Hoshijin(_Race):
    mod_id = 14
    abbr = "Hos"


class Arlian(_Race):
    mod_id = 20
    abbr = "Arl"


class _NPC(_Race):
    pc_ok = False


class Animal(_NPC):
    mod_id = 15
    abbr = "Ani"


class Saiba(_NPC):
    mod_id = 16
    abbr = "Sab"
    size = Sizes.LARGE


class Serpent(_NPC):
    mod_id = 17
    abbr = "Ser"


class Ogre(_NPC):
    mod_id = 18
    abbr = "Ogr"
    size = Sizes.LARGE


class Yardratian(_NPC):
    mod_id = 19
    abbr = "Yar"


class Dragon(_NPC):
    mod_id = 21
    abbr = "Drg"


class Mechanical(_NPC):
    mod_id = 22
    abbr = "Mec"


class Spirit(_NPC):
    mod_id = 23
    abbr = "Spi"