/* ************************************************************************
*  File: dg_event.h                                                       *
*                                                                         *
*  Usage: structures and prototypes for events                            *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Changed return type of EVENTFUNC from void to long.   *
*                   Moved struct event definition to events.c.            *
*  $Author: Mark A. Heilpern/egreen/Welcor $                              *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
************************************************************************ */
#pragma once
#include "structs.h"

extern std::set<struct trig_data*> triggers_waiting;

/********** Event related section *********/
/* - events - function protos need by other modules */
extern void event_process(uint64_t heart_pulse, double deltaTime);
