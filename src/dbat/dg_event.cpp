#include "dbat/dg_event.h"
#include "dbat/dg_scripts.h"
#include "dbat/utils.h"

std::set<struct trig_data*> triggers_waiting;

/* Process any events whose time has come. */
void event_process(uint64_t heart_pulse, double deltaTime) {
    // copy the queue to avoid any issues with the queue being modified while we're processing it
    auto queued = triggers_waiting;

    for (auto trig : queued) {
        trig->waiting -= deltaTime;
        if(trig->waiting <= 0) {
            triggers_waiting.erase(trig);
            trig->waiting = 0.0;
            switch(trig->owner.index()) {
                case 0:
                    script_driver(&std::get<0>(trig->owner), trig, WLD_TRIGGER, TRIG_RESTART);
                    break;
                case 1:
                    script_driver(&std::get<1>(trig->owner), trig, OBJ_TRIGGER, TRIG_RESTART);
                    break;
                case 2:
                    script_driver(&std::get<2>(trig->owner), trig, MOB_TRIGGER, TRIG_RESTART);
                    break;
            }
        }
    }
}
