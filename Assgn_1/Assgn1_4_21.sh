while IFS= read line
    do
        echo $line | grep -q "$2"
        if [ $? -eq 0 ]
        then
            echo "$line"|tr '[:upper:]' '[:lower:]'|sed 's/[a-z][^a-z\n]*[a-z]\?/\u\0/g'
        else
            echo "$line"
        fi
    done <$1
