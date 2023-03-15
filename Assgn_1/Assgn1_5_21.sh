#!/bin/bash
find $1 -name "*.py" | while read file; do
    multiline_flag=0
    echo "----------------------------------------------------------------------------------------------------------------------------------"
    echo "File: $(basename $file), Path: $file"
    echo "----------------------------------------------------------------------------------------------------------------------------------"
    # print line number and all lines
    grep -n "" $file | while read line; do
        if [[ $line == *'"""'* ]]; then
            # check if multiline comment is starting
            if [[ $line == *'"""'*'"""'* ]]; then
                # if multiline comment is starting, print the line but do not print line number
                echo $line
            elif [[ $multiline_flag -eq 1 ]]; then
                # if multiline comment is not starting, set multiline flag to 1
                echo $line | cut -d ":" -f 2-
                multiline_flag=0
            else
                # if multiline comment is not starting, set multiline flag to 1
                echo $line
                multiline_flag=1
                # print all the lines until multiline comment ends
            fi
        # if # is present inside double or single quotes, then it is not a comment
        elif [[ $line == *"'"*'#'*"'"* ]] || [[ $line == *'"'*'#'*'"'* ]]; then
            # if line is not a multiline comment, print the line
            continue
        elif [[ $line == *'#'* ]] && [[ $multiline_flag -eq 0 ]]; then
            # if line is not a multiline comment, print the line
            echo $line
        elif [[ $multiline_flag -eq 1 ]]; then
            # if line is a multiline comment, print the line but do not print line number
            echo $line | cut -d ":" -f 2-
        fi
    done
    echo ""
    echo ""
done >out.txt
