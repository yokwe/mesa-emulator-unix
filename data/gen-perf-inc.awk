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
    N++
}

END {
    for(i = 0; i < N; i++) {
        # uint64_t a1 = 0;
        name = "perf::" GROUP[i] "::" NAME[i]
        printf("uint64_t %-30s = 0;\n", name)
    }
    print("")

    print("namespace perf {")
    print("std::vector<perf::Entry> all {");
    for(i = 0; i < N; i++) {
        # {"a1", a1},
        name = GROUP[i] "::" NAME[i]

        printf("    {%-24s, %s},\n", "\"" name "\"", "perf::" name)
    }
    print("};")
    print("}")
}