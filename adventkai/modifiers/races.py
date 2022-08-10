from .base import Modifier as _BaseMod
from adventkai.components import Sizes
from . import transformations as t
import typing
from snekmud import EQUIP_SLOTS
from adventkai.utils import get_trait

class _Race(_BaseMod):
    category = "Race"
    pc_ok = True
    mimic_ok = True
    seeming_ok = True
    size = Sizes.MEDIUM
    abbr = "--"
    has_seeming = False
    rpp_cost = 0

    def get_available_transformations(self, ent) -> list[typing.Type[t._Form]]:
        return []

    def generate_sdesc(self) -> str:
        return f"{self.owner.gender} {self.get_name().lower()}"

    def get_available_limbs(self):
        return {"left_arm": 100, "right_arm": 100, "left_leg": 100, "right_leg": 100}


class Human(_Race):
    modifier_id = 0
    abbr = "Hum"


class Saiyan(_Race):
    modifier_id = 1
    abbr = "Sai"
    rpp_cost = 65

    def get_available_transformations(self, ent) -> list[typing.Type[t._Form]]:
        pflags = get_trait(ent, "PlayerFlags")
        if pflags.has("LegendarySSJ"):
            return [t.SuperSaiyan, t.LegendarySuperSaiyan]
        else:
            return [t.SuperSaiyan, t.SuperSaiyan2, t.SuperSaiyan3, t.SuperSaiyan4]

    def get_available_limbs(self):
        return super().get_available_limbs().update({"tail": 100})


class Icer(_Race):
    modifier_id = 2
    abbr = "Ice"

    def get_available_limbs(self):
        return super().get_available_limbs().update({"tail": 100})


class Konatsu(_Race):
    modifier_id = 3
    abbr = "Kon"


class Namekian(_Race):
    modifier_id = 4
    abbr = "Nam"

    def generate_sdesc(self) -> str:
        return f"{self.get_name().lower()}"


class Mutant(_Race):
    modifier_id = 5
    abbr = "Mut"


class Kanassan(_Race):
    modifier_id = 6
    abbr = "Kan"


class Halfbreed(_Race):
    modifier_id = 7
    abbr = "H-B"

    def get_available_transformations(self, ent) -> list[typing.Type[t._Form]]:
        return [t.HBSuperSaiyan, t.HBSuperSaiyan2, t.HBSuperSaiyan3]

    def get_available_limbs(self):
        return super().get_available_limbs().update({"tail": 100})


class BioAndroid(_Race):
    modifier_id = 8
    abbr = "Bio"
    seeming_ok = False

    def get_available_limbs(self):
        return super().get_available_limbs().update({"tail": 100})


class Android(_Race):
    modifier_id = 9
    abbr = "And"
    has_seeming = True


class Demon(_Race):
    modifier_id = 10
    abbr = "Dem"


class Majin(_Race):
    modifier_id = 11
    abbr = "Maj"


class Kai(_Race):
    modifier_id = 12
    abbr = "Kai"


class Tuffle(_Race):
    modifier_id = 13
    size = Sizes.SMALL
    abbr = "Tuf"


class Hoshijin(_Race):
    modifier_id = 14
    abbr = "Hos"
    mimic_ok = False
    seeming_ok = False


class Arlian(_Race):
    modifier_id = 20
    abbr = "Arl"


class _NPC(_Race):
    pc_ok = False
    mimic_ok = False
    seeming_ok = False


class Animal(_NPC):
    modifier_id = 15
    abbr = "Ani"


class Saiba(_NPC):
    modifier_id = 16
    abbr = "Sab"
    size = Sizes.LARGE


class Serpent(_NPC):
    modifier_id = 17
    abbr = "Ser"


class Ogre(_NPC):
    modifier_id = 18
    abbr = "Ogr"
    size = Sizes.LARGE


class Yardratian(_NPC):
    modifier_id = 19
    abbr = "Yar"


class Dragon(_NPC):
    modifier_id = 21
    abbr = "Drg"


class Mechanical(_NPC):
    modifier_id = 22
    abbr = "Mec"


class Spirit(_NPC):
    modifier_id = 23
    abbr = "Spi"
