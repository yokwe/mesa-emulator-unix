#!/usr/bin/awk -f

BEGIN {
    print("//")
    print("// generated file  --  DO NOT EDIT")
    print("//")
    print("")
    N = 0
}

/#define/ { next }

/PERF_DECLARE/ {
    a = index($0, "(");
    b = index($0, ")");
    names = substr($0, a + 1, b - a - 1)
    gsub(" ", "", names)
    split(names, temp, ",")

    GROUP[N] = temp[1];
    NAME[N]  = temp[2];
    t = length(temp[1]) + length(temp[2])
    if (L < t) L = t
    t = length(temp[1])
    if (G < t) G = t;
    N++
}

END {
    format = sprintf("uint64_t %%%ds = 0;\n", -(L+2))
    for(i = 0; i < N; i++) {
        # uint64_t a1 = 0;
        name = GROUP[i] "::" NAME[i]
        printf(format, name)
    }
    print("")
    
    format = sprintf("    {%%%ds, %%%ds, %%s},\n", -(G+2), -(L+4))
    print("std::vector<Entry> all {");
    for(i = 0; i < N; i++) {
        # {"a1", a1},
        name = GROUP[i] "::" NAME[i]

        printf(format, "\"" GROUP[i] "\"", "\"" name "\"", name)
    }
    print("};")
}