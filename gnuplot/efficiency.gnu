# set terminal pngcairo  transparent enhanced font "arial,10" fontscale 1.0 size 600, 400 
set terminal pngcairo enhanced font "arial,10" fontscale 1.0 size 600, 400 
set output 'efficiency_1ms_3.png'
set boxwidth 0.5 absolute
set style increment default
set title "Efficiency; 20ms delay; 1x10e-3 drop rate" 
set xrange [ 0.00000 : 6.0000 ] noreverse nowriteback
set x2range [ * : * ] noreverse writeback
set yrange [ 0.00000 : 10.0000 ] noreverse nowriteback
set y2range [ * : * ] noreverse writeback
set zrange [ * : * ] noreverse writeback
set cbrange [ * : * ] noreverse writeback
set rrange [ * : * ] noreverse writeback
set ylabel "Average Throughput (Mbps)" font ",10"
set xtics border nomirror ("BIC" 1, "Hybla" 2, "H-TCP" 3, "HighSpeed" 4, "NewReno" 5)
plot '../data/0.001/quantiles.data' using 1:3:2:6:5 with candlesticks lt 3 lw 2 notitle whiskerbars, \
     ''                       using 1:4:4:4:4 with candlesticks lt -1 lw 2 notitle