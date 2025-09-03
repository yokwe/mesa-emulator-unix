#!/usr/bin/awk -f

/#define/ { next }

/PERF_DECLARE/ {
    a = index($0, "(");
    b = index($0, ")");
    name = substr($0, a + 1, b - a - 1)
    NAME[N++] = name;
}

BEGIN {
    print("//")
    print("// generated file  --  DO NOT EDIT")
    print("//")
    print("")
}

END {
    for(i = 0; i < N; i++) {
        # uint64_t a1 = 0;
        printf("uint64_t %-20s = 0;\n", "perf::" NAME[i])
    }
    print("")

    print("namespace perf {")
    print("std::vector<perf::Entry> all {");
    for(i = 0; i < N; i++) {
        # {"a1", a1},
        printf("    {%-16s, %s},\n", "\"" NAME[i] "\"", "perf::" NAME[i])
    }
    print("};")
    print("}")
}