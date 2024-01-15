
cdef extern from "dbat/comm.h" namespace "game":
    void init_locale()
    bool init_sodium()
    void init_database()
    void init_zones()
    void run_loop_once(double deltaTime)