#Modified lambdas: 0.054 0.000 0.054 0.000 0.054 0.000 0.054 
set xrange  [0:2.0]
set xtics  0.1
set ytics  1
set grid
set yrange  [-5:5]
set xlabel "Boundary Admittance Y"
set ylabel "Relative error, %"
set title "Fitting LMeter to inductance table"
#TRIW gaps: 0.304 0.108
plot 100 * (1 - 1.0/(1.0/0.517899234061657 + 2*1*x/2.00)/0.345) title "TRIW/GND/2.00/0.304" with lines lt 1
replot 100 * (1 - 1.0/(1.0/0.517899234061657 + 2*1*x/5.00)/0.421) title "TRIW/GND/5.00/0.304" with lines lt 1
replot 100 * (1 - 1.0/(1.0/0.517899234061657 + 2*1*x/10.00)/0.460) title "TRIW/GND/10.00/0.304" with lines lt 1
replot 100 * (1 - 1.0/(1.0/0.517899234061657 + 2*1*x/20.00)/0.484) title "TRIW/GND/20.00/0.304" with lines lt 1
replot 100 * (1 - 1.0/(1.0/0.411004722394603 + 2*1*x/2.00)/0.291) title "WIRA/TRIW/2.00/0.219" with lines lt 2
replot 100 * (1 - 1.0/(1.0/0.411004722394603 + 2*1*x/5.00)/0.347) title "WIRA/TRIW/5.00/0.219" with lines lt 2
replot 100 * (1 - 1.0/(1.0/0.411004722394603 + 2*1*x/20.00)/0.390) title "WIRA/TRIW/20.00/0.219" with lines lt 2
replot 100 * (1 - 1.0/(1.0/0.765376356394603 + 2*1*x/2.00)/0.432) title "WIRA/GND/2.00/0.501" with lines lt 3
replot 100 * (1 - 1.0/(1.0/0.765376356394603 + 2*1*x/5.00)/0.567) title "WIRA/GND/5.00/0.501" with lines lt 3
replot 100 * (1 - 1.0/(1.0/0.765376356394603 + 2*1*x/10.00)/0.642) title "WIRA/GND/10.00/0.501" with lines lt 3
replot 100 * (1 - 1.0/(1.0/0.765376356394603 + 2*1*x/20.00)/0.692) title "WIRA/GND/20.00/0.501" with lines lt 3
replot 100 * (1 - 1.0/(1.0/0.995338899061143 + 2*1*x/3.50)/0.605) title "WIRB/TRIW/3.50/0.684" with lines lt 5
replot 100 * (1 - 1.0/(1.0/0.995338899061143 + 2*1*x/10.00)/0.789) title "WIRB/TRIW/10.00/0.684" with lines lt 5
replot 100 * (1 - 1.0/(1.0/0.995338899061143 + 2*1*x/20.00)/0.871) title "WIRB/TRIW/20.00/0.684" with lines lt 5
replot 100 * (1 - 1.0/(1.0/1.34971053306114 + 2*1*x/3.50)/0.732) title "WIRB/GND/3.50/0.966" with lines lt 7
replot 100 * (1 - 1.0/(1.0/1.34971053306114 + 2*1*x/10.00)/1.008) title "WIRB/GND/10.00/0.966" with lines lt 7
replot 100 * (1 - 1.0/(1.0/1.34971053306114 + 2*1*x/20.00)/1.137) title "WIRB/GND/20.00/0.966" with lines lt 7
