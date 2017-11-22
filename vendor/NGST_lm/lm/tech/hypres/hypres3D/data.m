#
# This is LM vs. SC2D data arranged for processing 
# with the Octave program.
#

mu0 = 4*pi/10;	# pHn/um
lambda = 0.085;	# um

Width     = [	2.0,	3.0,	4.0,	5.0,	6.0,	12.0	];
nWidth	  = columns(Width);

# What follows are inductances in pHn/um, calculated by sc2d

# 1um extra GP at sides
# LM0M1sc   = [	0.1583,	0.1143,	0.08978,0.07397,0.06293,0.03330	];	
# LM0M2sc   = [	0.2159,	0.1617,	0.1297,	0.1084,	0.09315,0.05076	];	
# LM0M1M3sc = [	0.1315,	0.09286,0.07179,0.05852,0.04938,0.02551	];
# LM0M2M3sc = [	0.1430,	0.1039,	0.08169,0.06727,0.05718,0.03009	];

# 6 um extra GP for M0, 2um for M0M3
LM0M1sc     = [	0.1534,	0.1111,	0.08741,0.07217,0.06151,0.03278	];	
LM0M2sc     = [	0.2040,	0.1536,	0.1238,	0.1039,	0.08958,0.04944	];	
LM0M1M3sc   = [	0.1314,	0.09260,0.07176,0.05849,0.04937,0.02550	];	
LM0M2M3sc   = [	0.1430,	0.1039,	0.08166,0.06725,0.05716,0.03009	];	
LM1M3sc     = [ 0.1784, 0.1784, 0.1784, 0.1521, 0.1328, 0.0761 ];
LM2M3sc     = [ 0.1385, 0.1385, 0.1385, 0.1182, 0.1032, 0.0591 ];

# low accuracy computation --- xsize: 0.15->0.30
#LM0M1sc   =  [	0.1545,	0.1116,	0.08770,0.07237,0.06165,0.03282	];	
#LM0M2sc   =  [	0.2057,	0.1544,	0.1243,	0.1042,	0.08984,0.04942	];	
#LM0M1M3sc =  [	0.1324,	0.09305,0.07202,0.05867,0.04949,0.02553	];	
#LM0M2M3sc =  [	0.1447,	0.1047,	0.08217,0.06761,0.05740,0.03016	];	

# and with flat M3 
#LM0M1sc   =  [	0.1545,	0.1116,	0.08770,0.07237,0.06165,0.03282	];	
#LM0M2sc   =  [	0.2057,	0.1544,	0.1243,	0.1042,	0.08984,0.04942	];	
#LM0M1M3sc =  [	0.1336,	0.09366,0.07239,0.05892,0.04966,0.02558	];	
#LM0M2M3sc =  [	0.1507,	0.1079,	0.08409,0.06890,0.05834,0.03041	];	


# Convert them to "admittances" Y (in some units used by LM)
# Y = mu0/L

YM0M1sc = LM0M1sc .\ mu0;
YM0M2sc = LM0M2sc .\ mu0;
YM0M1M3sc = LM0M1M3sc .\ mu0;
YM0M2M3sc = LM0M2M3sc .\ mu0;
YM1M3sc = LM1M3sc .\ mu0;
YM2M3sc = LM2M3sc .\ mu0;

# Magnetic gaps used by LMeter, um
# Note that for thin M0 modified lambda is used (0.1) 

dM0M1 = 0.1 + 0.15 + 0.086;
dM0M2 = 0.1 + 0.15 + 0.20 + 0.085;
dM1M3 = 0.086 + 0.2 + 0.5 + 0.085;
dM2M3 = 0.085 + 0.5 + 0.085;
dM0M1M3 = 1 /( 1/dM0M1 + 1/dM1M3 );
dM0M2M3 = 1 /( 1/dM0M2 + 1/dM2M3 );
 
# LMeter's admittances, Width/gap

YM0M1lm   = (Width-2*lambda)/dM0M1;
YM0M2lm   = (Width-2*lambda)/dM0M2;
YM0M1M3lm = (Width-2*lambda)/dM0M1M3;
YM0M2M3lm = (Width-2*lambda)/dM0M2M3;
YM1M3lm   = (Width-2*lambda)/dM1M3;
YM2M3lm   = (Width-2*lambda)/dM2M3;

# Yedge = 1/2 (Ysc-Ylm)

YM0M1edge   = 1/2*(YM0M1sc - YM0M1lm); 
YM0M2edge   = 1/2*(YM0M2sc - YM0M2lm);
YM0M1M3edge = 1/2*(YM0M1M3sc - YM0M1M3lm);
YM0M2M3edge = 1/2*(YM0M2M3sc - YM0M2M3lm); 
YM1M3edge = 1/2*(YM1M3sc - YM1M3lm);
YM2M3edge = 1/2*(YM2M3sc - YM2M3lm);

# 
#plot(Width , [YM0M1M3edge;YM0M2M3edge;YM0M1edge;YM0M2edge] )
#

