cimport comm

def initialize():
    comm.init_log()
    comm.init_locale()
    if not comm.init_sodium():
        raise Exception("Sodium failed to initialize!")
    comm.init_database()
    comm.init_zones()
