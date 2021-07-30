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

die "Usage: $0 DIR\n\n[DIR] directory to coverage diffs to compare\n" if @ARGV < 1;

my $diff_dir = $ARGV[0];
my $delete = $ARGV[1];
my @content;
my $line_count = 0;

# For truncating long strings in the graph.
my $MAX_LEN = 100;

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

	foreach my $gobj (keys %graph_content) {
		# TODO: print block of code as node, but
		# need to decide at what granularity.
		# Print out each subgraph cluster in the form:
		# SRC_FILES_NAME -> LINE.

		# First, check length of string beacuse of dot limits.
		if (scalar(@{$graph_content{$gobj}}) >= $MAX_LEN) {
			my $trunc = substr(@{$graph_content{$gobj}}, 0, $MAX_LEN - 1);
			print $fh "\"" , $gobj , "\" -> \"" , $trunc, "\";\n";
		} else {
			print $fh "\"" , $gobj , "\" -> \"" , @{$graph_content{$gobj}}, "\";\n";
		}
	}

	print $fh $bp_foot;
	close $fh;

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
			# Manually comment/uncomment in lieu of command line arg.
			if (defined $delete && $delete eq "--delete") {
				print $delete . " is defined. Removing code...\n";
				remove_feature($obj, @{$unused_code{$obj}});
			}
			#remove_feature($obj, @{$unused_code{$obj}});
		}
	}

	# Run lint check and compile new binary.
	if ($line_count > 0) {
		# sanity_check("../artifacts/mosquitto/");
	}
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

	my $sed_cmd = '';

	# First, check which of 3 dirs the file is in.
	# (libavcodec, libavformat, libavfilter)
	print "[+] in remove_feature function: " . $file . "\n";
	my $fname1 = "../artifacts/FFmpeg/libavcodec/$file";
	my $fname2 = "../artifacts/FFmpeg/libavformat/$file";
	my $fname3 = "../artifacts/FFmpeg/libavfilter/$file";
	my $mosq1 = "../artifacts/mosquitto/src/$file";
	my $mosq2 = "../artifacts/mosquitto/lib/$file";

	if (-e $fname1) {
		# Remove the LoC for a feature and save file.
		# Create .bak of original in case.
		$sed_cmd = "sed -i.bak -e '$cmd_substr' ../artifacts/FFmpeg/libavcodec/$file";
	} elsif (-e $fname2) {
		$sed_cmd = "sed -i.bak -e '$cmd_substr' ../artifacts/FFmpeg/libavformat/$file";
	} elsif (-e $fname3) {
		$sed_cmd = "sed -i.bak -e '$cmd_substr' ../artifacts/FFmpeg/libavfilter/$file";
	} elsif (-e $mosq1) {
		$sed_cmd = "sed -i.bak -e '$cmd_substr' ../artifacts/mosquitto/src/$file";
	} elsif (-e $mosq2) {
		$sed_cmd = "sed -i.bak -e '$cmd_substr' ../artifacts/mosquitto/lib/$file";
	} else {
		print colored(['bright_red on_black'], "Could not find $file in appropriate source dir\n");
	}

	print colored(['bright_green bold'], "Attempting to run [$sed_cmd]\n");

	system($sed_cmd) == 0
		or warn "Could not launch [$sed_cmd]: $! / $?\n"; # Change back to die later.

	return;
}

# Check each block of code to be removed and make sure
# all braces are closed, and for any unused variables.
# Then check whole source file before recompiling project
# to check for any errors in the whole program.
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

