set grid
plot "data/eg_attack.dat" u 1:2 w lines t "Attack", \
     "data/eg_decay.dat" u 1:2 w lines t "Decay", \
     "data/eg_sustain.dat" u 1:2 w lines t "Sustain", \
     "data/eg_release.dat" u 1:2 w lines t "Release"
pause mouse any
