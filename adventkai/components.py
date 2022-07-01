import typing
from collections import defaultdict
from dataclasses import dataclass, field
from enum import IntEnum
import kdtree
import sys
from mudforge.utils import lazy_property
from mudrich.circle import CircleToRich, CircleStrip

from .typing import Vnum, Entity, GridCoordinates, SpaceCoordinates

@dataclass
class InGame:
    pass


@dataclass
class PendingRemove:
    pass


@dataclass
class IsPersistent:
    pass


@dataclass
class EntityID:
    module_name: str
    ent_id: str


@dataclass
class FlagBase:
    flags: dict[str, typing.Any] = field(default_factory=dict)


@dataclass
class AffectFlags(FlagBase):
    pass


@dataclass
class PlayerCharacter:
    unique_id: int


@dataclass
class NPC:
    vnum: typing.Optional[Vnum]


@dataclass
class PlayerFlags(FlagBase):
    pass


@dataclass
class MobFlags(FlagBase):
    pass


@dataclass
class PreferenceFlags(FlagBase):
    pass


@dataclass
class Time:
    birth: int = 0
    created: int = 0
    maxage: int = 0
    logon: int = 0
    played: int = 0


@dataclass
class Molt:
    molt_exp: int = 0
    molt_level: int = 0


class AndroidType(IntEnum):
    ABSORB = 0
    REPAIR = 1
    SENSE = 2


@dataclass
class Android:
    upgrades: int = 0
    model: AndroidType = AndroidType.ABSORB


@dataclass
class Absorber:
    ingest_learned: int = 0
    absorbs: int = 0


@dataclass
class ForgetSkill:
    forget_skill: int = 0
    forget_count: int = 0


@dataclass
class LifeForce:
    life_percent: int = 0
    life: float = 0.0


@dataclass
class Health:
    health: float = 1.0
    stamina: float = 1.0
    energy: float = 1.0


@dataclass
class DeathData:
    death_time: int = 0
    death_count: int = 0
    # TODO: deal with death_room


@dataclass
class Transform:
    transformation: int = 0


@dataclass
class TransCost:
    transcost: dict[int, int] = field(default_factory=dict)


@dataclass
class TransClass:
    transclass: int = 0


@dataclass
class Clan:
    clan: str


@dataclass
class Kaioken:
    kaioken: int = 0


@dataclass
class Frozen:
    freeze_level: int = 0


@dataclass
class AccountOwner:
    account: int = 0


@dataclass
class AdminLevel:
    admin_level: int = 0
    admin_invis: int = 0


@dataclass
class Suppress:
    suppression: int = 0
    suppressed: int = 0


@dataclass
class PowerStats:
    power: int = 1
    ki: int = 1
    stamina: int = 1


@dataclass
class HasVnum:
    vnum: Vnum = -1


@dataclass
class Level:
    level: int = 0
    level_adj: int = 1


class Sizes(IntEnum):
    UNDEFINED = -1
    FINE = 0
    DIMINUTIVE = 1
    TINY = 2
    SMALL = 3
    MEDIUM = 4
    LARGE = 5
    HUGE = 6
    GARGANTUAN = 7
    COLOSSAL = 8


@dataclass
class Physics:
    weight: float = 0
    height: float = 0
    size: Sizes = Sizes.UNDEFINED


@dataclass
class Prototype:
    name: str
    ids: set[str] = field(default_factory=set)


@dataclass
class HasPrototype:
    prototype: Entity = -1


@dataclass
class HasLegacyMobProto:
    vnum: int = -1


@dataclass
class HasLegacyObjProto:
    vnum: int = -1


@dataclass
class Alignment:
    alignment: int = 0


@dataclass
class DgScriptProto:
    attach_type: int = 0
    data_type: int = 0
    trigger_type: int = 0
    cmdlist: list[str] = field(default_factory=list)
    narg: int = 0
    arglist: str = ""
    instances: set[int] = field(default_factory=set)


class TrigState(IntEnum):
    DORMANT = 0
    RUNNING = 1
    WAITING = 2
    ERROR = 3
    DONE = 4
    PURGED = 5


class NestType(IntEnum):
    IF = 0
    WHILE = 1
    SWITCH = 2


@dataclass
class DgScriptState:
    actor: Entity = -1
    state: TrigState = TrigState.DORMANT
    curr_line: int = 0
    depth: list[typing.Tuple[NestType, int]] = field(default_factory=list)


@dataclass
class Triggers:
    triggers: list[Vnum] = field(default_factory=list)


@dataclass
class DgScriptHolder:
    types: int = 0
    scripts: list[Entity] = field(default_factory=list)
    variables: dict[str, str] = field(default_factory=dict)


@dataclass
class Inventory:
    inventory: list[Entity] = field(default_factory=list)


@dataclass
class Equipment:
    equipment: dict[int, Entity] = field(default_factory=dict)


@dataclass
class InRoom:
    holder: Entity = -1


@dataclass
class InInventory:
    holder: Entity = -1


@dataclass
class Equipped:
    holder: Entity = -1
    slot: int = -1


class PointHolder:

    def __init__(self, coordinates: typing.Union[GridCoordinates, SpaceCoordinates], data: typing.Optional[typing.Any] = None):
        self.coordinates = coordinates
        self.data = data

    def __len__(self):
        return len(self.coordinates)

    def __getitem__(self, i):
        return self.coordinates[i]

    def __repr__(self):
        return f"Item({self.coordinates},  {self.data})"


@dataclass
class InSpace:
    space_sector: Entity = -1


