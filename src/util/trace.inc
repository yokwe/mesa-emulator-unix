//
// generated file  --  DO NOT EDIT
//

EventQueue guam;
EventQueue processor;
EventQueue interrupt;
EventQueue timer;

std::map<const char*, EventQueue*> map {
    {"guam"     , &guam},
    {"processor", &processor},
    {"interrupt", &interrupt},
    {"timer"    , &timer},
};
