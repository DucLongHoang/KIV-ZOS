#!/bin/bash

if test -d "$1"
then
	echo "$1 je adressar"
elif test -f "$1"
then
	echo "$1 je obycejny soubor"
else
	echo "$1 neni ani adresar ani soubor"
fi
