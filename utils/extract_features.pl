#!/usr/bin/env perl

use strict;
use warnings;

use File::Find;
use Term::ANSIColor;

=pod
First iteration for parsing the diff files.
We want to find lines that contain '#####:'
That corresponds to never being called.
'-:' corresponds to no code (comments, unreachable, etc.).
=cut

die "Usage: $0 FILE\n" if @ARGV < 1;

my $diff_file = $ARGV[0];
my @content;

find(\&wanted, '.');

=pod
foreach my $temp (@content) {
	print $temp . "\n";
}
=cut

if ($diff_file) {
	print "Loading: $diff_file\n";
	parse_file();
}

sub parse_file {
	open (FILE, $diff_file) || die "Error: $!\n";
	my @lines = <FILE>;

	#print scalar @lines;
	print "Lines to remove from ";
	print colored(['bright_red on_black'], "$diff_file");
	print "\n------------------\n";

	# Look for '#####'.
	foreach my $line (@lines) {
		print colored(['green bold'], "$line") if $line =~ /#{5}/;
	}
	print "------------------\n";

	close(FILE);
}

sub wanted {
	push @content, $File::Find::name;
	return;
}

exit 1;
