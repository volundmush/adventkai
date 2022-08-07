from snekmud.equip import EquipSlot


class CircleEquip(EquipSlot):
    category = "circle"

    @classmethod
    def is_available(cls, equipper, **kwargs):
        if not (limb := getattr(cls, "limb", None)):
            return True
        if isinstance(limb, str):
            return equipper.limbs.get(limb)
        elif isinstance(limb, list):
            for l in limb:
                if equipper.limbs.get(l):
                    return True
        return False


class _Finger(CircleEquip):
    slot_type = "finger"


class RightRingFinger(_Finger):
    key = "right_ring_finger"
    wear_display = "on $pron(your) right ring finger"
    list_display = "On Right Ring Finger"
    remove_display = "from $pron(your) right ring finger"
    sort_order = 10
    limb = "right_arm"


class LeftRingFinger(_Finger):
    key = "left_ring_finger"
    wear_display = "on $pron(your) left ring finger"
    list_display = "On Left Ring Finger"
    remove_display = "from $pron(your) left ring finger"
    sort_order = 20
    limb = "left_arm"


class Neck1(CircleEquip):
    key = "neck_1"
    wear_display = "around $pron(your) neck"
    remove_display = "from $pron(your) neck"
    slot_type = "neck"
    list_display = "Worn Around Neck"
    sort_order = 30


class Neck2(Neck1):
    key = "neck_2"
    slot_type = "neck"
    list_display = "Worn Around Neck"
    sort_order = 35


class Body(CircleEquip):
    key = "body"
    wear_display = "around $pron(your) body"
    remove_display = "from $pron(your) body"
    slot_type = "body"
    list_display = "Worn On Body"
    sort_order = 40


class Head(CircleEquip):
    key = "head"
    wear_display = "on $pron(your) head"
    remove_display = "from $pron(your) head"
    slot_type = "head"
    list_display = "Worn On Head"
    sort_order = 50


class Legs(CircleEquip):
    key = "legs"
    wear_display = "on $pron(your) legs"
    remove_display = "from $pron(your) legs"
    slot_type = "legs"
    list_display = "Worn On Legs"
    sort_order = 60
    limb = ["right_leg", "left_leg"]


class Feet(CircleEquip):
    key = "feet"
    wear_display = "on $pron(your) feet"
    remove_display = "from $pron(your) feet"
    slot_type = "feet"
    list_display = "Worn On Feet"
    sort_order = 70
    limb = ["right_leg", "left_leg"]


class Hands(CircleEquip):
    key = "hands"
    wear_display = "on $pron(your) hands"
    remove_display = "from $pron(your) hands"
    slot_type = "hands"
    list_display = "Worn On Hands"
    sort_order = 80
    limb = ["right_arm", "left_arm"]


class Arms(CircleEquip):
    key = "arms"
    wear_display = "on $pron(your) arms"
    remove_display = "from $pron(your) arms"
    slot_type = "arms"
    list_display = "Worn On Arms"
    sort_order = 90
    limb = ["right_arm", "left_arm"]


class About(CircleEquip):
    key = "about"
    wear_display = "about $pron(your) body"
    remove_display = "from about $pron(your) body"
    slot_type = "about"
    list_display = "Worn About Body"
    sort_order = 100


class Waist(CircleEquip):
    key = "waist"
    wear_display = "around $pron(your) waist"
    remove_display = "from $pron(your) waist"
    slot_type = "waist"
    list_display = "Worn About Waist"
    sort_order = 110


class RightWrist(CircleEquip):
    key = "right_wrist"
    wear_display = "around $pron(your) right wrist"
    remove_display = "from $pron(your) right wrist"
    slot_type = "wrist"
    list_display = "Worn On Right Wrist"
    sort_order = 120
    limb = "right_arm"


class LeftWrist(CircleEquip):
    key = "left_wrist"
    wear_display = "around $pron(your) left wrist"
    remove_display = "from $pron(your) left wrist"
    slot_type = "wrist"
    list_display = "Worn On Left Wrist"
    sort_order = 125
    limb = "left_arm"


class Wield1(CircleEquip):
    key = "wield_1"
    wear_verb = "$conj(wields)"
    wear_display = "as $pron(your) primary weapon"
    remove_verb = "$conj(stops) using"
    remove_display = "as $pron(your) primary weapon"
    slot_type = "wield"
    list_display = "Wielded"
    sort_order = 130
    limb = "right_arm"


class Wield2(CircleEquip):
    key = "wield_2"
    wear_verb = "$conj(holds)"
    wear_display = "in $pron(your) offhand"
    remove_verb = "$conj(stops) holding"
    remove_display = "in $pron(your) offhand"
    slot_type = "hold"
    list_display = "Offhand"
    sort_order = 135
    limb = "left_arm"


class Back(CircleEquip):
    key = "back"
    wear_display = "on $pron(your) back"
    remove_display = "from $pron(your) back"
    slot_type = "back"
    list_display = "Worn on Back"
    sort_order = 140


class RightEar(CircleEquip):
    key = "right_ear"
    wear_display = "on $pron(your) right ear"
    remove_display = "from pron(your) right ear"
    slot_type = "ear"
    list_display = "Worn on Right Ear"
    sort_order = 150


class LeftEar(CircleEquip):
    key = "left_ear"
    wear_display = "on $pron(your) left ear"
    remove_display = "from $pron(your) left ear"
    slot_type = "ear"
    list_display = "Worn on Left Ear"
    sort_order = 155


class Shoulders(CircleEquip):
    key = "shoulders"
    wear_display = "on $pron(your) shoulders"
    remove_display = "from $pron(your) shoulders"
    slot_type = "shoulders"
    list_display = "Worn on Shoulders"
    sort_order = 160


class Eyes(CircleEquip):
    key = "eyes"
    wear_display = "over $pron(your) eyes"
    remove_display = "from $pron(your) eyes"
    slot_type = "eyes"
    list_display = "Worn Over Eyes"
    sort_order = 170
