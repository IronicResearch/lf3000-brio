set title  "Small Mixer Test (Inputs)"
set xlabel "Time"
set ylabel "Level"

set data style lines
set grid
set yrange [-33000.0:33000.0]

plot 'mixIn1.txt', 'mixIn2.txt', 'mixIn3.txt', 'mixIn4.txt'
# 'mixIn1.txt', 'mixIn2.txt', 'mixIn3.txt', 'mixIn4.txt'










