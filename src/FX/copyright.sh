#!/bin/bash

for fl in *.cxx; do  #comment
mv $fl $fl.old
sed -e 's/2025 Joe/2026 Joe/g' $fl.old > $fl
rm -f $fl.old
done

