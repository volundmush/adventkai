from libcpp cimport bool
from libc.stdint cimport int64_t, int16_t
from libcpp.string cimport string
from libcpp.list cimport list
from libcpp.set cimport set
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
from libcpp.pair cimport pair

cdef extern from "dbat/net.h" namespace "net":

    cdef cppclass ProtocolCapabilities:
        string protocolName
        string clientName
        string clientVersion
        string hostAddress
        vector[string] hostNames
        int16_t hostPort

    cdef cppclass Connection:
        Connection(int64_t connId)
        int64_t connId
        void queueMessage(const string& event, const string& data)
        list[pair[string, string]] outQueue
        ProtocolCapabilities capabilities
        bool running
        void onHeartbeat(double deltaTime)

    cdef map[int64_t, shared_ptr[Connection]] connections
    cdef set[int64_t] pendingConnections
    cdef set[int64_t] deadConnections

    cdef shared_ptr[Connection] newConnection()