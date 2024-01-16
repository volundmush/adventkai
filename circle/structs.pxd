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

cdef extern from "dbat/structs.h":
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
        pass