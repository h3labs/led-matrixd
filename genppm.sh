#!/bin/bash

cwd=`pwd`
cd /home/pi/signcfg

echo "Converting cards..."
for F in `ls -1 bak/*.png`
do
   echo $F .. `basename $F .png`.ppm
   convert $F `basename $F .png`.ppm
done

echo
echo "Converting sprites..."
for F in `ls -1 bak/sprites/*.png`
do
   echo $F .. `basename $F .png`.ppm
   convert $F sprites/`basename $F .png`.ppm
done
#exit
echo
echo "Converting atom..."
for F in `ls -1 bak/atom/*.png`
do
   echo $F .. `basename $F .png`.ppm
   convert $F atom/`basename $F .png`.ppm
done

echo
echo "Converting day banners..."
for F in `ls -1 bak/daybanners/*.png`
do
    echo $F .. `basename $F .png`.ppm
    convert $F daybanners/`basename $F .png`.ppm
done

cd $cwd

