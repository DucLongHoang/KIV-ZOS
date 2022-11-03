

echo Budu analyzovat soubor $1
echo Analyza souboru $1 > vystup.txt

echo -n "Provedeno dne " >> vystup.txt
date >> vystup.txt

file $1 >> vystup.txt

echo "Analyzu provedl: " >> vystup.txt
whoami >> vystup.txt

