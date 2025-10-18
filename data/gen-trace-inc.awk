#!/usr/bin/awk -f

BEGIN {
    print("//")
    print("// generated file  --  DO NOT EDIT")
    print("//")
    print("")
    N = 0
}

/#define/ { next }

/TRACE_DECLARE/ {
    a = index($0, "(");
    b = index($0, ")");
    name = substr($0, a + 1, b - a - 1)
    NAME[N]  = name
    t = length(name)
    if (L < t) L = t
    N++
}

END {
    for(i = 0; i < N; i++) {
        printf("EventQueue %s;\n", NAME[i])
    }
    print("")
    format = sprintf("    {%%%ds, &%%s},\n", -(L + 2))
    print("std::map<const char*, EventQueue*> map {")
    for(i = 0; i < N; i++) {
        # {"a1", a1},
        name = NAME[i]

        printf(format, "\"" name "\"", name)
    }
    print("};")
}