from libc.time cimport time_t
from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
cimport utils

cdef extern from "dbat/structs.h":
    cdef cppclass account_data:
        account_data()
        account_data(const utils.json& j)
        int vn
        string name
        string email
        # string passHash - NOT including passHash here.
        time_t created
        time_t lastLogin
        time_t lastLogout
        time_t lastPasswordChanged
        double totalPlayTime
        string disabledReason
        time_t disabledUntil
        int adminLevel
        int rpp
        int slots
        vector[int] characters

        bool checkPassword(const string& password)
        bool setPassword(const string& password)
        void modRPP(int amt)

        utils.json serialize()
        utils.Document rserialize()
        void deserialize(const utils.json& j)

cdef extern from "dbat/account.h":
    account_data* createAccount(const string& name, const string& password) except+
    account_data* findAccount(const string& name)

    map[int, account_data] accounts