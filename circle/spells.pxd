from libcpp cimport bool
from libc.stdint cimport int64_t, int16_t, int8_t
from libcpp.string cimport string
from libcpp.list cimport list
from libcpp.set cimport set
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
cimport accounts

cdef extern from "dbat/spells.h":
    enum:  # Use enum to bring in #define constants
        NUM_CLASSES
        NUM_RACES
        SKILL_TABLE_SIZE

    cdef cppclass spell_info_type:
        int8_t min_position
        int mana_min
        int mana_max
        int mana_change
        int ki_min
        int ki_max
        int ki_change
        int min_level[NUM_CLASSES]  # Fixed-size array
        int routines
        int8_t violent
        int targets
        const char * name
        const char * wear_off_msg
        int race_can_learn[NUM_RACES]  # Fixed-size array
        int skilltype
        int flags
        int save_flags
        int comp_flags
        int8_t can_learn_skill[NUM_CLASSES]  # Fixed-size array
        int spell_level
        int school
        int domain

    cdef spell_info_type spell_info[SKILL_TABLE_SIZE]