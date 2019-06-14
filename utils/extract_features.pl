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

print scalar @lines;
print "\n---------------\n";

# Add regex here to identify '#####'.
foreach my $line (@lines) {
	print "$line";
}

close(FILE);

exit 1;
