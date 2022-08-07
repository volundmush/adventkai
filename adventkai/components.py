import typing
from pathlib import Path
from dataclasses import dataclass, field
from dataclasses_json import dataclass_json
from enum import IntEnum
import adventkai
import snekmud
from snekmud import components as old_cm
from snekmud.components import _Save, _NoSave, _SingleModifier, EntityID, Name, Description
from adventkai.typing import Vnum
from snekmud.typing import Entity


class Position(_SingleModifier):
    pass


@dataclass_json
@dataclass
class FlagBase(_Save):
    flags: dict[str, typing.Any] = field(default_factory=dict)

    def should_save(self) -> bool:
        return bool(self.flags)

    def export(self):
        return list(self.flags.keys())

    @classmethod
    def deserialize(cls, data: typing.Any, ent):
        names = snekmud.MODIFIERS_NAMES[cls.__name__]
        ids = snekmud.MODIFIERS_ID[cls.__name__]
        o = cls()
        for i in data:
            if isinstance(i, int):
                if (found := ids.get(i, None)):
                    o.flags[str(found)] = found
            elif isinstance(i, str):
                if (found := names.get(i, None)):
                    o.flags[str(found)] = found

@dataclass
class AffectFlags(FlagBase):
    pass


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




@dataclass_json
@dataclass
class Upgrades(_Save):
    upgrades: int = 0

    def should_save(self) -> bool:
        return bool(self.upgrades)


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
    def deserialize(cls, data: typing.Any, ent):
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


    def export(self):
        data = {}
        if self.weight:
            data["weight"] = self.weight
        if self.height:
            data["height"] = self.height
        if self.size != Sizes.UNDEFINED:
            data["size"] = int(self.size)
        return data

    @classmethod
    def deserialize(cls, data: typing.Any, ent):
        o = cls()
        for f in ("weight", "height"):
            if f in data:
                setattr(o, f, data.pop(f))
        if "size" in data:
            o.size = Sizes(data.pop("size"))
        return o


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

    @classmethod
    def deserialize(cls, data: typing.Any, ent):
        o = cls()
        for i in data:
            o.triggers.append(i)
        return o

    def export(self):
        return self.triggers


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
class SaveInLegacyRoom(EntityID):
    vnum: Vnum = -1

@dataclass_json
@dataclass
class InLegacyRoom(_Save):
    holder: Entity = -1

    def should_save(self) -> bool:
        return snekmud.WORLD.entity_exists(self.holder)

    def save_name(self) -> str:
        return "SaveInLegacyRoom"

    def export(self):
        data = {}
        if (ent_data := snekmud.WORLD.try_component(self.holder, HasVnum)):
            data["vnum"] = ent_data.vnum
        return data


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
class Race(_SingleModifier):
    pass


@dataclass_json
@dataclass
class AndroidType(_SingleModifier):
    pass


@dataclass_json
@dataclass
class Seeming(_Save):
    seeming: str


@dataclass_json
@dataclass
class Hair(_Save):
    length: int = 0
    style: int = 0
    color: int = 0


@dataclass_json
@dataclass
class Antennae(_Save):
    length: int = 0
    style: int = 0
    color: int = 0


@dataclass_json
@dataclass
class Forelock(_Save):
    length: int = 0
    style: int = 0
    color: int = 0


@dataclass_json
@dataclass
class Horns(_Save):
    length: int = 0
    style: int = 0
    color: int = 0


@dataclass_json
@dataclass
class Physiology(_Save):
    sex: int = 0
    skin_color: int = 0
    eye_color: int = 0
    aura_color: int = 0
    distinguishing_feature: int = 0

    def should_save(self) -> bool:
        return bool(self.sex or self.skin_color or self.eye_color or self.distinguishing_feature or self.aura_color)


@dataclass_json
@dataclass
class Genomes(FlagBase):
    pass


@dataclass_json
@dataclass
class Mutations(FlagBase):
    pass


@dataclass_json
@dataclass
class Sensei(_SingleModifier):
    pass


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

    @classmethod
    def deserialize(cls, data: typing.Any, ent):
        o = cls()
        if "skill_slots" in data:
            o.skill_spots = data.pop("skill_slots")
        if "skills" in data:
            for k, v in data.pop("skills"):
                o.skills[k] = Skill.from_dict(v)


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
        return [[k, v] for k, v in self.values.items() if v]

    @classmethod
    def deserialize(cls, data: typing.Any, ent):
        o = cls()
        for k, v in data:
            o.values[k] = v
        return o

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
class ItemType(_SingleModifier):
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

    def export(self):
        return [(int(k), v.to_dict()) for k, v in self.exits.items()]

    @classmethod
    def deserialize(cls, data: typing.Any, ent):
        o = cls()
        for d, ex in data:
            if not ex:
                continue
            e = RoomExit()

            if "description" in ex:
                e.description = Description(ex.pop("description"))
            if "keyword" in ex:
                e.keyword = Name(ex.pop("keyword"))

            for f in ("exit_info", "key", "to_room", "fail_room", "total_fail_room", "dclock", "dchide", "dcskill", "dcmove", "failsavetype", "dcfailsave"):
                if f in ex:
                    setattr(e, f, ex.pop(f))

            o.exits[ExitDir(d)] = e
        return o


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
class SectorType(_SingleModifier):
    pass


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
    objects: dict[Vnum, "Prototype"] = field(default_factory=dict)
    mobiles: dict[Vnum, "Prototype"] = field(default_factory=dict)
    triggers: dict[Vnum, Entity] = field(default_factory=dict)
    shops: dict[Vnum, "Prototype"] = field(default_factory=dict)
    guilds: dict[Vnum, "Prototype"] = field(default_factory=dict)


@dataclass_json
@dataclass
class Price(_Save):
    price: int = 0
    price_per_day: int = 0

    def should_save(self) -> bool:
        return bool(self.price or self.price_per_day)


@dataclass_json
@dataclass
class DupeCheck(_Save):
    generation: int = 0
    unique_id: int = 0


@dataclass_json
@dataclass
class DgScriptID(_Save):
    script_id: int = 0


@dataclass_json
@dataclass
class SpawnRoom(_NoSave):
    room: Entity


class HasCmdHandler(old_cm.HasCmdHandler):

    async def update(self):
        pass