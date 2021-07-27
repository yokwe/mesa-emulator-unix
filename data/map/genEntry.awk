#

BEGIN {
	RS="\r";
	FS="[ ]+";
}

#    Bytes   EVI  Offset    IPC   Module               Procedure
#    4     6   1020B     10B  ProcessorHeadGuam    CountCSBanks
#    2     3   4         5    6                    7
NF==7 && $7 != "<nested>" {
#	printf("<!-- %s -->\n", $0);
#	printf("<entry bytes=\"%s\" evi=\"%s\" offset=\"%s\" ipc=\"%s\" module=\"%s\" name=\"%s\" />\n", $2, $3, $4, $5, $6, $7);
#	printf("<entry pc=\"%s\" module=\"%s\" name=\"%s\" />\n", $5, $6, $7);
	printf("<entry %-14s  %-34s %s />\n", sprintf("pc=\"%s\"", $5), sprintf("module=\"%s\"", $6), sprintf("proc=\"%s\"", $7));
}

#    Bytes   EVI  Offset    IPC   Module               Procedure
#    10B   EV   1010B          ProcessorHeadGuam
#    2     3    4              5
#NF==5 && $3=="EV" {
#	printf("<!-- %s -->\n", $0);
#	printf("<entry bytes=\"%s\" evi=\"%s\" offset=\"%s\" ipc=\"%s\" module=\"%s\" name=\"%s\" />\n", $2, $3, $4, "", $5, "");
#}
