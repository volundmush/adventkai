import typing
from pathlib import Path
from collections import defaultdict
from dataclasses import dataclass, field
from dataclasses_json import dataclass_json
from enum import IntEnum
import kdtree
import sys
from mudforge.utils import lazy_property
from mudrich.circle import CircleToRich, CircleStrip
import adventkai

from .typing import Vnum, Entity, GridCoordinates, SpaceCoordinates


@dataclass_json
@dataclass
class _Save:

    def should_save(self) -> bool:
        return True

    def save_name(self) -> str:
        return str(self.__class__.__name__)

    def export(self):
        return self.to_dict()

    @classmethod
    def deserialize(cls, data: typing.Any):
        return cls.from_dict(data)


@dataclass_json
@dataclass
class _NoSave(_Save):

    def should_save(self) -> bool:
        return False


@dataclass_json
@dataclass
class InGame(_NoSave):
    pass


@dataclass_json
@dataclass
class PendingRemove(_NoSave):
    pass


@dataclass_json
@dataclass
class IsPersistent(_NoSave):
    pass


@dataclass_json
@dataclass
class EntityID(_Save):
    module_name: str = ""
    prototype: str = ""
    ent_id: str = ""


@dataclass
class FlagBase(_Save):
    flags: dict[str, typing.Any] = field(default_factory=dict)

    def should_save(self) -> bool:
        return bool(self.flags)

    def export(self):
        return list(self.flags.keys())


@dataclass
class AffectFlags(FlagBase):
    pass


@dataclass_json
@dataclass
class PlayerCharacter(_Save):
    player_id: int


@dataclass_json
@dataclass
class NPC(_Save):
    vnum: Vnum = -1


@dataclass
class PlayerFlags(FlagBase):
    pass


@dataclass
class MobFlags(FlagBase):
    pass


@dataclass
class PreferenceFlags(FlagBase):
    pass


@dataclass_json
@dataclass
class Time(_Save):
    birth: int = 0
    created: int = 0
    maxage: int = 0
    logon: int = 0
    played: int = 0

    def should_save(self) -> bool:
        return bool(self.birth or self.created or self.maxage or self.logon or self.played)


@dataclass_json
@dataclass
class Molt(_Save):
    molt_exp: int = 0
    molt_level: int = 0

    def should_save(self) -> bool:
        return bool(self.molt_exp or self.molt_level)


class AndroidType(IntEnum):
    ABSORB = 0
    REPAIR = 1
    SENSE = 2


@dataclass_json
@dataclass
class Android(_Save):
    upgrades: int = 0
    model: AndroidType = AndroidType.ABSORB


@dataclass_json
@dataclass
class Absorber(_Save):
    ingest_learned: int = 0
    absorbs: int = 0

    def should_save(self) -> bool:
        return bool(self.ingest_learned or self.absorbs)


@dataclass_json
@dataclass
class ForgetSkill(_Save):
    forget_skill: int = 0
    forget_count: int = 0

    def should_save(self) -> bool:
        return bool(self.forget_count or self.forget_skill)


@dataclass_json
@dataclass
class LifeForce(_Save):
    life_percent: int = 0
    life: float = 1.0

    def should_save(self) -> bool:
        return bool(self.life < 1.0 or self.life_percent != 0)


@dataclass_json
@dataclass
class Health(_Save):
    health: float = 1.0
    stamina: float = 1.0
    energy: float = 1.0

    def should_save(self) -> bool:
        return bool(self.health < 1.0 or self.stamina < 1.0 or self.energy < 1.0)


@dataclass_json
@dataclass
class DeathData(_Save):
    death_time: int = 0
    death_count: int = 0
    # TODO: deal with death_room

    def should_save(self) -> bool:
        return bool(self.death_time or self.death_count)



@dataclass_json
@dataclass
class Transform(_Save):
    transformation: int = 0

    def should_save(self) -> bool:
        return bool(self.transformation)


@dataclass_json
@dataclass
class TransCost(_Save):
    transcost: set[int] = field(default_factory=set)

    def should_save(self) -> bool:
        return bool(self.transcost)

    @classmethod
    def deserialize(cls, data: typing.Any):
        c = cls()
        for i in data:
            c.transcost.add(i)
        return c


@dataclass_json
@dataclass
class TransClass(_Save):
    transclass: int = 0

    def should_save(self) -> bool:
        return bool(self.transclass)


