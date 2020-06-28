ics2txt
=======
Set of tools to work with the popular iCalendar format and converting to even
simpler TSV and text forms.

The current implementation uses [awk](//josuah.net/wiki/awk/) scripts, but a
rather complete implementation of iCalendar, without memory leak or crash, is
already there, and used for the `ics2tree` linting tool.

Plans include to have an `ics2json` tool for a 1:1 mapping of iCalendar, and
have `ics2tsv` a more general-purpose tool with user-chosen column fields.

So far, Awk-based parsing have been tested with the following input formats
(sample account created for testing):

* Zoom meetings generated events
* FOSDEM events, like <https://fosdem.org/2020/schedule/ical>
* Google Calendar
