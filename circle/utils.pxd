from libcpp.string cimport string

cdef extern from "nlohmann/json.hpp" namespace "nlohmann":
    cdef cppclass json:
        pass

cdef extern from "rapidjson/document.h" namespace "rapidjson":
    cdef cppclass Document:
        pass

    cdef cppclass Value:
        pass

cdef extern from "dbat/utils.h":
    string jdump(const json& j)
    string jdump_pretty(const json& j)
    json jparse(const string& s) except+

    string rjdump(const Value& j)
    string rjdump_pretty(const Value& j)
    string rjdump(const Document& j)
    string rjdump_pretty(const Document& j)
    Document rjparse(const string& s)