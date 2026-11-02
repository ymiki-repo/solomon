se ter png
se si sq
se xl 'time'
se yl 'relative error |E(t) / E(t = 0) - 1|'
se log y
se g

file="dat/collapse_energy.dat"

# E(t = 0): the second column of the first data row
stats file u 2 every ::0::0 nooutput
E0=STATS_min

se o "fig/collapse_error.png"
p file u 1:(abs($2/E0-1.0)) not
se o
