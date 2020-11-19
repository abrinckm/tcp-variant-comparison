set boxwidth 0.9 absolute
set style fill pattern 1.00 border lt -1
set terminal pngcairo enhanced font "arial,10" fontscale 1.0 size 600, 400 
set output 'index.png'
set key outside top horizontal Right noreverse noenhanced autotitle nobox
set style histogram clustered gap 2 title textcolor lt -1
set datafile missing '-'
set style data histograms
set xtics border in scale 0,0 nomirror autojustify
set x2tics border in scale 0,0 nomirror autojustify
#set xtics  norangelimit 
#set xrange [ 0 : 4 ]
set xtics border nomirror ("10ms delay" 0, "10ms delay" 1, "100ms delay" 2, "100ms delay" 3, "" 4)
set x2tics border nomirror ("1x10e-5" 0, "1x10e-3" 1,"1x10e-5" 2, "1x10e-3" 3, "" 4)
set title " " #Fairness of variants against NewReno" 
#set x2range [ * : * ] noreverse writeback
set yrange [ 0.5 : 1.0 ] noreverse writeback
set y2range [ * : * ] noreverse writeback
set zrange [ * : * ] noreverse writeback
set cbrange [ * : * ] noreverse writeback
set rrange [ * : * ] noreverse writeback
set ylabel "Jain's fairness index"
set xlabel "Delay" 
set x2label "Drop probability"
NO_ANIMATION = 1
plot '../data/f_c/index.data' using 2 ti col, '' u 3 ti col, '' u 4 ti col, '' u 5 ti col