//
// generated file  --  DO NOT EDIT
//

EventQueue guam;
EventQueue processor;
EventQueue requestRescheduleTimer;
EventQueue requestRescheduleInterrupt;
EventQueue checkRequestReschedule;
EventQueue interrupt;
EventQueue notifyInterrupt;
EventQueue timer;
EventQueue processTimeout;

std::map<const char*, EventQueue*> map {
    {"guam"                      , &guam},
    {"processor"                 , &processor},
    {"requestRescheduleTimer"    , &requestRescheduleTimer},
    {"requestRescheduleInterrupt", &requestRescheduleInterrupt},
    {"checkRequestReschedule"    , &checkRequestReschedule},
    {"interrupt"                 , &interrupt},
    {"notifyInterrupt"           , &notifyInterrupt},
    {"timer"                     , &timer},
    {"processTimeout"            , &processTimeout},
};