@dataclass_json
@dataclass
class Clan(_Save):
    clan: str = ""

    def should_save(self) -> bool:
        return bool(self.clan)


@dataclass_json
@dataclass
class Kaioken(_Save):
    kaioken: int = 0

    def should_save(self) -> bool:
        return bool(self.kaioken)


@dataclass_json
@dataclass
class Frozen(_Save):
    freeze_level: int = 0

    def should_save(self) -> bool:
        return bool(self.freeze_level)


@dataclass_json
@dataclass
class AccountOwner(_Save):
    account: int = 0


@dataclass_json
@dataclass
class AdminLevel(_Save):
    admin_level: int = 0
    admin_invis: int = 0

    def should_save(self) -> bool:
        return bool(self.admin_level or self.admin_invis)


@dataclass_json
@dataclass
class Suppress(_Save):
    suppression: int = 0
    suppressed: int = 0

    def should_save(self) -> bool:
        return bool(self.suppressed or self.suppression)


@dataclass_json
@dataclass
class PowerStats(_Save):
    power: int = 1
    ki: int = 1
    stamina: int = 1

    def should_save(self) -> bool:
        return bool(self.power > 1 or self.ki > 1 or self.stamina > 1)


@dataclass_json
@dataclass
class HasVnum(_Save):
    vnum: Vnum = -1


@dataclass_json
@dataclass
class Level(_Save):
    level: int = 0
    level_adj: int = 0

    def should_save(self) -> bool:
        return bool(self.level or self.level_adj)


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


@dataclass_json
@dataclass
class Physics(_Save):
    weight: float = 0
    height: float = 0
    size: Sizes = Sizes.UNDEFINED

    def should_save(self) -> bool:
        return bool(self.weight or self.height or self.size != Sizes.UNDEFINED)


@dataclass_json
@dataclass
class Prototype(_Save):
    name: str
    ids: set[str] = field(default_factory=set)



@dataclass_json
@dataclass
class HasLegacyMobProto(_Save):
    vnum: int = -1


@dataclass_json
@dataclass
class HasLegacyObjProto(_Save):
    vnum: int = -1


@dataclass_json
@dataclass
class Alignment(_Save):
    alignment: int = 0

    def should_save(self) -> bool:
        return bool(self.alignment)


@dataclass_json
@dataclass
class DgScriptProto(_Save):
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


@dataclass_json
@dataclass
class DgScriptState(_Save):
    actor: Entity = -1
    state: TrigState = TrigState.DORMANT
    curr_line: int = 0
    depth: list[typing.Tuple[NestType, int]] = field(default_factory=list)


@dataclass_json
@dataclass
class Triggers(_Save):
    triggers: list[Vnum] = field(default_factory=list)

    def should_save(self) -> bool:
        return bool(self.triggers)


@dataclass
class DgScriptHolder(_Save):
    types: int = 0
    scripts: list[Entity] = field(default_factory=list)
    variables: dict[int, dict[str, str]] = field(default_factory=dict)

    def should_save(self) -> bool:
        return bool(self.types or self.variables)

    def export(self):
        return {"types": self.types, "variables": self.variables}


@dataclass_json
@dataclass
class Inventory(_NoSave):
    inventory: list[Entity] = field(default_factory=list)


@dataclass_json
@dataclass
class Equipment(_NoSave):
    equipment: dict[int, Entity] = field(default_factory=dict)


@dataclass_json
@dataclass
class SaveInRoom(EntityID):
    vnum: Vnum = -1


@dataclass_json
@dataclass
class InRoom(_Save):
    holder: Entity = -1

    def should_save(self) -> bool:
        return adventkai.WORLD.entity_exists(self.holder)

    def save_name(self) -> str:
        return "SaveInRoom"

    def export(self):
        data = {}
        if (ent_data := adventkai.WORLD.try_component(self.holder, EntityID)):
            data.update(ent_data.to_dict())
        if (vn := adventkai.WORLD.try_component(self.holder, HasVnum)):
            if vn:
                data["vnum"] = vn.vnum
        return data


@dataclass_json
@dataclass
class SaveInInventory(EntityID):
    pass


@dataclass_json
@dataclass
class InInventory(_Save):
    holder: Entity = -1

    def should_save(self) -> bool:
        return adventkai.WORLD.entity_exists(self.holder)

    def save_name(self) -> str:
        return "SaveInInventory"

    def export(self):
        ent_data = adventkai.WORLD.try_component(self.holder, EntityID)
        if ent_data:
            return ent_data.to_dict()


