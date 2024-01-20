from libcpp cimport bool
from libc.stdint cimport int64_t, int16_t, int8_t
from libcpp.pair cimport pair
from libc.time cimport time_t
from libcpp.string cimport string
from libcpp.list cimport list
from libcpp.set cimport set
from libcpp.map cimport map
from libcpp.unordered_map cimport unordered_map
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
cimport structs
cimport utils

ctypedef structs.char_data* char_data_ptr
ctypedef structs.obj_data* obj_data_ptr
ctypedef structs.trig_data* trig_data_ptr

cdef extern from "dbat/db.h":
    structs.time_info_data time_info
    structs.weather_data weather_info

    void load_config()
    void migrate_db()

    map[int, structs.room_data] world
    map[int, structs.zone_data] zone_table
    map[int, structs.area_data] areas

    map[int, structs.index_data] mob_index
    map[int, structs.char_data] mob_proto

    unordered_map[int64_t, pair[time_t, char_data_ptr]] uniqueCharacters

    map[int, structs.index_data] obj_index
    map[int, structs.obj_data] obj_proto

    unordered_map[int64_t, pair[time_t, obj_data_ptr]] uniqueObjects

    map[int, structs.index_data] trig_index
    map[int64_t, pair[time_t, trig_data_ptr]] uniqueScripts


cdef extern from "dbat/guild.h":
    map[int, structs.guild_data] guild_index


cdef extern from "dbat/shop.h":
    map[int, structs.shop_data] shop_index

cdef extern from "dbat/players.h":
    map[int64_t, structs.player_data] players