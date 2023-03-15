awk '{print$'2'}' $1|sort|uniq -c|awk '{print$2" "$1}'|sort -k2nr
echo
awk '{print$'1'}' $1|sort|uniq -d
awk '{print$'1'}' $1|sort|uniq -u|wc -l
