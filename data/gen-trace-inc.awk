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
    group = substr($0, a + 1, b - a - 1)
    
    GROUP[N] = group
    all[group] = 0
    N++
    t = length(group)
    if (G < t) G = t
}

END {
    for(i = 0; i < N; i++) {
        printf("EventQueue %s;\n", GROUP[i])
    }
    print("")
    format = sprintf("    {%%%ds, &%%s},\n", -(G + 2))
    print("std::vector<Entry> all {")
    for(i = 0; i < N; i++) {
        group = GROUP[i]
        printf(format, "\"" group "\"", group)
    }
    print("};")
}