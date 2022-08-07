from .base import Modifier as _BaseMod


class _RoomFlag(_BaseMod):
    category = "RoomFlags"
    modifier_id = -1
    planet = False


class Dark(_RoomFlag):
    modifier_id = 0

    def is_providing_darkness(self, obj) -> bool:
        return True


class Death(_RoomFlag):
    modifier_id = 1


class NoMob(_RoomFlag):
    modifier_id = 2


class Indoors(_RoomFlag):
    modifier_id = 3


class Peaceful(_RoomFlag):
    modifier_id = 4


class Soundproof(_RoomFlag):
    modifier_id = 5


class NoTrack(_RoomFlag):
    modifier_id = 6


class NoInstant(_RoomFlag):
    modifier_id = 7


class Tunnel(_RoomFlag):
    modifier_id = 8


class Private(_RoomFlag):
    modifier_id = 9


class GodRoom(_RoomFlag):
    modifier_id = 10


class House(_RoomFlag):
    modifier_id = 11


class HouseCrash(_RoomFlag):
    modifier_id = 12


class Atrium(_RoomFlag):
    modifier_id = 13


class OLC(_RoomFlag):
    modifier_id = 14


class BFSMark(_RoomFlag):
    modifier_id = 15


class Vehicle(_RoomFlag):
    modifier_id = 16


class Underground(_RoomFlag):
    modifier_id = 17


class Current(_RoomFlag):
    modifier_id = 18


class TimedDeathtrap(_RoomFlag):
    modifier_id = 19


class Earth(_RoomFlag):
    modifier_id = 20
    planet = True


class Vegeta(_RoomFlag):
    modifier_id = 21
    planet = True


class Frigid(_RoomFlag):
    modifier_id = 22
    planet = True


class Konack(_RoomFlag):
    modifier_id = 23
    planet = True


class Namek(_RoomFlag):
    modifier_id = 24
    planet = True


class Neo(_RoomFlag):
    modifier_id = 25


class Afterlife(_RoomFlag):
    modifier_id = 26


class Space(_RoomFlag):
    modifier_id = 27


class Hell(_RoomFlag):
    modifier_id = 28


class Regen(_RoomFlag):
    modifier_id = 29


class RHell(_RoomFlag):
    modifier_id = 30


class GravityX10(_RoomFlag):
    modifier_id = 31


class Aether(_RoomFlag):
    modifier_id = 32
    planet = True


class HBTC(_RoomFlag):
    modifier_id = 33


class Past(_RoomFlag):
    modifier_id = 34


class ClanBank(_RoomFlag):
    modifier_id = 35


class Ship(_RoomFlag):
    modifier_id = 36


class Yardrat(_RoomFlag):
    modifier_id = 37
    planet = True


class Kanassa(_RoomFlag):
    modifier_id = 38
    planet = True


class Arlia(_RoomFlag):
    modifier_id = 39
    planet = True


class Aura(_RoomFlag):
    modifier_id = 40


class EarthOrbit(_RoomFlag):
    modifier_id = 41


class FrigidOrbit(_RoomFlag):
    modifier_id = 42


class KonackOrbit(_RoomFlag):
    modifier_id = 43


class NamekOrbit(_RoomFlag):
    modifier_id = 44


class VegetaOrbit(_RoomFlag):
    modifier_id = 45


class AetherOrbit(_RoomFlag):
    modifier_id = 46


class YardratOrbit(_RoomFlag):
    modifier_id = 47


class KanassaOrbit(_RoomFlag):
    modifier_id = 48


class ArliaOrbit(_RoomFlag):
    modifier_id = 49


class Nebulae(_RoomFlag):
    modifier_id = 50


class Asteroid(_RoomFlag):
    modifier_id = 51


class Wormhole(_RoomFlag):
    modifier_id = 52


class Station(_RoomFlag):
    modifier_id = 53


class Star(_RoomFlag):
    modifier_id = 54


class Cerria(_RoomFlag):
    modifier_id = 55
    planet = True


class CerriaOrbit(_RoomFlag):
    modifier_id = 56


class Bedroom(_RoomFlag):
    modifier_id = 57


class Workout(_RoomFlag):
    modifier_id = 58


class Garden1(_RoomFlag):
    modifier_id = 59


class Garden2(_RoomFlag):
    modifier_id = 60


class Fertile1(_RoomFlag):
    modifier_id = 61


class Fertile2(_RoomFlag):
    modifier_id = 62


class Fishing(_RoomFlag):
    modifier_id = 63


class FishFresh(_RoomFlag):
    modifier_id = 64


class CanRemodel(_RoomFlag):
    modifier_id = 65