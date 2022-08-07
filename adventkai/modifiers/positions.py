from .base import Modifier as _BaseMod


class _Position(_BaseMod):
    category = "Position"
    modifier_id = -1
    incap_message = "unimplemented"


class Dead(_Position):
    modifier_id = 0
    incap_message = "Lie still; you are |rDEAD!|n"


class MortallyWounded(_Position):
    name = "Mortally Wounded"
    modifier_id = 1
    incap_message = "You are in pretty bad shape, unable to do anything!"


class Incapacitated(_Position):
    modifier_id = 2


class Stunned(_Position):
    modifier_id = 3
    incap_message = "All you can do right now is think about the stars!"


class Sleeping(_Position):
    modifier_id = 4
    incap_message = "In your dreams, or what?"


class Resting(_Position):
    modifier_id = 5
    incap_message = "Nah... You feel too relaxed to do that..."


class Sitting(_Position):
    modifier_id = 6
    incap_message = "Maybe you should get on your feet first?"


class Fighting(_Position):
    modifier_id = 7
    incap_message = "No way! You're fighting for your life!"


class Standing(_Position):
    modifier_id = 8
