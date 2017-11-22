#!/usr/local/bin/perl

use Math::Trig;
$mu0 = 1.256637;

%layer= ('GND',0,'GNDCSIOG',1,'TRIW',2,'SIOA',3,'WIRA',4,'SIOB',5,'WIRB',6);
@thick= (0.200,  0.304,   0.200,   0.219,   0.300,   0.490,   0.600);
@overe= (0.000,  0.022,   0.000,   0.025,   0.000,   0.000,   0.000);
@lambda=(0.054,  0,       0.054,   0,       0.054,   0,       0.054);

sub modlambda
  {
    foreach $i ('GND', 'TRIW', 'WIRA', 'WIRB')
      {
	my $ind = $layer{$i};
	$lambda[$ind] = $lambda[$ind] * coth($thick[$ind]/$lambda[$ind]);
      }
    print "#Modified lambdas: ";
    for (@lambda)
      {
	printf "%5.3f ", $_;
      }
    print "\n";
  }

sub gapI
  {
    my $top = $_[0];
    my $bot = $_[1];
    my $t = $layer{$top};
    my $b = $layer{$bot};
    
    my $h = $overe[$t-1]; 
    for ($i=$b+1; $i<$t; $i+=2)
      {
	$h += $thick[$i] - $overe[$i];
      }
    return $h;
  }

sub gapLambda
  {
    my $top = $_[0];
    my $bot = $_[1];
    my $t = $layer{$top};
    my $b = $layer{$bot};
    
    return $lambda[$t] + $lambda[$b];
  }

sub gap
  {
    my $top = $_[0];
    my $bot = $_[1];

    return gapI($top,$bot)+gapLambda($top,$bot);
  }

sub readtable
  {
    open(TAB, "<indtab") or die "indtab: $!";
    my $i=0;
    while(<TAB>)
      {
	chop;
	@toplays = split if ($i==0);
	@botlays = split if ($i==1);
	if ($i>1)
	  {
	    @vals = split;
	    $w = $vals[0];
	    $width[@width] = $w;
	    foreach $i (1..@toplays)
	      {
		$L{ $toplays[$i-1].$botlays[$i-1].$w}=$vals[$i];
	      }
	  }
	$i++;
      }
  }


modlambda();
readtable();

$plot = "plot";
sub err0
  {
    my $top=$_[0];
    my $bot=$_[1];
    my $w =$_[2];
    my $Lsq = $mu0 * gap ($top, $bot);
    my $L3d = $L{$top.$bot.$w};
    
    $L3d > 0 || die "No such value ".$top.$bot.$w;
    $comment = gapI($top, $bot);
    print "$plot 100 * (1 - ($Lsq * $w) / ($L3d * $w + $L3d * 2 * x*$comment)) title \"$top/$bot/$w/$comment\" with lines lt $style\n";
    $plot = "replot";
  }

sub err
  {
    my $top=$_[0];
    my $bot=$_[1];
    my $w =$_[2];
    my $LsqI = $mu0 * gapI($top, $bot);
    my $LsqLambda = $mu0 * gapLambda($top, $bot);
    my $Lsq = $LsqI + $LsqLambda;
    my $L3d = $L{$top.$bot.$w};
    
    $comment = gapI($top, $bot);

    $frac = gapI($top, $bot)/gap($top, $bot);

    $L3d > 0 || die "No such value ".$top.$bot.$w;

    $frac=1;
    $L2d = "1.0/(1.0/$Lsq + 2*$frac*x/$w)";
    print "$plot 100 * (1 - $L2d/$L3d)";
    print " title \"$top/$bot/$w/$comment\" with lines lt $style\n";
    $plot = "replot";
  }

print "set xrange  [0:2.0]\n";
print "set xtics  0.1\n";
print "set ytics  1\n";
print "set grid\n";
print "set yrange  [-5:5]\n";
print "set xlabel \"Boundary Admittance Y\"\n";
print "set ylabel \"Relative error, %\"\n";
print "set title \"Fitting LMeter to inductance table\"\n";

$style = 1;
printf "#TRIW gaps: %5.3f %5.3f\n", gapI('TRIW','GND'),gapLambda('TRIW','GND');
err('TRIW','GND','2.00');
err('TRIW','GND','5.00');
err('TRIW','GND','10.00');
err('TRIW','GND','20.00');

$style = 2;
#printf "#WIRA gaps: GND: %5.3f TRIW: %5.3f\n", gap('WIRA','GND'), gap('WIRA','TRIW');
err('WIRA','TRIW','2.00');
err('WIRA','TRIW','5.00');
#err('WIRA','TRIW','10.00');
err('WIRA','TRIW','20.00');

$style = 3;
err('WIRA','GND','2.00');
err('WIRA','GND','5.00');
err('WIRA','GND','10.00');
err('WIRA','GND','20.00');

#$plot = "plot";
#printf "#WIRB gaps: GND: %5.3f TRIW: %5.3f WIRA: %5.3f\n", gap('WIRB','GND'), gap('WIRB','TRIW'), gap('WIRB','WIRA');
$style = 4;
#err('WIRB','WIRA','3.50');
#err('WIRB','WIRA','10.00');
#err('WIRB','WIRA','20.00');

$style = 5;
err('WIRB','TRIW','3.50');
err('WIRB','TRIW','10.00');
err('WIRB','TRIW','20.00');

$style = 7;
err('WIRB','GND','3.50');
err('WIRB','GND','10.00');
err('WIRB','GND','20.00');