nYedge = 5;
Yedge = linspace(0.7,2.1,nYedge);
for i=1:nYedge
	YM0M1error(i,1:nWidth) = 100 * (YM0M1lm + 2*Yedge(i)*ones(YM0M1sc) - YM0M1sc) ./ YM0M1sc;
	YM0M2error(i,1:nWidth) = 100 * (YM0M2lm + 2*Yedge(i)*ones(YM0M2sc) - YM0M2sc) ./ YM0M2sc;
	YM0M1M3error(i,1:nWidth) = 100*(YM0M1M3lm + 2*Yedge(i)*ones(YM0M1M3sc) - YM0M1M3sc)./YM0M1M3sc;
	YM0M2M3error(i,1:nWidth) = 100*(YM0M2M3lm + 2*Yedge(i)*ones(YM0M2M3sc) - YM0M2M3sc)./YM0M2M3sc;
	YM1M3error(i,1:nWidth) = 100*(YM1M3lm + 2*Yedge(i)*ones(YM1M3sc) - YM1M3sc)./YM1M3sc;
	YM2M3error(i,1:nWidth) = 100*(YM2M3lm + 2*Yedge(i)*ones(YM2M3sc) - YM2M3sc)./YM2M3sc;
endfor


# colors for postscript driver:
# 1-green, 2-blue, 3-red, 4-magenta, 5-cyan, 6-yellow, 7-black, 8-red, 9-grey, 10-green

set terminal postscript color 
set output "|lpr -Pps"

grid;
xlabel("Edge admittance Yedge=1/Ledge, arbitrary units");
ylabel("(Ylm(Yedge)-Ysc2d)/Ysc2d, %");

title("One groundplane (HYPRES tech)")

M1 = [Yedge',YM0M1error(:,:)];
M2 = [Yedge',YM0M2error(:,:)];

#gplot [0.6:2.2] [-10:10] M1 u 1:2 w linesp 4 2 t "2um",M1 u 1:3 w linesp 4 3 t "3um", M1 u 1:4 w linesp 4 4 t "4um", M1 u 1:5 w linesp 4 5 t "5um", M1 u 1:6 w linesp 4 6 t "6um", M1 u 1:7 w linesp 4 7 t "12um", M2 u 1:2 w linesp 5 2 t "2um",M2 u 1:3 w linesp 5 3 t "3um", M2 u 1:4 w linesp 5 4 t "4um", M2 u 1:5 w linesp 5 5 t "5um", M2 u 1:6 w linesp 5 6 t "6um", M2 u 1:7 w linesp 5 7 t "12um";

gplot [0.6:2.2] [-10:10] M1 u 1:2 w linesp 4 2 t "M1,2um", M1 u 1:4 w linesp 4 5 t "M1,4um", M1 u 1:7 w linesp 4 7 t "M1,12um", M2 u 1:2 w linesp 5 2 t "M2,2um", M2 u 1:4 w linesp 5 4 t "M2,4um", M2 u 1:7 w linesp 5 7 t "M2,12um";

input("Next plot?")

set output "lm4.ps"

title("One groundplane M3 (HYPRES tech)")

M1 = [Yedge',YM1M3error(:,3:6)];
M2 = [Yedge',YM2M3error(:,3:6)];

#gplot [0.6:2.2] [-10:10] M1 u 1:2 w linesp 4 2 t "4um",M1 u 1:3 w linesp 4 3 t "5um", M1 u 1:4 w linesp 4 4 t "6um", M1 u 1:5 w linesp 4 5 t "12um", M2 u 1:2 w linesp 5 2 t "4um",M2 u 1:3 w linesp 5 3 t "5um", M2 u 1:4 w linesp 5 4 t "6um", M2 u 1:5 w linesp 5 5 t "12um";

input("Next plot?")

title("Two groundplanes M0/M3 (HYPRES tech)")

M1 = [Yedge',YM0M1M3error(:,:)];
M2 = [Yedge',YM0M2M3error(:,:)];

#gplot [0.6:2.2] [-10:10] M1 u 1:2 w linesp 4 2 t "2um",M1 u 1:3 w linesp 4 3 t "3um", M1 u 1:4 w linesp 4 4 t "4um", M1 u 1:5 w linesp 4 5 t "5um", M1 u 1:6 w linesp 4 6 t "6um", M1 u 1:7 w linesp 4 7 t "12um", M2 u 1:2 w linesp 5 2 t "2um",M2 u 1:3 w linesp 5 3 t "3um", M2 u 1:4 w linesp 5 4 t "4um", M2 u 1:5 w linesp 5 5 t "5um", M2 u 1:6 w linesp 5 6 t "6um", M2 u 1:7 w linesp 5 7 t "12um";

gplot [0.6:2.2] [-10:10] M1 u 1:2 w linesp 4 2 t "M1,2um", M1 u 1:4 w linesp 4 4 t "M1,4um", M1 u 1:7 w linesp 4 7 t "M1,12um", M2 u 1:2 w linesp 5 2 t "M2,2um", M2 u 1:4 w linesp 5 4 t "M2,4um", M2 u 1:7 w linesp 5 7 t "M2,12um";







