#!/bin/bash

echo $USER | tr [a-z] [A-Z] > autor.txt

case $1 in
	-a) ln -s $2 slink ;;
	-b) cat $2 | head -3 | tail -1 ;;
	-c) chmod 740 $2 ;;
	*) echo Neplatny parametr ;;
esac


