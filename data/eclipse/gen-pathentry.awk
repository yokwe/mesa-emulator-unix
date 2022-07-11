#!/bin/awk

$1=="INCPATH" && $2=="=" {
	for(i = 3; i <= NF; i++) {
		if (substr($i, 1, 2) == "-I") {
			path=substr($i, 3)
#			printf("XX %s\n", path)
			if (substr(path, 1, 1) == "/") {
				# <pathentry include="/opt/local/include" kind="inc" path="" system="true"/>
				printf("<pathentry include=\"%s\" kind=\"inc\" path=\"\" system=\"true\"/>\n", path)
			}
		}
	}
}