mkdir -p $2
# Loop through all files in the directory specified by $1 with the .jsonl file extension
for json_file in "$1"/*.jsonl
do 
    # Create a new CSV file in the directory specified by $2 using the same file name as the original JSONL file, but with a .csv file extension
    csv_file="$2/$(basename "$json_file" .jsonl).csv"
    # Write the list of attributes specified by "${@:3}" as the header row of the CSV file, with each attribute separated by a comma
    echo "${@:3}"|tr ' ' ','> "$csv_file" 
    # Read each line of the original JSONL file
    while read -r line
    do 
    # Extract the specified attributes using jq and write them as a new row in the CSV file, with each attribute separated by a comma
    echo "$(for attribute in "${@:3}"
    do echo -n ",$(echo "$line" | jq -r ".$attribute")"
    done | cut -c2-)" >> "$csv_file"
    done < "$json_file"
done
