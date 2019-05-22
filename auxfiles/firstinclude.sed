/^ *# *include ["<].*[">]/ {
s/^ *# *include ["<]\(.*\)[">]/\1/
t YESREALLY
i\
Internal error - regexes are inconsistent
q
: YESREALLY
#s/^\(config\|cssc\).h$//
s/^config\.h$//
t OK
=
i\
config.h should be #included first
: OK
# q with an exit code is a GNU extension, don't use it.
q
}
