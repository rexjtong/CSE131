#!/bin/bash

name=""
pid=""
email=""

if [[ $name == "" ]]
then 
	echo "Edit this script to enter your name and partners name above"
	exit
fi

if [[ $pid == "" ]]
then
        echo "Edit this script to enter your PID and partners PID above"
        exit
fi

if [[ $email == "" ]]
then
        echo "Edit this script to enter one email where you can be reached easily"
        exit
fi


if [[ $pid == [A][0-9]* || $pid == [A][0-9]*[_][A][0-9]* || $pid == [A][0-9]*[_][A][0-9]*[_][A][0-9]* ]]
then 
	rm -rf $pid.zip
	rm -rf $pid

	mkdir $pid
	
	cp Makefile $pid/
	cp README.md $pid/		
	cp errors.h $pid/		
	cp main.cc $pid/			
	cp scanner.h $pid/		
	cp submission.sh $pid/		
	cp utility.h $pid/
	cp Project_Description.txt $pid/	
	cp errors.cc $pid/		
	cp location.h $pid/		
	cp scanner.l $pid/		
	cp utility.cc $pid/

	zip -r $pid.zip $pid/*
else 
        echo "The pid provided does not match the criteria on piazza"
        exit
fi
 
