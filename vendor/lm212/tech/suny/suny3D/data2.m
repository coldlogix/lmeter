#
# This is LM vs. SC2D data arranged for processing 
# with the Octave program.
#
# Now let us try choose cut-off, not Yedge 
#

mu0 = 4*pi/10;	# pHn/um
lambda = 0.085;	# um

Width     = [	1.5,	3.0,	6.0];
nWidth	  = columns(Width);

# What follows are inductances in pHn/um, calculated by sc2d

LM2M5sc     = [	0.2894,	0.1787,	0.1032	];	

# sharp edge
#LM4M5sc    = [	0.2193,	0.1369,	0.07973	];	
# flat M5
#LM4M5sc    = [	0.2389,	0.1449,	0.08248	];	
# "real" case
LM4M5sc    = [ 0.2318,	0.1418,	0.08133 ];	

LM2M4M5sc  = [ 0.1394,	0.07553, 0.03941 ];

# Convert them to "admittances" Y (in some units used by LM)
# Y = mu0/L

YM2M5sc = LM2M5sc .\ mu0;
YM4M5sc = LM4M5sc .\ mu0;
YM2M4M5sc = LM2M4M5sc .\ mu0;

# Magnetic gaps used by LMeter, um
# Note that for thin superconductor lambda is changed to lambda*coth(t/lambda) 

dM2M5 = 0.09 + 0.135 + 0.3 + 0.085;
dM4M5 = 0.086 + 0.3 + 0.085;
dM2M4 = 0.09 + 0.135 + 0.086;
dM2M4M5 = 1/(1/dM2M4 + 1/dM4M5);
 
# LMeter's admittances, Width/gap

YM2M5lm   = (Width-2*lambda)/dM2M5;
YM4M5lm   = (Width-2*lambda)/dM4M5;
YM2M4M5lm = (Width-2*lambda)/dM2M4M5;

nCutOffs = 5;
CutOff = linspace(0,1,nCutOffs);
for i=1:nCutOffs
	YM2M5error(i,1:nWidth) = 100 * (YM2M5lm + 2*CutOff(i)/dM2M5*ones(YM2M5sc) - YM2M5sc) ./ YM2M5sc;
	YM4M5error(i,1:nWidth) = 100 * (YM4M5lm + 2*CutOff(i)/dM4M5*ones(YM4M5sc) - YM4M5sc) ./ YM4M5sc;
	YM2M4M5error(i,1:nWidth) = 100 * (YM2M4M5lm + 2*CutOff(i)/dM2M4M5*ones(YM2M4M5sc) - YM2M4M5sc) ./ YM2M4M5sc;
endfor


# colors for postscript driver:
# 1-green, 2-blue, 3-red, 4-magenta, 5-cyan, 6-yellow, 7-black, 8-red, 9-grey, 10-green

#set terminal postscript color
#set output "|lpr -Pps"

grid;
xlabel("Additional shift, um");
ylabel("(Ylm(CutOff)-Ysc2d)/Ysc2d, %");

title("SUNY tech")

M2 = [CutOff',YM2M5error(:,:)];
M4 = [CutOff',YM4M5error(:,:)];
M2M4 = [CutOff',YM2M4M5error(:,:)];

gplot [0.4:0.8] M2 u 1:2 w linesp 4 2 t "1.5um",M2 u 1:3 w linesp 4 3 t "3um", M2 u 1:4 w linesp 4 4 t "6um", M4 u 1:2 w linesp 5 2 t "1.5um",M4 u 1:3 w linesp 5 3 t "3um", M4 u 1:4 w linesp 5 4 t "6um"

#gplot [0:0.3] M2M4 u 1:2 w linesp 9 2 t "1.5um,2GP",M2M4 u 1:3 w linesp 9 3 t "3um,2GP", M2M4 u 1:4 w linesp 9 4 t "6um,2GP"


title("Two groundplanes, SUNY tech, complex shape of M4")









