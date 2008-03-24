#!/bin/bash

# make-brio-patch-set.sh.
#
#    3/20/2008 R Dowling
#
# Make a minimalized BrioPatch-X.lfp from a Brio-X.lfp, by diffing
# md5sums with a golden Brio-GM.lfp.
#
# Clever trick from Dave Milici is to copy new .lfp, then simply
# delete any files that have the same md5sum as gold copies.
#
# Futher cleverness is to used packagefiles.md5 as list of files and
# md5sums.

# Our new Brio LFP
NewBrioLFP=$1
if [ "x$NewBrioLFP" = "x" ]; then
	echo "Usage: ./make-patch-set.sh Brio-X.Y.Z.T.lfp"
	exit 1
fi

# Extract a temp copy of the new lfp's packagefiles.md5
NewBrioMD5=`mktemp`
unzip -p $NewBrioLFP Brio/packagefiles.md5 > $NewBrioMD5

# The gold Brio
GMBrioLFP=Brio-0.1.47.3011.lfp
GMBrioMD5=${GMBrioLFP/.lfp/}-packagefiles.md5

# The directory which Brio unpacks into
BrioDir=Brio

PatchBrioLFP=${NewBrioLFP/Brio/BrioPatch}

echo "Making minimal patch set $PatchBrioLFP (that takes from $GMBrioLFP)"
echo "Using $GMBrioMD5 as GM base."
echo ========================================================================

cp $NewBrioLFP $PatchBrioLFP

# diff -rq $GMBrioDir $NewBrioDir
for i in `cat $GMBrioMD5 | cut -b 35-`; do
	gmm5=`grep "$i" $GMBrioMD5 | cut -b 1-33`
	newm5=`grep "$i" $NewBrioMD5 | cut -b 1-33`
	if [ "$gmm5" == "$newm5" ]; then
		# Remove from copy of .lfp
		echo $gmm5 == $newm5 $i
		zip -dq $PatchBrioLFP ${i/./$BrioDir}
	else
		# Keep
		echo $gmm5 != $newm5 $i
	fi
done

rm -f $NewBrioMD5

# Show results
echo ========================================================================
ls -l $NewBrioLFP $PatchBrioLFP
