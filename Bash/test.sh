#! /bin/bash
str="Embedded"

if [ -n $str ];	# Check the string is NOT empty?
then
    echo "String is not empty"
fi

if [ -z $str ];	# Check the string is empty?
then
    echo "String is empty."
fi
