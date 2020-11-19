set terminal pngcairo enhanced font "arial,10" fontscale 1.0 size 600, 400 
set output './gnuplot/moving-average.png'
set boxwidth 1.5 absolute
set style fill solid 1.00 border lt -1
set key fixed right top vertical Right noreverse noenhanced autotitle nobox
set style histogram clustered gap 1 title textcolor lt -1
# set datafile missing '-'
set style data lines
#set xtics border in scale 0,0 nomirror
#set xtics  norangelimit 
#set x2tics border nomirror ("1x10e-1" 0, "1x10e-2" 1, "1x10e-3" 2, "1x10e-4" 3, "1x10e-5" 4)
set yrange [0:10]
set xrange [0:60]
#set x2range [4:0]
#show x2label
set title "" 
set ylabel "Average Throughput (Mbps)" font ",10"
set xlabel "Seconds" font ",10"
#set x2label "Drop Probability (%)" 
NO_ANIMATION = 1