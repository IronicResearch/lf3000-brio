set title  "Small Mixer Test"
set xlabel "Time"
set ylabel "Level"

set data style lines
set grid
set yrange [-33000.0:33000.0]

plot 'mixIn1.txt', 'mixIn2.txt', 'mixOutL.txt'










