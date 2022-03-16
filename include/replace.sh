#!/bin/bash

for f in `dir -d $3`; do
  echo "Changing $f"
  cat $f | sed s/$1/$2/g > tmp.txt
  mv tmp.txt $f
done
