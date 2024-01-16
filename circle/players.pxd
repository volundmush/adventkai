from libcpp cimport bool
from libc.stdint cimport int64_t, int16_t
from libcpp.string cimport string
from libcpp.list cimport list
from libcpp.set cimport set
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
cimport accounts

cdef extern from "dbat/structs.h":
    cdef cppclass player_data:
        int64_t id
        string name
        accounts.account_data* account