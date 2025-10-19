//
// generated file  --  DO NOT EDIT
//

EventQueue processor::run;
EventQueue processor::requestRescheduleTimer;
EventQueue processor::requestRescheduleInterrupt;
EventQueue processor::checkRequestReschedule;
EventQueue interrupt::run;
EventQueue interrupt::notifyInterrupt;
EventQueue timer::run;
EventQueue timer::processTimeout;

std::vector<Entry> all {
    {"processor", "processor::run"                       , &processor::run},
    {"processor", "processor::requestRescheduleTimer"    , &processor::requestRescheduleTimer},
    {"processor", "processor::requestRescheduleInterrupt", &processor::requestRescheduleInterrupt},
    {"processor", "processor::checkRequestReschedule"    , &processor::checkRequestReschedule},
    {"interrupt", "interrupt::run"                       , &interrupt::run},
    {"interrupt", "interrupt::notifyInterrupt"           , &interrupt::notifyInterrupt},
    {"timer"    , "timer::run"                           , &timer::run},
    {"timer"    , "timer::processTimeout"                , &timer::processTimeout},
};
