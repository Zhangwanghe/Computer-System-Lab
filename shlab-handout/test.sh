#/bin/bash
for((i = 1; i <= 9; i++)); 
do
    text="trace0${i}.txt"
    ./sdriver.pl -t $text -s ./tsh -a "-p" > result
    ./sdriver.pl -t $text -s ./tshref -a "-p" > result_ref
    diff result result_ref > "diff${i}"
    echo -e "${i}test finish\n\n"
done

for((i = 10; i <= 16; i++)); 
do
    text="trace${i}.txt"
    ./sdriver.pl -t $text -s ./tsh -a "-p" > result
    ./sdriver.pl -t $text -s ./tshref -a "-p" > result_ref
    diff result result_ref > "diff${i}"
    echo -e "${i}test finish\n\n"
done