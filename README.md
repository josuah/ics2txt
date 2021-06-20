ics2txt
=======
Set of tools to work with the popular iCalendar format and converting to even
simpler TSV and text forms.

* `ics2tsv` converts the iCalendar data to an easier-to-parse TSV format.
* `ics2tree` lints exhaustively all iCalendar content for inspection.
* `tsv2ics` convert back the TSV format to iCalendar.
* `tsv2agenda` displays converted TSV data to readable output.

An ical.c/ical.h library walks through the icalendar structure and is
used by the various `ics2*` parsing tools above.

So far, parsing have been tested with the following inputs:

* Zoom meetings generated events
* FOSDEM events, like <https://fosdem.org/2020/schedule/ical>
* Google Calendar
* L'agenda du Libre: <https://www.agendadulibre.org/events.ics>
