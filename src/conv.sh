#!/bin/bash

for fl in *.cxx; do  #comment
mv $fl $fl.old
sed -e 's/for(/for (/g' -e 's/while(/while (/g' -e 's/if(/if (/g' -e 's/switch(/switch (/g' $fl.old > $fl
rm -f $fl.old
done

