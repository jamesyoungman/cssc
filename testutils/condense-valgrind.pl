#! /usr/bin/perl 


my %ignore;
sub i {
    $ignore{$_[0]} = 1;
}

i '/bin/bash' ;
i '/bin/cat' ;
i '/bin/chmod' ;
i '/bin/cp' ;
i '/bin/echo' ;
i '/bin/egrep' ;
i '/bin/false' ;
i '/bin/grep' ;
i '/bin/ln' ;
i '/bin/mkdir' ;
i '/bin/mv' ;
i '/bin/rm' ;
i '/bin/sed' ;
i '/bin/true' ;
i '/bin/uname' ;
i '/bin/uncompress' ;
i '/lib/ld' ;
i '/lib/libc' ;
i '/lib/libdl' ;
i '/lib/libm' ;
i '/lib/libncurses.so' ;
i '/lib/libnsl' ;
i '/lib/libnss_compat' ;
i '/usr/bin/basename' ;
i '/usr/bin/diff' ;
i '/usr/bin/expr' ;
i '/usr/bin/head' ;
i '/usr/bin/make' ;
i 'make' ;
i '/usr/bin/pr' ;
i '/usr/bin/tail' ;
i '/usr/bin/tr' ;
i '/usr/bin/wc' ;
i 'libstdc' ;


my $program = "";
my $in_entry = 0;
my $pid = undef;
my $ignore_until = "";
my $start_regexp = "^valgrind-[0-9]+";
my $ignore_count = 0;
my $line = 1;

sub end_entry {
}


while (<>) {
    ++$line;

    s/^==([0-9]*)== //;
    $pid=$1;

    if ($ignore_count gt 0) {
	--$ignore_count;
	next;
    }
    
    if ($ignore_until ne "") {
	# print "Checking for ignore ($ignore_until)...\n";
	if (m/$ignore_until/) {
	    # print "Found match, stopped ignoring...\n";
	    $ignore_until = "";
	} else {
	    # print "Ignored this line...\n";
	    next;
	}
    } else {
	# print "Not ignoring...\n";
    }
    
    # printf("%4d [%s] -> %s", $line, $program, $_);


    if (m/^valgrind-[0-9]*.*a memory/) {
	&end_entry;
	$in_entry = 1;
	$program = "";
	$ignore_until = "^Reading syms from ";
	# print "!> end of entry\n";
	# next;
    } elsif ( ($program eq "") && (m/^Reading syms from (.*)$/)) {
	
	$program = $1;
	# $program =~ s/.*\///;
	# print "program = " . $program . "\n";
	
	if ($ignore{$1}) {
	    # print "ignoring...\n";
	    # print "Ignoring until next /$start_regexp/\n";
	    $ignore_until = $start_regexp;
	} else {
	    # print "NOT ignoring...\n";
	    print "The program " . $program . " is interesting...\n";
	}
    } else {
	printf("== %d == %s", $pid, $_);
    }

    
}
if ($in_entry) {
    &end_entry;
}
