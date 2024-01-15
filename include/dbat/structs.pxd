ctypedef int vnum
from libcpp cimport bool
from libcpp.string cimport string

cdef extern from "structs.h":
    cdef cppclass account_data:
        vnum vn
        string name
        string email
        int adminLevel
        bool checkPassword(const string& password)
        bool setPassword(const string& password)