@dataclass
class GridMap:
    rooms: kdtree.Node = field(default_factory=lambda: kdtree.create(dimensions=3))


@dataclass
class SpaceMap:
    contents: kdtree.Node = field(default_factory=lambda: kdtree.create(dimensions=3))


@dataclass
class Money:
    money: int = 0


@dataclass
class BankAccount:
    last_interest: int = 0
    value: int = 0


@dataclass
class Physiology:
    race: "Race"
    sex: int = 0
    hair_length: int = 0
    hair_style: int = 0
    hair_color: int = 0
    skin_color: int = 0
    eye_color: int = 0
    distinguishing_feature: int = 0
    aura_color: int = 0
    racial_pref: int = 0


class Sensei:
    sensei: "Sensei"

@dataclass
class Skill:
    level: int = 0
    bonus: int = 0
    perfection: int = 0


@dataclass
class HasSkills:
    skills: dict[0, Skill] = field(default_factory=dict)
    skill_slots: int = 0


@dataclass
class Stats:
    strength: int = 0
    intelligence: int = 0
    wisdom: int = 0
    agility: int = 0
    constitution: int = 0
    speed: int = 0
    luck: int = 0


@dataclass
class StatTrain:
    strength: int = 0
    intelligence: int = 0
    wisdom: int = 0
    agility: int = 0
    constitution: int = 0
    speed: int = 0


class StringBase:
    rich_cache = dict()

    def __init__(self, s: str):
        self.color = sys.intern(s)
        if self.color not in self.rich_cache:
            self.rich_cache[self.color] = CircleToRich(s)

    def __str__(self):
        return self.rich.plain

    @lazy_property
    def rich(self):
        return self.rich_cache[self.color]

    def __rich_console__(self, console, options):
        return self.rich.__rich_console__(console, options)

    def __rich_measure__(self, console, options):
        return self.rich.__rich_measure__(console, options)

    def render(self, console, end: str = ""):
        return self.rich.render(console, end=end)


class Name(StringBase):
    pass


class Description(StringBase):
    pass


class ShortDescription(StringBase):
    pass


class LongDescription(StringBase):
    pass


class ActionDescription(StringBase):
    pass


@dataclass
class ExDescriptions:
    ex_descriptions: list[typing.Tuple[Name, Description]] = field(default_factory=list)


@dataclass
class ItemValues:
    values: dict[int, int] = field(default_factory=dict)


@dataclass
class ItemType:
    type_flag: int = -1


class WearFlags(FlagBase):
    pass


class ExtraFlags(FlagBase):
    pass


class ExitDir(IntEnum):
    UNKNOWN = -1
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3
    UP = 4
    DOWN = 5
    NORTHWEST = 6
    NORTHEAST = 7
    SOUTHEAST = 8
    SOUTHWEST = 9
    INWARDS = 10
    OUTWARDS = 11

    def reverse(self) -> "ExitDir":
        match self:
            case ExitDir.NORTH:
                return ExitDir.SOUTH
            case ExitDir.EAST:
                return ExitDir.WEST
            case ExitDir.SOUTH:
                return ExitDir.NORTH
            case ExitDir.WEST:
                return ExitDir.EAST
            case ExitDir.UP:
                return ExitDir.DOWN
            case ExitDir.DOWN:
                return ExitDir.UP
            case ExitDir.NORTHWEST:
                return ExitDir.SOUTHEAST
            case ExitDir.NORTHEAST:
                return ExitDir.SOUTHWEST
            case ExitDir.SOUTHEAST:
                return ExitDir.NORTHWEST
            case ExitDir.SOUTHWEST:
                return ExitDir.NORTHEAST
            case ExitDir.INWARDS:
                return ExitDir.OUTWARDS
            case ExitDir.OUTWARDS:
                return ExitDir.INWARDS
            case _:
                return ExitDir.UNKNOWN

    def abbr(self) -> str:
        match self:
            case ExitDir.NORTH:
                return "N"
            case ExitDir.EAST:
                return "W"
            case ExitDir.SOUTH:
                return "S"
            case ExitDir.WEST:
                return "W"
            case ExitDir.UP:
                return "U"
            case ExitDir.DOWN:
                return "D"
            case ExitDir.NORTHWEST:
                return "NW"
            case ExitDir.NORTHEAST:
                return "NE"
            case ExitDir.SOUTHEAST:
                return "SE"
            case ExitDir.SOUTHWEST:
                return "SW"
            case ExitDir.INWARDS:
                return "I"
            case ExitDir.OUTWARDS:
                return "O"
            case _:
                return "--"

@dataclass
class RoomExit:
    description: typing.Optional[Description] = None
    keyword: typing.Optional[Name] = None
    exit_info: int = 0
    key: typing.Optional[Vnum] = None
    to_room: typing.Optional[Vnum] = None
    fail_room: typing.Optional[Vnum] = None
    total_fail_room: typing.Optional[Vnum] = None
    dclock: int = 0
    dchide: int = 0
    dcskill: int = 0
    dcmove: int = 0
    failsavetype: int = 0
    dcfailsave: int = 0


@dataclass
class Exits:
    exits: dict[ExitDir, RoomExit] = field(default_factory=dict)


@dataclass
class RoomFlags(FlagBase):
    pass


@dataclass
class SectorType:
    sector: typing.Any


@dataclass
class Session:
    connections: set["GameConnection"] = field(default_factory=set)
    character: Entity = -1
    puppet: Entity = -1


@dataclass
class HasSession:
    session: Entity = -1
