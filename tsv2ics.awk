#!/usr/bin/awk -f

function isleap(year)
{
	return (year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0)
}

function mdays(mon, year)
{
	return (mon == 2) ? (28 + isleap(year)) : (30 + (mon + (mon > 7)) % 2)
}

# Split the time in seconds since epoch into a table, with fields
# named as with gmtime(3): tm["year"], tm["mon"], tm["mday"],
# tm["hour"], tm["min"], tm["sec"]
function gmtime(sec, tm,
	s)
{
	tm["year"] = 1970
	while (sec >= (s = 86400 * (365 + isleap(tm["year"])))) {
		tm["year"]++
		sec -= s
	}
	tm["mon"] = 1
	while (sec >= (s = 86400 * mdays(tm["mon"], tm["year"]))) {
		tm["mon"]++
		sec -= s
	}
	tm["mday"] = 1
	while (sec >= (s = 86400)) {
		tm["mday"]++
		sec -= s
	}
	tm["hour"] = 0
	while (sec >= 3600) {
		tm["hour"]++
		sec -= 3600
	}
	tm["min"] = 0
	while (sec >= 60) {
		tm["min"]++
		sec -= 60
	}
	tm["sec"] = sec
}

BEGIN {
	FS = "\t"

	DTSTART["VEVENT"] = "DTSTART"
	DTEND["VEVENT"] = "DTEND"

	DTEND["VTODO"] = "DUE"

	DTSTART["VJOURNAL"] = "DTSTAMP"

	DTSTART["VFREEBUSY"] = "DTSTART"
	DTEND["VFREEBUSY"] = "DTEND"

	DTSTART["VALARM"] = "DTSTART"

	print "BEGIN:VCALENDAR"
	print "VERSION:2.0"
	print "CALSCALE:GREGORIAN"
	print "METHOD:PUBLISH"
}

NR == 1 {
	if ($1 != "TYPE" || $2 != "START" || $3 != "END" || $4 != "RECUR") {
		print "tsv2ics: invalid column names on first line" >"/dev/stderr"
		exit(EXIT = 1)
	}
	for (i = 1; i <= NF; i++) {
		FIELD[$i] = i
		NAME[i] = $i
	}
	next
}

{
	type = $FIELD["TYPE"]
	print "BEGIN:"type

	if (type in DTSTART) {
		gmtime($FIELD["START"] + offset, tm)
		printf "%s:%04d%02d%02dT%02d%02d00Z\n", DTSTART[type],
		  tm["year"], tm["mon"], tm["mday"], tm["hour"], tm["min"]
	}

	if (type in DTEND) {
		gmtime($FIELD["END"] + offset, tm)
		printf "%s:%04d%02d%02dT%02d%02d00Z\n", DTEND[type],
		  tm["year"], tm["mon"], tm["mday"], tm["hour"], tm["min"]
	}

	for (i = 5; i in NAME; i++)
		print$NAME[i]":"$i

	print "END:"type
}

END {
	if (EXIT) exit(EXIT)
	print ""
	print "END:VCALENDAR"
}
