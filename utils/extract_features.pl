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
	my %graph_content;

	# Initialize our dot graph first.
	my $filename = 'FDG.dot';
	open(my $fh, '>', $filename) or die "Could not open file '$filename' $!";

	my $bp_head = "digraph G {
	graph [fontsize=10 fontname=\"Verdana\" compound=true];
	subgraph cluster_components {
		label=\"MQTT Components\";
		\"WebSocket Support\";
		\"Bridge Support\";
		\"With Wrap\";
		\"...\"
	}\n
	subgraph cluster_bridge {
		label=\"TODO\";\n";
	my $bp_foot = "\n}\n}";

	print $fh $bp_head;

	foreach my $file (@files) {
		#print $file . "\n";
		open (FILE, $file) || die "Error: $!\n";
		my @lines = <FILE>;

		#print scalar @lines;
		# Don't print the empty files.
		if (@lines) {
			print "\n------------------\n";
			print "Lines to remove from ";
			print colored(['bright_red on_black'], "$file");
			print "\n------------------\n";
		}

		# Look for '#####'.
		foreach my $line (@lines) {
			# Some idl files just return a lot of /*EOF*/.
			if (($line =~ /#{5}/) && !($line =~ m/\/\*EOF\*\//)) {
				print colored(['bright_cyan'], "$line");

				# Save off file name + line num.
				my ($line_num) = $line =~ /(\d+)\:/;
				my ($line_src) = $line =~ /\d+\:(.*)/;
				$line_src =~ s/\"/\\"/g;
				#$line_src =~ s/\%/\\%/g; # Not sure if these break graphviz.
				my ($file_name) = $file =~ /\/{2}(.*?)\.gcov/;

				push(@{$unused_code{$file_name}}, $line_num);
				push(@{$graph_content{$file_name}}, $line_src . "\n");

				$line_count++;
			}
		}
		close(FILE);
	}

	print "\n------------------\n";
	print colored(['bright_green bold'], "Total lines to remove: " . $line_count . "\n");

	# Print just the file + line numbers to remove.
	# This will be input to automate removing/recompiling.
	foreach my $obj (keys %unused_code) {
		print colored(['bright_cyan'], "\t$obj");
		print ": @{$unused_code{$obj}}\n";

		# For each of the files, remove the extracted code.
		# This will preserve original in a *.bak file.
		if ($line_count > 0) {
			remove_feature($obj, @{$unused_code{$obj}});
		}
	}

	# Run lint check and compile new binary.
	if ($line_count > 0) {
		sanity_check("./mosquitto/");
	}

	foreach my $gobj (keys %graph_content) {
		# TODO: print block of code as node, but
		# need to decide at what granularity.
		# Print out each subgraph cluster in the form:
		# SRC_FILES_NAME -> LINE.
		print $fh "\"" , $gobj , "\" -> \"" , @{$graph_content{$gobj}}, "\";\n";
	}

	print $fh $bp_foot;
	close $fh;
}

# Using the line numbers provided above, remove
# the lines of code from the file then lint check
# and recompile debloated binary.
# Copy over whole source dir to work dir or
# work directly in source dir?
sub remove_feature {
	my ($file, @lines) = @_;
	#print "Removing @lines from $file \n";
	my $cmd_substr = '';

	# Construct the string to pass to sed.
	foreach (@lines) {
		$cmd_substr = $cmd_substr . $_ . 'd;';
	}

	# Remove the LoC for a feature and save file.
	# Create .bak of original in case.
	my $sed_cmd = "sed -i.bak -e '$cmd_substr' ./mosquitto/src/$file";

	print "Attempting to run [$sed_cmd]\n";

	system($sed_cmd) == 0
		or warn "Could not launch [$sed_cmd]: $! / $?\n"; # Change back to die later.

	return;
}

# Check each block of code to be removed and make sure
# all braces are closed, and for any unused variables.
# Then check whole source file before recompiling project
# to check for any errors i nthe whole program.
sub sanity_check {
  	my ($dir) = @_;
	chdir($dir) or die "$!";

	print colored(['bright_cyan'], "\n\nCompiling debloated binary...\n");

	my $mk_cmd = "make clean && make binary WITH_COVERAGE=yes -j";

	# Call make command.
	system($mk_cmd) == 0
		or die "Could not launch [$mk_cmd]: $! / $?\n";

	return;
}

sub wanted {
	push @content, $File::Find::name;
	return;
}

__END__

