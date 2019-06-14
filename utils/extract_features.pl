#!/usr/bin/env perl

use strict;
use warnings;

use Term::ANSIColor;

=pod
First iteration for parsing the diff files.
We want to find lines that contain '#####:'
That corresponds to never being called.
'-:' corresponds to no code (comments, unreachable, etc.).
=cut

open (FILE, $ARGV[0]) || die "Error: $!\n";
my @lines = <FILE>;

#print scalar @lines;
print "Lines to remove from ";
print colored(['bright_red on_black'], "$ARGV[0]");
print "\n------------------\n";

# Look for '#####'.
foreach my $line (@lines) {
	print colored(['green bold'], "$line") if $line =~ /#{5}/;
}
print "------------------\n";

close(FILE);

exit 1;
