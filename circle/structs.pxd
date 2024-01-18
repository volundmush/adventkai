from libcpp cimport bool
from libc.stdint cimport int64_t, int16_t, int8_t
from libc.time cimport time_t
from libcpp.string cimport string
from libcpp.list cimport list
from libcpp.set cimport set
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
cimport accounts
cimport utils

cdef extern from "dbat/structs.h":

    cdef cppclass area_data:
        area_data(const utils.json& j)
        utils.json serialize()
        utils.Document rserialize()

    cdef cppclass player_data:
        player_data(const utils.json& j)
        int64_t id
        string name
        accounts.account_data * account

        utils.json serialize()
        utils.Document rserialize()

    cdef cppclass extra_descr_data:
        char* keyword
        char* description
        extra_descr_data* next

    cdef cppclass unit_data:
        int vn
        int zone
        char* name
        char* room_description
        char* look_description
        char* short_description
        bool exists
        extra_descr_data *ex_description

        vector[int] proto_script
        obj_data* contents

        int64_t id
        time_t generation

    cdef cppclass obj_data(unit_data):
        obj_data(const utils.json& j)
        utils.json serializeInstance()
        utils.Document rserializeInstance()
        utils.json serializeProto()
        utils.Document rserializeProto()
        string serializeLocation()
        utils.json serializeRelations()

        void deserializeLocation(const string& txt, int16_t slot)
        void deserializeRelations(const utils.json& j)

        void deserializeProto(const utils.json& j)
        void deserializeInstance(const utils.json& j, bool isActive)
        void deserializeContents(const utils.json& j, bool isActive)

        int worn_on

    cdef cppclass room_data(unit_data):
        room_data(const utils.json& j)

        utils.json serialize()
        utils.Document rserialize()

    cdef cppclass time_info_data:
        void deserialize(const utils.json& j)
        utils.json serialize()

    cdef cppclass char_data(unit_data):
        char_data(const utils.json& j)
        utils.Document rserializeBase()
        utils.json serializeInstance()
        utils.Document rserializeInstance()
        utils.json serializeProto()
        utils.Document rserializeProto()
        utils.json serializePlayer()
        utils.json serializeLocation()
        utils.json serializeRelations()

        void deserializeProto(const utils.json& j)
        void deserializeInstance(const utils.json& j, bool isActive)
        void deserializeMobile(const utils.json& j)
        void deserializePlayer(const utils.json& j)
        void deserializeLocation(const utils.json& j)
        void deserializeRelations(const utils.json& j)

    cdef cppclass weather_data:
        utils.json serialize()
        void deserialize(const utils.json& j)

    cdef cppclass index_data:
        int vn
        trig_data *proto
        utils.json serializeProto()
        utils.Document rserializeProto()


cdef extern from "dbat/guild.h":
    cdef cppclass guild_data:
        guild_data(const utils.json& j)
        utils.json serialize()
        utils.Document rserialize()


cdef extern from "dbat/shop.h":
    cdef cppclass shop_data:
        shop_data(const utils.json& j)
        utils.json serialize()
        utils.Document rserialize()


cdef extern from "dbat/dg_scripts.h":
    cdef cppclass trig_data:
        trig_data()
        trig_data(const utils.json& j)
        int vn
        char* name
        utils.json serializeProto()
        utils.Document rserializeProto()
        utils.json serializeInstance()
        utils.Document rserializeInstance()
        string serializeLocation()

        void deserializeInstance(const utils.json& j)
        void deserializeLocation(const string& txt)

        int order


cdef extern from "dbat/db.h":
    cdef cppclass zone_data:
        zone_data(const utils.json& j)
        utils.json serialize()
        utils.Document rserialize()