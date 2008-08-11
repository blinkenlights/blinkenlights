set autoscale
unset log
unset label
set xtic auto
set ytic auto
set title "Arcade Power consumption"
set ylabel "Time (seconds)"
set xlabel "Consumption (watts)"

set title 'Arcade power consumption - average'
plot "video-playlist.power.data2" using 1:3 title 'average (LUT 1)' with lines, \
"video-playlist-lut1.power.data2" using 1:3 title 'average (LUT 2)' with lines, \
"video-playlist-lut2.power.data2" using 1:3 title 'average (LUT 3)' with lines   
set term post eps enhan color
set out 'power-average-sum.eps'
replot
set out
set term wxt

set title 'Arcade power consumption - total'
plot "video-playlist.power.data2" using 1:4 title 'total (LUT 1)' with lines, \
"video-playlist-lut1.power.data2" using 1:4 title 'total (LUT 2)' with lines, \
"video-playlist-lut2.power.data2" using 1:4 title 'total (LUT 3)' with lines   
set term post eps enhan color
set out 'power-total.eps'
replot
set out
set term wxt

set title 'Arcade power consumption - maximum'
plot "video-playlist.power.data2" using 1:2 title 'maximum (LUT 1)' with lines, \
"video-playlist-lut1.power.data2" using 1:2 title 'maximum (LUT 2)' with lines, \
"video-playlist-lut2.power.data2" using 1:2 title 'maximum (LUT 3)' with lines   
set term post eps enhan color
set out 'power-maximum.eps'
replot
set out
set term wxt

set title 'Arcade power consumption - average'
plot "video-playlist.power.data2" using 1:5 title 'average (LUT 1)' with lines, \
"video-playlist-lut1.power.data2" using 1:5 title 'average (LUT 2)' with lines, \
"video-playlist-lut2.power.data2" using 1:5 title 'average (LUT 3)' with lines   
set term post eps enhan color
set out 'power-average.eps'
replot
set out
set term wxt

set title 'Arcade power consumption - total'
plot 'video-playlist.data' using 1:24 title 'total' with lines

