maxpwr=9
pwr=1
nelems=10

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
