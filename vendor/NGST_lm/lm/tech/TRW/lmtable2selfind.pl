#!/usr/local/bin/perl


# values from j230 inductance table/Chang formula
$indvalC{"PT10a"} =   2.178;
$indvalC{"PT2a"} =    1.857;
$indvalC{"PA10a"} =   1.355;
$indvalC{"PA3a"} =    1.083;
$indvalC{"PB3p5a"} =  1.233;
$indvalC{"PB7a"} =    1.577;
$indvalC{"PB10a"} =   1.726;
$indvalC{"PB20a"} =   1.958;

# values from j230 experimental table
$indvalX{"PA10a"} =   1.32;
$indvalX{"PA3a"} =    1.05;
$indvalX{"PB10a"} =   1.67;
$indvalX{"PB20a"} =   1.91;
$indvalX{"PB3p5a"} =  1.18;
$indvalX{"PB7a"} =    1.54;
$indvalX{"PT10a"} =   2.25;
$indvalX{"PT2a"} =    1.78;

$HTRIW=1.85;
$HWIRA=1.29;
$HWIRB=1.96;
$mu0 = 1.256637;

printf "%-6s %4s %10s %10s %10s%% %10s %10s%% %10s%%\n", "Layer","W,um", "LMeter", "Chang", "Chang-LM,", "SEA", "SEA-LM,", "Chang-SEA,";
while(<>)
  {
   chop;
   ($a,$b,$c) = split;
   if (($a eq $b) && ($a =~ /a$/))
     {
       $indsq = 1.0/$c/5.0; # inductances are drawn as 5 squares long
       $width = 2.0 if ($a =~ /2a$/);
       $width = 3.0 if ($a =~ /3a$/);
       $width = 3.5 if ($a =~ /3p5a$/);
       $width = 7.0 if ($a =~ /7a$/);
       $width = 10.0 if ($a =~ /10a$/);
       $width = 20.0 if ($a =~ /20a$/);
       $h = $HTRIW if ($a =~ /^PT/);
       $h = $HWIRA if ($a =~ /^PA/);
       $h = $HWIRB if ($a =~ /^PB/);

       $l = "TRIW" if ($a =~ /^PT/);
       $l = "WIRA" if ($a =~ /^PA/);
       $l = "WIRB" if ($a =~ /^PB/);
       
       $indLM = $indsq/$width;
       $indChang=$indvalC{$a}/$width;       
       $indX=$indvalX{$a}/$width;       
       $y = 0.5*($mu0/$indChang - $mu0/$indLM);
       printf "%-6s %4.1f %10.3f %10.3f %10.1f%% %10.3f %10.1f%% %10.1f%%\n", $l, $width, $indsq, 
       $indvalC{$a}, 100.0*($indvalC{$a}-$indsq)/$indvalC{$a}, 
       $indvalX{$a}, 100.0*($indvalX{$a}-$indsq)/$indvalX{$a}, 
       100.0*($indvalC{$a}-$indvalX{$a})/$indvalC{$a};
       # printf "%s\t%f\t%f\t%f\%\t%f\t%f\t%f\n",$a, $indsq, $indval{$a}, 100.0*($indval{$a}-$indsq)/$indval{$a}, $y, $width/$h, $h;
     }
 }
