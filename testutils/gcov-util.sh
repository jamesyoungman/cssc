#! /bin/sh

formatter=fmt_text

do_coverage () {
    if [ $# -eq 0 ]
    then
	set x *.o
	shift
	if [ $# -eq 0 ]
	then
	    echo "I'm confused - where are all the object files?" >&2
	    exit 1
	fi
    fi

    for objfile
    do
      gcov $objfile
    done
}

get_file_info () {
    cat "$@" | expand | cut -c1-16 | sed -e 's/^ *//' -e 's/ *$//'
}


do_hits () {
    get_file_info "$@" | egrep -c  "^ *[0-9]+ *$"
}

do_misses () {
    get_file_info "$@" | egrep -c  "^ *###### *$"
}

do_all2 () {
    # set -x
    awk '
function endfile (m, h, of, f) {
   if (length(of)) {
      printf("%10d %10d %s\n", m, h, of);
      misses=0; hits=0;
   }
}

BEGIN { misses=0; hits=0; }
{
  if (FILENAME != oldfile) {
    endfile(misses, hits, oldfile, FILENAME);
    oldfile = FILENAME;
  }
}
END {
    endfile(misses, hits, oldfile, FILENAME);
}
/^ *######/ { ++misses; }
/^ *[0-9]+/ { ++hits; }
' "$@" < /dev/null
}

do_summary () {
    if [ $# -eq 0 ]
    then
	set x *.gcov
	shift
	if [ $# -eq 0 ]
	then
	    echo "I'm confused - did you run the 'coverage' command first?" >&2
	    exit 1
	fi
    fi
    
    do_all2 "$@" | sort -rn | $formatter
    # do_all2 "$@"
}


do_detail () {
    less -j9 "+/######" *.gcov
}


usage () {
exec >&2
cat <<EOF
usage: $0 summary|coverage [object file list]
EOF
exit 1
}


fmt_text () {
    awk '{printf("%10d %10d %s\n", $1, $2, $3);}'
}

fmt_html () {
cat<<EOF    
<table border="1">
<tr><th> Misses </th><th> Hits </th><th> Filename </th></tr>

EOF
    while read m h f
    do
      printf "<tr><td>%d</td><td>%d</td><td>%s</td></tr>\n" \
	  $m $h "$f"
    done

cat <<EOF
</table>
EOF
}


do_f_option () {
    case "$1" in 
	text) formatter=fmt_text ;;
        html) formatter=fmt_html ;;
	*)    echo "Unknown formatting method $1" >&2; usage ;;
    esac
}

do_sub_command () {
    while [ $# -gt 0 ]
    do
      case "$1" in
	  -f)        do_f_option "$2" ; shift 2 ;;
	  summary)   shift; do_summary  "$@" ; exit $? ;;
	  detail)    shift; do_detail   "$@" ; exit $? ;;
	  coverage)  shift; do_coverage "$@" ; exit $? ;;
	  *)         usage ;;
      esac
    done
}

do_sub_command "$@"
