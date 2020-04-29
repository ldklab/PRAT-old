#!/usr/bin/env perl

use strict;
use warnings;

use File::Find;

#my $path = $ENV{PWD};
my $path = $ARGV[0];

my $symbol_map = {};
find( make_ifdef_processor( $symbol_map ), $path );

foreach my $fn ( keys %$symbol_map ) {
   my @symbols = @{ $symbol_map->{$fn} };

   my @options;
   foreach my $symbol (@symbols) {
      push @options, [
         "-D$symbol=0",
         "-D$symbol=1"
      ];
   }

=pod
   my @combinations = @{ cartesian( @options ) };
   foreach my $combination (@combinations) {
      print "compile $fn with these symbols defined:\n";
      print "\t", join ' ', ( @$combination );
      print "\n";
   }
=cut
}

sub make_ifdef_processor {
   my $map_symbols = shift;

   return sub {
      my $fn = $_;

      if ( $fn =~ /svn-base/ ) {
         return;
      }

      open FILE, "<$fn" or die "Error opening file $fn ($!)";
      while ( my $line = <FILE> ) {
         if ( $line =~ /^\/\// ) { # skip C-style comments.
            next;
         }

         if ( $line =~ /#ifdef\s+(.*)$/ ) {
            print "matched line in $fn $line";
            my $symbol = $1;
            push @{ $map_symbols->{$fn} }, $symbol;
         }
      }
   }
}

sub cartesian {
   my $first_set = shift @_;
   my @product = map { [ $_ ] } @$first_set;

   foreach my $set (@_) {
      my @new_product;
      foreach my $s (@$set) {
         foreach my $list (@product) {
            push @new_product, [ @$list, $s ];
         }
      }
      @product = @new_product;
   }
   return \@product;
}
