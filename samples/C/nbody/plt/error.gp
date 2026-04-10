se ter png
se si sq
se xl 'time'
se yl 'relative error |E(t) / E(t = 0) - 1|'
se log y
se g

file="dat/collapse_energy.dat"

E0=system(sprintf('awk "NR==2 {print \$2}" %s', file))

se o "fig/collapse_error.png"
p file u 1:(abs($2/E0-1.0)) not
se o
