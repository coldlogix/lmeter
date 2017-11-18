#
# data for 2um M2 MSL measured by LMeter --- to verify shift substraction
#
set terminal postscript color
set output "|lpr -Pps"
shift=[-0.25,0,0.25]
curr1=[0.9790,0.7832,0.5874];
curr2=[1.5822,1.2818,0.9679];
width = 2*ones(shift) - 2*shift;
plt=[width',curr1',curr2'];
xlabel("MSL width, um")
ylabel("Current, reported by LM, Ie")
title("Technology shifts in LMeter")
gplot [0:3][0:1.5] plt u 1:2, plt u 1:3

