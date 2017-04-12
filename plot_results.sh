#/bin/bash

# merge dat's

# $1 = Graphic title
# $2 = base binary
# $3 = target binary
# $4 = iterations time

export FIRST=10000
export STEP=500
export LAST=$(echo "$FIRST + $4 * $STEP" | bc)

rm -f $2.dat $3.dat
for i in $(seq $FIRST $STEP $LAST); do
  echo $i
  ./$2 $i >> $2.dat
  ./$3 $i >> $3.dat
done


#plot results
gnuplot -e "set terminal png size 2000,2000;
  set output '$1.png';
   set style line 1 lc rgb '#0000b0' lt 1 lw 2 pt 7 ps 1.5;
   set style line 2 lc rgb '#f07000' lt 1 lw 2 pt 5 ps 1.5;
   plot '$2.dat'  with linespoints ls 1, \
        '$3.dat'  with linespoints ls 2;"

