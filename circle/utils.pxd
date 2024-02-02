from libcpp.string cimport string

cdef extern from "json.hpp" namespace "nlohmann":
    cdef cppclass json:
        pass

cdef extern from "dbat/utils.h":
    string jdump(const json& j)
    string jdump_pretty(const json& j)
    json jparse(const string& s) except+