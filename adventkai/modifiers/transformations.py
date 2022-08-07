from .base import Modifier as _BaseMod


class _Form(_BaseMod):
    category = "Transformation"
    modifier_id = -1
    can_revert = True
    echo_transform = "Unimplemented"
    echo_revert = "|c$You()|W $conj(reverts) from {trans_name}"
    trans_name = "nameless transform"

    @classmethod
    def can_transform(cls, obj) -> bool:
        return True


class _EvenForm(_Form):
    power_bonus = 0
    power_mult = 0.0

    def stat_bonus(self, obj, stat_name) -> int:
        match stat_name:
            case "powerlevel" | "ki" | "stamina":
                return self.power_bonus
            case _:
                return 0

    def stat_multiplier(self, obj, stat_name) -> float:
        match stat_name:
            case "powerlevel" | "ki" | "stamina":
                return self.power_mult
            case _:
                return 0


class _SuperSaiyan(_EvenForm):
    pass


class SuperSaiyan(_SuperSaiyan):
    modifier_id = 0
    name = "Super Saiyan"

    echo_transform = "|c$You()|w $conj(screams) in rage as lightning begins to crash all around! $Pron(your) hair turns golden and eyes change to an emerald color as a bright golden aura bursts up around $pron(your) body! As $pron(your) energy stabilizes, a fierce look spreads across $pron(your) face, having transformed into a |cSuper |ySaiyan|w!|n"

    trans_name = "|cSuper |CSaiyan |g1|n"

    power_bonus = 800000
    power_mult = 1.0


class SuperSaiyan2(_SuperSaiyan):
    modifier_id = 1
    name = "Super Saiyan 2"

    echo_transform = "|w$You()|w $conj(stands) up straight with $pron(your) head back as $pron(you) $conj(releases) an ear piercing scream! A blindingly bright golden aura bursts up around $pron(your) body, glowing as bright as the sun. As rushing winds begin to rocket out from $pron(you) in every direction, bolts of electricity flash and crackle in $pron(your) aura. As $pron(your) aura begins to dim $pron(you) stand confidently, having achieved |cSuper |ySaiyan |g2|w!|n"

    trans_name = "|cSuper |CSaiyan |g2|n"

    power_bonus = 20000000
    power_mult = 2.0


class SuperSaiyan3(_SuperSaiyan):
    modifier_id = 2
    name = "Super Saiyan 3"

    trans_name = "|cSuper |CSaiyan |g3|n"

    power_bonus = 80000000
    power_mult = 3.0


class SuperSaiyan4(_SuperSaiyan):
    modifier_id = 3
    name = "Super Saiyan 4"

    trans_name = "|cSuper |CSaiyan |g4|n"

    power_bonus = 182000000
    power_mult = 4.5


class LegendarySuperSaiyan(_SuperSaiyan):
    modifier_id = 4
    name = "Legendary Super Saiyan"
    power_bonus = 185000000
    power_mult = 5.0

    trans_name = "|gLegendary |ySuper Saiyan|n"


class Oozaru(_EvenForm):
    modifier_id = 5
    can_revert = False
    trans_name = "|YOozaru|n"
    power_bonus = 10000
    power_mult = 1.0


class HBSuperSaiyan(SuperSaiyan):
    power_bonus = 900000
    modifier_id = 6


class HBSuperSaiyan2(SuperSaiyan2):
    power_bonus = 16500000
    power_mult = 3.0
    modifier_id = 7


class HBSuperSaiyan3(SuperSaiyan3):
    power_bonus = 240000000
    power_mult = 4.0
    modifier_id = 8
