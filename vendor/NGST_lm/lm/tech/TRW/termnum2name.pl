#!/usr/local/bin/perl

open(NAMETAB,"<term.name");
while(<NAMETAB>)
  {
    chop;
    ($name, $num) = split;
    $table[$num] = $name;
  }

while(<>)
  {
   chop;
   ($a,$b,$c) = split;
   printf "%s\t%s\t%f\n",$table[$a], $table[$b], $c;
  }
