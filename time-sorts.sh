# Tests sorting algorithms on arrays with sizes
# that are powers of 10, from 1 to 10^maxpwr,
# running ./sorts 5 times. 
# Designed to show how disgustingly slow soft heaps
# perform on basic algorithms.
maxpwr=8
pwr=0
nelems=1

until [ $pwr -ge `expr $maxpwr + 1` ]
do
    echo $nelems
    for i in 1 2 3 4 5
    do
        ./sorts $nelems
    done

    nelems=`expr $nelems \* 10`
    pwr=`expr $pwr + 1`
done
