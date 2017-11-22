#Modified lambdas: 0.429 0.000 1.258 0.000 0.027 0.000 0.429 0.000 0.355 
set xrange  [0:2.0]
set xtics  0.1
set ytics  1
set grid
set yrange  [-5:5]
set xlabel "Boundary Admittance Y"
set ylabel "Relative error, %"
set title "Fitting LMeter to inductance table"
#TRIW gaps: 1.937
plot 100 * (1 - 1.0/(1.0/(2.43461383688414*(2.00/(2.00-2*0))) + 2*0.129038636534682*x/2.00)/1.731) title "TRIW/GND/2.00/0.25" with lines lt 1
replot 100 * (1 - 1.0/(1.0/(2.43461383688414*(20.00/(20.00-2*0))) + 2*0.129038636534682*x/20.00)/2.304) title "TRIW/GND/20.00/0.25" with lines lt 1
#WIRA gaps: GND: 1.344 TRIW: 2.037
replot 100 * (1 - 1.0/(1.0/(1.68829407559946*(3.00/(3.00-2*0))) + 2*0.360996910318244*x/3.00)/1.094) title "WIRA/GND/3.00/0.485" with lines lt 3
replot 100 * (1 - 1.0/(1.0/(1.68829407559946*(20.00/(20.00-2*0))) + 2*0.360996910318244*x/20.00)/1.527) title "WIRA/GND/20.00/0.485" with lines lt 3
#WIRB gaps: GND: 1.870 TRIW: 2.564 WIRA: 1.505
replot 100 * (1 - 1.0/(1.0/(1.8909049030591*(3.50/(3.50-2*0))) + 2*0.478489763571004*x/3.50)/1.104) title "WIRB/WIRA/3.50/0.72" with lines lt 4
replot 100 * (1 - 1.0/(1.0/(1.8909049030591*(20.00/(20.00-2*0))) + 2*0.478489763571004*x/20.00)/1.638) title "WIRB/WIRA/20.00/0.72" with lines lt 4
replot 100 * (1 - 1.0/(1.0/(2.3495774080591*(3.50/(3.50-2*0))) + 2*0.580296329171082*x/3.50)/1.196) title "WIRB/GND/3.50/1.085" with lines lt 7
replot 100 * (1 - 1.0/(1.0/(2.3495774080591*(20.00/(20.00-2*0))) + 2*0.580296329171082*x/20.00)/1.940) title "WIRB/GND/20.00/1.085" with lines lt 7
