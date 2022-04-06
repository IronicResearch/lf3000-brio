#!/bin/bash

# make-patch-set.sh.
#
#    3/24/2008 R Dowling

# Given two directories: gmpackages and newpackages, make a new directory
# patchpackages with patch files for each package in newpackages.
#
# For each package X-V.lfp in newpackages, make a minimized XPatch-V.lfp
# by diffing md5sums found in packagefiles.md5 of a golden X-V.lfp.


# The directories: from, to
GMDIR=gmpackages
NEWDIR=newpackages

# Create output directory
PATCHDIR=patchpackages
rm -rf $PATCHDIR
mkdir -p $PATCHDIR

for n in $NEWDIR/*; do
	# Examples: 
	#   n="newpackages/Avatar1-28.lfp"
	#   g="gmpackages/Avatar1-26.4.lfp"
	#   p="patchpackages/Avatar1Patch-28.lfp"
	g1=${n/$NEWDIR/$GMDIR}
	g2=${g1/-*.lfp/}
	g=`echo $g2-*.lfp`
	p1=${n/$NEWDIR/$PATCHDIR}
	p=${p1/-/Patch-}
	echo "n=$n g=$g p=$p"
	# Make a copy of package in patch directory
	cp -v $n $p
	# Extract the packagefiles.md5 from g and n packages
	tg=`mktemp`; unzip -p $g */packagefiles.md5 > $tg
	tn=`mktemp`; unzip -p $n */packagefiles.md5 > $tn
	# For each file listed in package
	for i in `cat $tg | cut -b 35-`; do
		gm5=`grep "$i" $tg | cut -b 1-33`
		nm5=`grep "$i" $tn | cut -b 1-33`
		if [ "$gm5" == "$nm5" ]; then
			# Remove from copy of .lfp
			echo $gm5 == $nm5 $i
			zip -dq $p ${i/./*}
		else
			# Keep
			echo $gm5 != $nm5 $i
		fi
	done
	rm -f $tg $tn
done
exit 0;
