while read line
do 
echo $line | grep -iqFf fruits.txt 
flag=$?
len=${#line}
echo $line | grep -qE '^[a-zA-Z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*$'
if (( !$?&&$flag&&$len > 4&&$len < 21 ))
then
    echo "YES"
else
    echo "NO"
fi
done < $1 > validation_results.txt
