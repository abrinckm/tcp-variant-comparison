set terminal pngcairo enhanced font "arial,10" fontscale 1.0 size 600, 400 
set output './responsiveness_1ms_3.png'
set boxwidth 1.5 absolute
set style fill solid 1.00 border lt -1
set key fixed left bottom vertical Right noreverse noenhanced autotitle nobox
#set style histogram clustered gap 1 title textcolor lt -1
set title "Responsiveness; 20ms delay"
# set datafile missing '-'
set style data linespoints
#set xtics border in scale 0,0 nomirror
#set xtics  norangelimit 
#set x2tics border mirror ("1x10e-1" 1, "1x10e-2" 2, "1x10e-3" 3, "1x10e-4" 4, "1x10e-5" 5)
set xtics border mirror ("1x10e-1" 1, "1x10e-2" 2, "1x10e-3" 3, "1x10e-4" 4, "1x10e-5" 5)
#set yrange [0:20]
#set xrange [0:75]
set xrange [5:1]
#show x2label
set ylabel "Average Throughput (Mbps)" font ",10"
#set xlabel "Seconds" font ",10"
set xlabel "Probable Drop Rate" 
plot '../data/TcpBic-resp.data' using 1:2 title 'Bic', \
     '../data/TcpHighSpeed-resp.data' using 1:2 title 'HighSpeed', \
     '../data/TcpHtcp-resp.data' using 1:2 title 'H-Tcp', \
     '../data/TcpHybla-resp.data' using 1:2 title 'Hybla', \
     '../data/TcpNewReno-resp.data' using 1:2 title 'NewReno'
NO_ANIMATION = 1