
def _powerstat_get_set(obj, script, field, handler, arg):
    if arg:
        try:
            getattr(obj, handler).mod_current(int(arg))
        except (ValueError, TypeError):
            script.script_log(f"invalid arg for {field}: {arg}")
    return str(getattr(obj, handler).current())


def race(obj, script, arg):
    return obj.race.get().get_name()


def sensei(obj, script, arg):
    return obj.sensei.get().get_name()


def CLASS(obj, script, arg):
    return sensei(obj, script, arg)


def bank(obj, script, arg):
    return stat_get_set(obj, script, "bank", "bank_balance", arg)


def strength(obj, script, arg):
    return stat_get_set(obj, script, "strength", "strength", arg)


STR = strength

def intelligence(obj, script, arg):
    return stat_get_set(obj, script, "intelligence", "intelligence", arg)


INT = intelligence


def constitution(obj, script, arg):
    return stat_get_set(obj, script, "constitution", "constitution", arg)


con = constitution


def agility(obj, script, arg):
    return stat_get_set(obj, script, "agility", "agility", arg)

agi = agility


def wisdom(obj, script, arg):
    return stat_get_set(obj, script, "wisdom", "wisdom", arg)


wis = wisdom


def speed(obj, script, arg):
    return stat_get_set(obj, script, "speed", "speed", arg)


spd = speed
cha = speed


def carry(obj, script, arg):
    return "1" if obj.db.carrying else "0"


def dead(obj, script, arg):
    return "1" if obj.affect_flags.has("Spirit") else "0"


def death(obj, script, arg):
    return str(obj.attributes.get("death_time", 0))


def drag(obj, script, arg):
    return "1" if obj.db.dragging else "0"


def drunk(obj, script, arg):
    return stat_get_set(obj, script, "drunk", "drunk", arg)


def exp(obj, script, arg):
    return stat_get_set(obj, script, "exp", "experience", arg)


def fighting(obj, script, arg):
    if (opp := obj.db.fighting):
        return opp.dbref
    return ""


def gold(obj, script, arg):
    return stat_get_set(obj, script, "gold", "zenni", arg)

zenni = gold
money = gold


def hisher(obj, script, arg):
    return "hisher"


def heshe(obj, script, arg):
    return "heshe"


def himher(obj, script, arg):
    return "himher"


def hitp(obj, script, arg):
    _powerstat_get_set(obj, script, "hitp", "powerlevel", arg)

def hunger(obj, script, arg):
    return stat_get_set(obj, script, "hunger", "hunger", arg)


def is_killer(obj, script, arg):
    if arg:
        match arg.lower():
            case "1" | "on":
                obj.player_flags.set("Killer")
            case "0" | "off":
                obj.player_flags.remove("Killer")

    return "1" if obj.player_flgas.has("Killer") else "0"


def mana(obj, script, arg):
    _powerstat_get_set(obj, script, "mana", "ki", arg)

ki = mana


def maxpowerlevel(obj, script, arg):
    return str(obj.powerlevel.effective())


maxhitp = maxpowerlevel
maxpl = maxpowerlevel


def maxki(obj, script, arg):
    return str(obj.ki.effective())


maxmana = maxki


def maxstamina(obj, script, arg):
    return str(obj.stamina.effective())


def master(obj, script, arg):
    if (ma := obj.db.master):
        return ma.dbref
    return ""


def position(obj, script, arg):
    if arg:
        obj.position.set(arg)


def prac(obj, script, arg):
    stat_get_set(obj, script, "prac", "practices", arg)


def plr(obj, script, arg):
    if arg:
        return "1" if obj.player_flags.has(arg) else "0"
    return "0"


def pref(obj, script, arg):
    if arg:
        return "1" if obj.preference_flags.has(arg) else "0"
    return "0"


def rpp(obj, script, arg):
    if not obj.account:
        return "0"
    return stat_get_set(obj.account, script, "rpp", "rpp", arg)


def sex(obj, script, arg):
    return obj.gender


gender = sex


def size(obj, script, arg):
    if arg and arg.isnumeric():
        obj.size.set(int(arg))
    return str(int(obj.size.get()))


def skill(obj, script, arg):
    return "0"
    #TODO: finish this.


def skillset(obj, script, arg):
    return "0"
    #TODO: Finish this


def stamina(obj, script, arg):
    _powerstat_get_set(obj, script, "move", "stamina", arg)


move = stamina


def thirst(obj, script, arg):
    return stat_get_set(obj, script, "thirst", "thirst", arg)


def vnum(obj, script, arg):
    return str(obj.attributes.get("mob_vnum", -1))


def worn_by(obj, script, arg):
    if obj.db.equipped:
        return obj.db.equipped[1].dbref
    return ""