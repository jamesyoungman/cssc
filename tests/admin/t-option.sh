#! /bin/sh

# t-option.sh:  Testing for correct operation of admin -t.

# Import common functions & definitions.
. test-common

expands_to () {
    # $1 -- label
    # $2 -- format
    # $3 -- expansion
docommand $1 "${prs} \"-d$2\" s.bar" 0 "$3"
}

remove s.bar 

# Create file with description.
echo "Descriptive Text" > DESC
docommand T1 "${admin} -n -tDESC s.bar" 0 "" ""
remove DESC

# Make sure the decription is there.
expands_to T2 ':FD:'   'Descriptive Text\n\n'


# Remove the description.
docommand T3 "${admin} -t s.bar" 0 "" ""

# Make sure the decription has been removed.
expands_to T4 ':FD:'   '(none)\n\n'

remove s.bar 
success

