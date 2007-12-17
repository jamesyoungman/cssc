#! /usr/bin/perl -w
#
# Copyright (C) 2001 Richard Kettlewell
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use integer;
use IO::File;
use IO::Handle;

# configurable items:

# root of SCCS files
my $root = "/cdrom";
# allowed filters
my @allowed_filters = qw(raw diff);
# add "man2html" if you trust groff

my $tmproot;
my $tmpcount = 0;

my %allowed_filters = map(($_ => 1), @allowed_filters);

sub decode( $ );
sub quote( $ );
sub directory( $ );
sub regfile( $ );
sub dirent( $$ );
sub tempfile();
sub filter( $$$ );
sub save( $ );
sub copy( $$ );

my @query = split(/\&/, $ENV{'QUERY_STRING'});
my %q = ();
for(@query) {
  if(/^([^=]+)=(.*)$/) {
    $q{decode($1)} = decode($2);
  }
}

my $path = $ENV{'PATH_INFO'} || "";
error("invalid path \"$path\"") if $path =~ /\.\./;

if(-d "$root/$path" && $path !~ /\/$/) {
  # make sure directory URLs always end in /
  my $url = "$ENV{'REQUEST_URI'}/";
  # REQUEST_URI might or might not include http://....
  if($url !~ /^http:/) {
    if($ENV{'SERVER_PORT'} eq "80") {
      $url = "http://$ENV{'SERVER_NAME'}$url";
    } else {
      $url = "http://$ENV{'SERVER_NAME'}:$ENV{'SERVER_PORT'}$url";
    }
  }
  output("Location: $url\n");
  html("Redirect",
       "<p>redirect to <a href=\"", quote($url), "\">", quote($url), "</a></p>\n");
} else {
  stat "$root/$path";
  if (-d _) {
    directory($path);
  } elsif (-f _) {
    regfile($path);
  } else {
    error("\"$path\" is not a valid path");
  }
}
(close STDOUT) || die "$0: stdout: $!";

exit 0;

sub directory( $ ) {
  opendir(D, "$root/$path")
    || error("opening $path: $!");
  my @files = readdir D;
  closedir D;
  my @bgcolors = ("#ffffff", "#c0ffc0");
  my $n = 0;
  html("SCCS: $path",
       "<table cellspacing=0 cellpadding=2>\n",
       " <tr bgcolor=#f0f0f0>\n",
       "  <th align=left>Filename</th>\n",
       "  <th align=left>Type</th>\n",
       "  <th align=left>User</th>\n",
       "  <th align=left>Rev</th>\n",
       "  <th align=left>Date</th>\n",
       "  <th align=left>Comment</th>\n",
       " </tr>\n",
       ($path ne "/"
	? (" <tr bgcolor=" . $bgcolors[$n++%2] .">\n",
	   dirent($path, ".."),
	   " </tr>\n",
	  )
	: ()),
       map((
	    " <tr bgcolor=" . $bgcolors[$n++%2] .">\n",
	    dirent($path, $_),
	    " </tr>\n",
	   ), grep { $_ ne "." && $_ ne ".." } sort @files),
       "</table>\n");
}

sub dirent( $$ ) {
  my $path = shift;
  my $file = shift;

  my $link;
  if($file eq "..") {
    # parent directory is done specially
    $link = "$ENV{'SCRIPT_NAME'}$path";
    $link =~ s/[^\/]+\/$//;
  } else {
    # include the trailing / on directories, to save a redirect
    stat "$root/$path/$file";
    $link = -d _ ? "$file/" : $file;
  }
  lstat "$root/$path/$file";
  my $type = (-l _ ? "link"
	      : -d _ ? "dir"
	      : -f _ ? "file"
	      : "?");
  my $who = "&nbsp;";
  my $id = "&nbsp;";
  my $date = "&nbsp;";
  my $comment = "&nbsp;";
  if($type eq "file") {
    my $prs = `sccs prs -d\Q:P: :I: :D: :C:\E \Q$root/$path/$file\E`;
    ($who, $id, $date, $comment) = map(quote($_),
				       ($prs =~ /^(\S+) (\S+) (\S+) (.*)/))
      if !$?;
  }
  return ("  <td>\n",
	  "   <a href=\"",
	  quote($link),
	  "\">",
	  quote($file),
	  "</a>\n",
	  "  </td>\n",
	  "  <td>$type</td>\n",
	  "  <td>", $who, "</td>\n",
	  "  <td>", $id, "</td>\n",
	  "  <td>", $date, "</td>\n",
	  "  <td>", $comment, "</td>\n",
	 );
}

