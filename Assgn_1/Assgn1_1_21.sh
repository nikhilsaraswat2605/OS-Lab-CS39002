lcm=1
for line in $(rev $1)
do
    gcd=$lcm
    temp=$line
    while(($temp))
    do 
        temp2=`expr $gcd % $temp`
        gcd=$temp
        temp=$temp2
    done
    lcm=`expr $lcm / $gcd \* $line`
done
echo $lcm
