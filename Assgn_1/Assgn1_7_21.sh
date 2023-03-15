mkdir -p $2
for i in {a..z}
    do 
        >$2/$i.txt
    done
for file in $(ls $1)
    do 
        awk '{print>>"./'$2'/"tolower(substr($1,1,1))".txt"}' ./$1/$file
    done
for file in $(ls $2)
    do 
        sort -o ./$2/$file ./$2/$file
    done
>$2/.txt
rm $2/.txt