sub regfile( $ ) {
  my $path = shift;
  local $_;
  (my $file = $path) =~ s/^.*\///;

  if(exists $q{'r'}) {
    # output file contents
    (my $sccs = new IO::File("sccs get -p -r\Q$q{'r'}\E \Q$root/$path\E|"))
      || die "$0: executing sccs command: $!";
    filter($q{'f'} || "raw", $sccs, *STDOUT);
    $sccs->close()
      || die "$0: sccs error: $?/$!";
  } elsif(exists $q{'d'}) {
    # output a diff
    (my $sccs = new IO::File("sccs sccsdiff -r\Q$q{'d'}\E -r\Q$q{'e'}\E -u \Q$root/$path\E|"))
      || die "$0: executing sccs command: $!";
    filter($q{'f'} || "raw", $sccs, *STDOUT);
    $sccs->close()
      || die "$0: sccs error: $?/$!";
  } else {
    # prs output
    my @prs = `sccs prs \Q$root/$path\E`;
    if($?) {
      html("SCCS: $path",
	   "<p>Cannot get any SCCS information for ", quote($path), "</p>\n");
      return;
    }
    my @revs = ();
    my %when = ();
    my %who = ();
    my %comments = ();
    my $rev;
    my $incomment = 0;
    for(@prs) {
      if(/^D ([0-9\.]+) (\d+\/\d+\/\d+ \d+:\d+:\d+) (\S+)/) {
	$rev = $1;
	push(@revs, $rev);
	$when{$rev} = quote($2);
	$who{$rev} = quote($3);
	$incomment = 0;
      } elsif(/^COMMENTS:/) {
	$incomment = 1;
      } elsif($incomment) {
	if(exists $comments{$rev}) {
	  $comments{$rev} .= "<br>" . quote($_);
	} else {
	  $comments{$rev} = quote($_);
	}
      }
    }
    my @bgcolors = ("#ffffff", "#c0ffc0");
    my $n = 0;
    html("SCCS: $path",
	 "<table cellspacing=0 cellpadding=2>\n",
	 " <tr bgcolor=#f0f0f0>\n",
	 "  <th align=left>Rev</th>\n",
	 "  <th align=left>&nbsp;</th>\n",
	 "  <th align=left>User</th>\n",
	 "  <th align=left>When</th>\n",
	 "  <th align=left>Comments</th>\n",
	 " </tr>\n",
	 map((
	      " <tr bgcolor=" . $bgcolors[$n++%2] .">\n",
	      "  <td valign=top>",
	      "<a href=\"", quote("$file?r=$revs[$_]"), "\">$revs[$_]</a>",
	      "</td>\n",
	      "  <td valign=top>",
	      $_ != $#revs
	      ? (" (<a href=\"",
		 quote("$file?d=$revs[$_+1]&e=$revs[$_]&f=diff"),
		 "\">diff</a>)",
		)
	      : "&nbsp;",
	      $path =~ /\.\d+$/ && exists $allowed_filters{'man2html'}
	      ? (" (<a href=\"",
		 quote("$file?r=$revs[$_]&f=man2html"),
		 "\">format</a>)",
		)
	      : (),
	      "</td>\n",
	      "  <td valign=top>$who{$revs[$_]}</td>\n",
	      "  <td valign=top>$when{$revs[$_]}</td>\n",
	      "  <td valign=top>$comments{$revs[$_]}</td>\n",
	      " </tr>\n",
	     ), 0 .. $#revs),
	 "</table>\n",
	 "<form method=GET>\n",
	 " <p>Revisions: ",
	 "  <input type=text name=d>\n",
	 "  <input type=text name=e>\n",
	 "  <input type=submit name=f value=Diff>\n",
	 " </p>\n",
	 "</form>\n",
	);
  }
}

sub decode( $ ) {
  local $_ = shift;
  s/\+/ /g;
  s/%(..)/chr(hex($1))/ge;
  return $_;
}

sub error {
  html("Error", "<p>", map(quote($_), @_), "</p>\n");
}

sub html {
  my $title = quote(shift);
  output("Content-Type: text/html\n",
	 "\n",
	 "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n",
	 "<html>\n",
	 " <head>\n",
	 "  <title>$title</title>\n",
	 " </head>\n",
	 " <body bgcolor=#ffffff text=#000000 link=#0000ff vlink=#ff0000 alink=#ff00ff>\n",
	 "  <h1>$title</h1>\n",
#	 map(("<p>", quote($_), "=", quote($ENV{$_}), "</p>\n"),
#	     sort keys %ENV),
	 @_,
	 " </body>\n",
	 "</html>\n",
	 "<!-- made by sccs.cgi copyright (c) 2001 Richard Kettlewell -->\n",
	 "<!-- see http://www.greenend.org.uk/rjk/ -->\n",
	);
}

sub quote( $ ) {
  local $_ = shift;

  s/[\&<>\"]/sprintf("&#%d;", ord($&))/ge;
  return $_;
}

sub output {
  (print @_) || die "$0: stdout: $!";
}

sub tempfile() {
  ++$tmpcount;
  if(!defined $tmproot) {
    my $tmpdir = $ENV{'TMPDIR'} || "/tmp";
    for(my $n = 0; $n < 32; ++$n) {
      my $t = "$tmpdir/sccs.$$.$n";
      if(mkdir($t, 0700)) {
	$tmproot = $t;
	return "$tmproot/$tmpcount";
      }
    }
    die "$0: cannot create a temporary directory";
  }
  return "$tmproot/$tmpcount";
}

# filter(TYPE, IN, OUT)
#
# Read data from IN, apply filter TYPE, write to OUT

sub filter( $$$ ) {
  my ($type, $in, $out) = @_;

  $type = "\L$type\E";
  if(!exists $allowed_filters{$type}) {
    $type = "raw";
  }
  if($type eq "man2html") {
    my $path = save($in);
    my $command = `grog -Thtml \Q$path\E`;
    die "$0: grog failed" if $?;
    output("Content-Type: text/html\n",
	   "\n",
	  );
    command($command, $out);
    return;
  }
  if($type eq "diff") {
    local $_;
    output("Content-Type: text/html\n",
	   "\n",
	   "<html>\n",
	   " <body>\n",
	   "  <pre>",
	  );
    while(defined($_ = $in->getline())) {
      my $color;
      chomp;
      if(/^\+/) {
	$color = "#0000ff";
      } elsif(/^-/) {
	$color = "#ff0000";
      } elsif(/^[^ ]/) {
	$color = "#00ff00";
      }
      if(defined $color) {
	output("<font color=\"$color\">", quote($_), "</font>\n");
      } else {
	output(quote($_), "\n");
      }
    }
    return;
  }
  # raw
  output("Content-Type: text/plain\n",
	 "\n");
  copy($in, $out);
}

# save(HANDLE)
#
# read from HANDLE and save to a temporary file.  Return name of temporary
# file.

sub save( $ ) {
  my $in = shift;
  my $tmpfile = tempfile;
  (my $o = new IO::File($tmpfile, "w"))
    || die "$0: opening $tmpfile: $!";
  copy($in, $o);
  $o->close()
    || die "$0: writing $tmpfile: $!";
  return $tmpfile;
}

# command(COMMAND, OUT)
#
# Execute COMMAND and send the output to OUT

sub command( $$ ) {
  my ($command, $out) = @_;
  (my $i = new IO::File("$command|"))
    || die "$0: executing $command: $!";
  copy($i, $out);
  $i->close()
    || die "$0: read error: $!/$?";
}

# copy(IN, OUT)
#
# copy data from IN to OUT

sub copy( $$ ) {
  my ($in, $out) = @_;
  my $b;
  my $n;
  while(($n = $in->read($b, 1024))) {
    $out->print($b)
      || die "$0: write error: $!";
  }
  die "$0: read error: $!" if ! defined $n;
}

END {
  if(defined $tmproot) {
    system("rm -rf $tmproot");
  }
}