@dataclass_json
@dataclass
class SaveEquipped(EntityID):
    slot: int = -1


@dataclass_json
@dataclass
class Equipped(_Save):
    holder: Entity = -1
    slot: int = -1

    def should_save(self) -> bool:
        return adventkai.WORLD.entity_exists(self.holder)

    def save_name(self) -> str:
        return "SaveEquipped"

    def export(self):
        if (ent_data := adventkai.WORLD.try_component(self.holder, EntityID)):
            data = ent_data.to_dict()
            data["slot"] = self.slot
            return data


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


@dataclass_json
@dataclass
class Money(_Save):
    money: int = 0

    def should_save(self) -> bool:
        return bool(self.money)


@dataclass_json
@dataclass
class Bonuses(FlagBase):
    pass


@dataclass_json
@dataclass
class BankAccount(_Save):
    last_interest: int = 0
    value: int = 0

    def should_save(self) -> bool:
        return bool(self.last_interest or self.value)


@dataclass_json
@dataclass
class Race(_Save):
    race: "Race"

    def export(self):
        return self.race.mod_id

    @classmethod
    def deserialize(cls, data: typing.Any):
        if(isinstance(data, int)):
            if (r := adventkai.MODIFIERS_ID["race"].get(data, None)):
                return cls(race=r)
        if(isinstance(data, str)):
            if (r := adventkai.MODIFIERS_NAMES["race"].get(data, None)):
                return cls(race=r)
        raise Exception(f"Cannot locate race {data}")


@dataclass_json
@dataclass
class Physiology(_Save):
    sex: int = 0
    hair_length: int = 0
    hair_style: int = 0
    hair_color: int = 0
    skin_color: int = 0
    eye_color: int = 0
    distinguishing_feature: int = 0
    aura_color: int = 0
    racial_pref: int = 0

    def should_save(self) -> bool:
        return bool(self.sex or self.hair_color or self.hair_length or self.hair_style or self.skin_color
                    or self.eye_color or self.distinguishing_feature or self.aura_color or self.racial_pref)


@dataclass_json
@dataclass
class Sensei(_Save):
    sensei: "Sensei"

    def export(self):
        return self.sensei.mod_id

    @classmethod
    def deserialize(cls, data: typing.Any):
        if (isinstance(data, int)):
            if (r := adventkai.MODIFIERS_ID["sensei"].get(data, None)):
                return cls(sensei=r)
        if (isinstance(data, str)):
            if (r := adventkai.MODIFIERS_NAMES["sensei"].get(data, None)):
                return cls(sensei=r)
        raise Exception(f"Cannot locate sensei {data}")


@dataclass_json
@dataclass
class Skill:
    level: int = 0
    bonus: int = 0
    perfection: int = 0

    def should_save(self) -> bool:
        return bool(self.level or self.bonus or self.perfection)


@dataclass_json
@dataclass
class HasSkills(_Save):
    skills: dict[int, Skill] = field(default_factory=dict)
    skill_slots: int = 0

    def should_save(self) -> bool:
        if self.skil_slots:
            return True
        for k, v in self.skills:
            if v.should_save():
                return True

    def export(self):
        data = {"skill_slots": self.skill_slots}
        skill_data = {k: v.to_dict() for k, v in self.skills.items() if v.should_save()}
        if skill_data:
            data["skills"] = skill_data
        return data


@dataclass_json
@dataclass
class Stats(_Save):
    strength: int = 0
    intelligence: int = 0
    wisdom: int = 0
    agility: int = 0
    constitution: int = 0
    speed: int = 0
    luck: int = 0

    def should_save(self) -> bool:
        return bool(self.strength or self.intelligence or self.wisdom or self.agility or self.constitution
                    or self.speed or self.luck)


@dataclass_json
@dataclass
class StatTrain(_Save):
    strength: int = 0
    intelligence: int = 0
    wisdom: int = 0
    agility: int = 0
    constitution: int = 0
    speed: int = 0

    def should_save(self) -> bool:
        return bool(self.strength or self.intelligence or self.wisdom or self.agility or self.constitution
                    or self.speed)


class StringBase(_Save):
    rich_cache = dict()

    def should_save(self) -> bool:
        return bool(self.rich.plain)

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

    def export(self):
        return self.color


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


