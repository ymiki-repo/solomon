se ter png
se si sq
se xl 'x'
se yl 'y'
se xr [-1.5:1.5]
se yr [-1.5:1.5]
se g
se st da p

ini = 0
fin = 80
stp = 1
do for[ii = ini : fin : stp]{
    input = sprintf("dat/collapse_snp%03d.dat", ii)
    se o sprintf("fig/collapse_xymap%03d.png", ii)
    p input u 1:2 not
    se o
}
