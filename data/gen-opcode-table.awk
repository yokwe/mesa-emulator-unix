#!/usr/bin/awk -f

BEGIN {
    MN = 0
    EN = 0
}

function octal_to_decimal(oct_str) {
    decimal_val = 0
    power = 1
    # Iterate through the string from right to left
    for (i = length(oct_str); i >= 1; i--) {
        digit = substr(oct_str, i, 1)
        # Convert character digit to its numeric value
        digit_val = digit - "0"
        # Add to decimal_val, multiplying by the appropriate power of 8
        decimal_val += digit_val * power
        power *= 8
    }
    return decimal_val
}

# /* 0026 */ DECL_M(1, LLD8)
/\/* 0/ && /DECL_M/ {
    a = index($0, " ");
    b = index($0, " */")

    c = index($0, ", ")
    d = index($0, ")")

    e = index($0, "(")
    f = index($0, ", ")

    code = substr($0, a + 1, b - a - 1)
    num  = octal_to_decimal(code)
    name = substr($0, c + 2, d - c - 2)
    size = substr($0, e + 1, f - e - 1)

    MOP[MN++]="{\"type\": \"mop\", \"code\": " num ", \"length\":" size ", \"name\":\"" name "\"}"
}
# /* 0060 */ DECL_E(2, DMUL)
/\/* 0/ && /DECL_E/ {
    a = index($0, " ");
    b = index($0, " */")

    c = index($0, ", ")
    d = index($0, ")")

    e = index($0, "(")
    f = index($0, ", ")

    code = substr($0, a + 1, b - a - 1)
    num  = octal_to_decimal(code)
    name = substr($0, c + 2, d - c - 2);
    size = substr($0, e + 1, f - e - 1)

    ESC[EN++]="{\"type\": \"esc\", \"code\": " num ", \"length\":" size ", \"name\":\"" name "\"}"
}

END {
    print("[")
    for(i = 0; i < MN; i++) {
        print(MOP[i] ",")
    }
    for(i = 0; i < EN; i++) {
        print(ESC[i] ",")
    }
    print("]")

}