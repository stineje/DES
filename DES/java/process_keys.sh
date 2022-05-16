#!/bin/bash

numBits=${1:-"20"}
numKeys=`wc -l plaintext.txt | awk '{print $1}'`
echo $numKeys
#java genDESkey 36 | python3 main.py > keys.txt
java genDESkey $numKeys $numBits > keys.txt
paste plaintext.txt keys.txt > plaintext_key.txt

while read -r line; do 
      echo $line | xargs java DES -q
done < plaintext_key.txt





