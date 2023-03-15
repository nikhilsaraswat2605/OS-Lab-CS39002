#!/bin/bash
# store smallest prime factor of each number in an array
spf=()
for((i=1;i<=1000010;i++))
do 
    spf[$i]=$i
done
for((i=2;i<=1000010;i++))
do 
    if((spf[$i]==$i))
    then 
        for((j=$((i*i));j<=1000010;j+=i))
        do 
                spf[$j]=$i
        done
    fi
done

# clear output file
> "output.txt"

while read n; do
    # Initialize array to store prime divisors of n
    while [[ $n -ne 1 ]]; do
        # echo in output file the smallest prime factor of n in same line
        div=${spf[$n]}
        echo -n "${spf[$n]} " >> "output.txt"
        # loop while n > 1 && n is divisible by spf[n]
        while [[ $(($n%$div)) -eq 0 && $n -ne 1 ]]; do
            n=$(($n/$div))
        done
    done
    # Print prime divisors of n in output file
    echo "" >> "output.txt"
done < "input.txt"
