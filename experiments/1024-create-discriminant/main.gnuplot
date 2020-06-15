# Set first two line styles to blue (#0060ad) and red (#dd181f)

#set term qt font "Times,11"
set terminal svg font "Times,24"
#set terminal pdf font "Times,11"
#set terminal latex
set datafile separator ","

set style line 1 pointtype 7 pointsize 0.2 lc rgb 'black'
set style line 2 pointtype 9 pointsize 0.2 lc rgb '#0060ad'
set style line 80  lt 0 lc rgb "#808080" lw 0.5
set style line 81  lt 0 lc rgb "#808080" lw 0.5
set style line 102 lt 1 lc rgb '#d6d7d9' lw 1

set mxtics 2
set mytics 2
# tic long
set tics scale -0.5

set yr [0:12000]

# nomirror means do not put tics on the opposite side of the plot
set xtics nomirror
set ytics nomirror

set grid xtics
set grid ytics
set grid mxtics
set grid mytics

# Put the grid behind anything drawn and use the linestyle 81
set grid back ls 81

set grid xtics ytics mxtics mytics ls 102, ls 80

# set style line 102 lc rgb '#d6d7d9' lt 0 lw 1
# set grid back ls 102

set xlabel 'Number of iterations'
set ylabel 'Elapsed time (ms)' offset 1,0

# Put the legend at the bottom left of the plot
set key top left box samplen 1 spacing 1 font ",18"
# at 1000,8500,1000 
plot 'mac_1024.csv' using 3:4 t 'Overall' with points ls 1, \
'' using 3:2 t 'Last iteration' with points ls 2
