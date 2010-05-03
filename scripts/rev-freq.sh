#! /bin.sh

# Show how often the CVS files have been edited. 
# Changes in the lrelease number will mess all this
# up, but we don't have any of those at the moment.

use_existing=false
status=.cvs-status
scale=0

while [ $# -gt 0 ]
do
    case $1 in
    --existing)	
	    use_existing=true
	    echo Using existing statistics.
	    ;;
    --scale=*)
	    scale=$(echo $1| sed s/--scale=//)
	    ;;
    *)
	    echo "usage: $0 [--existing] [--scale=N]" >&2
	    exit 1
	    ;;
    esac
    shift
done

if [ $use_existing = "true" ] && [ -f $status ]
then
    true
else
    rm -f $status
    cvs status 2>/dev/null > $status || { echo \"cvs status\" failed.  ; exit 1; }
fi

grep "Repository revision" < $status | 
    cut -d: -f2 | 
    awk '{print $1, $2;}' > .ver-status

# Show which files have been edited most...
# sort -r -n -t. +1 < .ver-status 


# Show the distributon of changes in buckets.
awk -v scale=${scale} < .ver-status '
{ ++buckets[$1]; }

END {
    if (scale == 0) {
	max = 1;
	for (b in buckets) if (buckets[b] > max) max = buckets[b];
	scale = max / 60;
    }
    printf("scaling=%g\n", 1.0/scale);

    for (b in buckets) {
	printf("%-6s:", b);
	n = buckets[b] / scale;
	for (i=1; i<n; ++i) {
	    printf("#");
	    if (i > 60) {
		printf("+");
		break;
	    }
	}
	printf("\n");
    }
}' | sort -n -t. +1

# rm -f $status .ver-status
