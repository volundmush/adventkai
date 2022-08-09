

def cost(obj, script, arg):
    return stat_get_set(obj, script, "cost", "price", arg)

price = cost

def cost_per_day(obj, script, arg):
    return stat_get_set(obj, script, "cost_per_day", "price_per_day", arg)

price_per_day = cost_per_day

def carried_by(obj, script, arg):
    if obj.location and obj.location.obj_type == "character":
        return obj.location.dbref
    return ""

from .shared import inventory as _inv

contents = _inv


def extra(obj, script, arg):
    if arg:
        return "1" if obj.extra_flags.has(arg) else "0"
    return "0"


def shortdesc(obj, script, arg):
    if arg:
        obj.aliases.clear()
        obj.aliases.add(arg)
    else:
        return obj.get_display_name(looker=script.handler.owner)


def size(obj, script, arg):
    if arg and arg.isnumeric():
        obj.size.set(int(arg))
    return str(int(obj.size.get()))


def type(obj, script, arg):
    return obj.item_type.get().mod_id


def _val(obj, v):
    if obj.db.item_values:
        return str(obj.db.item_values.get(v, 0))
    return "0"


def val0(obj, script, arg):
    return _val(obj, 0)


def val1(obj, script, arg):
    return _val(obj, 1)


def val2(obj, script, arg):
    return _val(obj, 2)


def val3(obj, script, arg):
    return _val(obj, 3)


def val4(obj, script, arg):
    return _val(obj, 4)


def val5(obj, script, arg):
    return _val(obj, 5)


def val6(obj, script, arg):
    return _val(obj, 6)


def val7(obj, script, arg):
    return _val(obj, 7)


def val8(obj, script, arg):
    return _val(obj, 8)





def vnum(obj, script, arg):
    return str(obj.attributes.get("item_vnum", -1))