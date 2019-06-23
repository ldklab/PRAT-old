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

die "Usage: $0 DIR\n" if @ARGV < 1;

my $diff_dir = $ARGV[0];
my @content;
my $line_count = 0;

find(\&wanted, '.');

if ($diff_dir) {
	#print "Loading: $diff_dir\n";
	parse_file();
}

sub parse_file {
	my @files = <$diff_dir/*>;

	foreach my $file (@files) {
		#print $file . "\n";
		open (FILE, $file) || die "Error: $!\n";
		my @lines = <FILE>;

		#print scalar @lines;
		print "Lines to remove from ";
		print colored(['bright_red on_black'], "$file");
		print "\n------------------\n";

		# Look for '#####'.
		foreach my $line (@lines) {
			if ($line =~ /#{5}/) {
				print colored(['bright_cyan'], "$line");
				$line_count++;
			}
		}
		print "------------------\n";

		close(FILE);
	}
	print colored(['bright_green bold'], "Total lines to remove: " . $line_count . "\n");
}

# Check each block of code to be removed and make sure
# all braces are closed, and for any unused variables.
# Then check whole source file before recompiling project
# to check for any errors i nthe whole program.
sub sanity_check {
	print "TODO\n";
	my $cmd = `make`;
	# Call make command.
	print $cmd;
}

sub wanted {
	push @content, $File::Find::name;
	return;
}

exit 1;
