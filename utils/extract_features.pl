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
	my %unused_code;

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

				# Save off file name + line num.
				my ($line_num) = $line =~ /(\d+)\:/;
				my ($file_name) = $file =~ /\/{2}(.*?)\.gcov/;
				push(@{$unused_code{$file_name}}, $line_num);

				$line_count++;
			}
		}
		print "------------------\n";

		close(FILE);
	}
	print colored(['bright_green bold'], "Total lines to remove: " . $line_count . "\n");

	# Print just the file + line numbers to remove.
	# This will be input to automate removing/recompiling.
	foreach my $obj (keys %unused_code) {
		print colored(['bright_cyan'], "\t$obj");
		print ": @{$unused_code{$obj}}\n";
	}
}

# Check each block of code to be removed and make sure
# all braces are closed, and for any unused variables.
# Then check whole source file before recompiling project
# to check for any errors i nthe whole program.
sub sanity_check {
	print "TODO\n";
	my $mk_cmd = `make`;
	my $cp_cmd = `cp`;

	# Call make command.
	print $mk_cmd;
	return;
}

sub wanted {
	push @content, $File::Find::name;
	return;
}

__END__
