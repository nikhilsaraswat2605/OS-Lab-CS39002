file=main.csv
if [ ! -f $file ]; then 
    touch $file
    echo "DATE,CATEGORY,AMOUNT,NAME">$file
fi
declare -a flag_array
function Print_by_name()
{
    total_amount_by_name=0
    while IFS="," read -r _ _ AMOUNT NAME
    do 
        if [ "$NAME" == "$1" ]; then
        total_amount_by_name=$(($total_amount_by_name+$AMOUNT))
        fi
    done < $file
    echo "Total Amount Spent by $1 is $total_amount_by_name"
}
function Print_by_category()
{
    total_amount_by_category=0
    while IFS="," read -r _ CATEGORY AMOUNT _
    do 
        if [ "$CATEGORY" == "$1" ]; then
        total_amount_by_category=$(($total_amount_by_category+$AMOUNT))
        fi
    done <$file
    echo "Total Amount Spent in category $1 is $total_amount_by_category"
}
function column_sorting()
{
    column_number=0
    check=1
    case $1 in
    date) column_number=1;;
    category) column_number=2;;
    amount) column_number=3;;
    name) column_number=4;;
    *) echo "Sorry $1 is an invalid column. Valid Columns are:date, category, amount, name";;
    esac
    if [ $column_number != 0 ]; then
    check=0
    fi
    # echo $check
    if [ $column_number != 1 ];
      then
        sorted_output=$(tail -n +2 $file|sort -t, -k $column_number )
    else
        sorted_output=$(tail -n +2 $file|sort -t- -k 3.1,3.2 -k 2n -k 1n  )
    fi
    if [ $check != 1 ];
        then
    echo "DATE,CATEGORY,AMOUNT,NAME">$file
    echo "$sorted_output">>$file
    fi
}
bold_output=$(tput bold)
normal_output=$(tput sgr0)
function output_for_help()
{
    echo  -e "${bold_output}NAME\n${normal_output}\t Assign_1_8_21.sh \n${bold_output}USAGE:\n${normal_output}\tThis script helps to track expenses, sort, filter and analyze a csv file containing expenses\n"
}

while getopts "n:c:s:h" flag;
do
    case $flag in
    n)flag_array+=($flag) NAME=$OPTARG;;
    c)flag_array+=($flag) CATEGORY=$OPTARG;;
    s)flag_array+=($flag) SORT=$OPTARG;;
    h)flag_array+=($flag) output_for_help=1;;
    ?)echo "$flag : isn't a correct flag:";;
    esac
done

shift $((OPTIND-1))

if [[ $# -eq 4 ]]; then
    echo $1,$2,$3,$4 >> $file
    column_sorting "date"
    else
    echo "Default number of arguments not present"
fi

for flag in "${flag_array[@]}"
do
case $flag in
n)Print_by_name "$NAME";;
c)Print_by_category "$CATEGORY";;
s)column_sorting "$SORT";;
h)output_for_help;;
esac
done