@dataclass_json
@dataclass
class ExDescriptions(_Save):
    ex_descriptions: list[typing.Tuple[Name, Description]] = field(default_factory=list)

    def should_save(self) -> bool:
        return bool(self.ex_descriptions)

    def export(self):
        return [(key.export(), desc.export()) for (key, desc) in self.ex_descriptions]


@dataclass_json
@dataclass
class ItemValues(_Save):
    values: dict[int, int] = field(default_factory=dict)

    def should_save(self) -> bool:
        for k, v in self.values.items():
            if v:
                return True
        return False

    def export(self):
        return {k: v for k, v in self.values.items() if v}


@dataclass_json
@dataclass
class Experience(_Save):
    exp: int = 0

    def should_save(self) -> bool:
        return bool(self.exp)


@dataclass_json
@dataclass
class AdminFlags(FlagBase):
    pass


@dataclass_json
@dataclass
class ItemType:
    type_flag: typing.Any

    def export(self):
        return self.type_flag.mod_id

    @classmethod
    def deserialize(cls, data: typing.Any):
        if (isinstance(data, int)):
            if (r := adventkai.MODIFIERS_ID["item_types"].get(data, None)):
                return cls(type_flag=r)
        if (isinstance(data, str)):
            if (r := adventkai.MODIFIERS_NAMES["item_types"].get(data, None)):
                return cls(type_flag=r)
        raise Exception(f"Cannot locate Item Type {data}")


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


@dataclass_json
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


@dataclass_json
@dataclass
class Exits(_Save):
    exits: dict[ExitDir, RoomExit] = field(default_factory=dict)

    def should_save(self) -> bool:
        return bool(self.exits)


@dataclass
class RoomFlags(FlagBase):
    pass


@dataclass
class ZoneFlags(FlagBase):
    pass


@dataclass
class ItemFlags(FlagBase):
    pass


@dataclass
class SectorType:
    sector: typing.Any

    def export(self):
        return self.sector.mod_id

    @classmethod
    def deserialize(cls, data: typing.Any):
        if (isinstance(data, int)):
            if (r := adventkai.MODIFIERS_ID["room_sectors"].get(data, None)):
                return cls(sector=r)
        if (isinstance(data, str)):
            if (r := adventkai.MODIFIERS_NAMES["room_sectors"].get(data, None)):
                return cls(sector=r)
        raise Exception(f"Cannot locate Room Type {data}")


@dataclass
class Session(_Save):
    connections: set["GameConnection"] = field(default_factory=set)
    character: Entity = -1
    puppet: Entity = -1

    def export(self):
        data = {}
        data["connections"] = [c.conn_id for c in self.connections]
        if adventkai.WORLD.entity_exists(self.character):
            data["character"] = adventkai.WORLD.get_component(self.character, EntityID).export()
        return data


@dataclass
class HasSession(_NoSave):
    session: Entity = -1


@dataclass_json
@dataclass
class ZoneResetCmd:
    """
    Commands:
    M - Read a mobile
    O - Read an Object
    G - Give obj to mob
    P - put obj in obj
    G - obj to Char
    E - obj to char equip
    D - set state of door
    T - trigger command
    V - assign variable
    """
    command: str = "-"
    if_flag: bool = False
    arg1: int = 0
    arg2: int = 0
    arg3: int = 0
    arg4: int = 0
    arg5: int = 0
    sarg1: str = ""
    sarg2: str = ""


@dataclass_json
@dataclass
class Zone(_Save):
    dir: Path = None
    legacy_builders: list[str] = field(default_factory=list)
    builders: list[Entity] = field(default_factory=list)
    lifespan: int = 0
    age: int = 0
    bot: int = 0
    top: int = 0
    reset_mode: int = 0
    commands: list[ZoneResetCmd] = field(default_factory=list)
    min_level: int = 0
    max_level: int = 0


@dataclass_json
@dataclass
class ZoneVnums(_NoSave):
    rooms: dict[Vnum, Entity] = field(default_factory=dict)
    objects: dict[Vnum, Entity] = field(default_factory=dict)
    mobiles: dict[Vnum, Entity] = field(default_factory=dict)
    triggers: dict[Vnum, Entity] = field(default_factory=dict)
    shops: dict[Vnum, Entity] = field(default_factory=dict)
    guilds: dict[Vnum, Entity] = field(default_factory=dict)
