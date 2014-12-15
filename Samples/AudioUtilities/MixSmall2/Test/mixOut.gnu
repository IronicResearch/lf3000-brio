set title  "Small Mixer Test (Outputs)"
set xlabel "Time"
set ylabel "Level"

set data style lines
set grid
set yrange [-33000.0:33000.0]

plot 'mixOut1.txt', 'mixOut2.txt'











