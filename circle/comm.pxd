from libcpp cimport bool

cdef extern from "dbat/comm.h" namespace "game":
    void init_log()
    void init_locale()
    bool init_sodium()
    void init_database()
    void init_zones()
    void run_loop_once(double deltaTime)

cdef extern from "dbat/comm.h":
    void migrate_db()