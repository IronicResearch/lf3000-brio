#!/bin/bash -xe
#

set -e

DEFAULT_REVISION=HEAD
DEFAULT_NAME="LF-ThirdParty-${DEFAULT_REVISION}-`date +%Y%m%d-%H%M`"

# parse command line
ARGS=`getopt -o hr:n:r: \
--long help,name:,revision: -- "$@"`

eval set -- "$ARGS"

while true ; do
	case "${1}" in
		-h|--help )	HELP=y		; shift	  ;;
		-n|--name )	NAME=${2}	; shift 2 ;;
		-r|--revision )	REVISION=${2}	; shift 2 ;;
		--)		shift		; break	  ;;

		*)	echo "Internal error!"		;
			echo "Press Enter to close..."
			read
			exit 1 ;;
	esac
done

# If show help, display info and exit
if [ "X${HELP}" == "Xy" ]
then
	echo
	echo "${0} Make public tarball, options are:"
	echo "    -h, --help      this list"
	echo "    -n, --name      name output tarball, \
default is '${DEFAULT_NAME}'"
	echo "    -r, --revision  svn revision to extract, \
default is '${DEFAULT_REVISION}'"
	echo
	echo "Press Enter to close..."
	read
	exit 0
fi


# Add user input validation, if any here

# validate REVISION setting.  Note it's part of the DEFAULT_NAME
if [ "X${REVISION}" == "X" ]
then
	REVISION=${DEFAULT_REVISION}
	echo "REVISION not set, using default '${REVISION}'"
fi

# update DEFAULT_NAME now, in case REVISION is set
DEFAULT_NAME="LF-ThirdParty-${REVISION}-`date +%Y%m%d-%H%M`"

# validate NAME setting
if [ "X${NAME}" == "X" ]
then
	NAME=${DEFAULT_NAME}
	echo "NAME not set, using default '${NAME}'"
fi

echo "Making /tmp/$NAME.tar.gz ..."

pushd $PROJECT_PATH/../
echo "exporting from svn revision '$REVISION' ..."

if [ "x$SVNUSER" = "x" ]; then
	MYSVNUSER=$USER
else
	MYSVNUSER=$SVNUSER
fi

echo "GSTEST: make_public_tarball.sh doing svn export as $MYSVNUSER"
echo "svn export --username $MYSVNUSER -r $REVISION ./Brio2/ThirdParty/ /tmp/$NAME"
svn export --username $MYSVNUSER -r $REVISION ./Brio2/ThirdParty/ /tmp/$NAME
pushd /tmp
echo "removing unneeded files and directories..."
rm -rf $NAME/MagicEyes
rm -rf $NAME/Mobileer
rm -rf $NAME/PowerVR
rm $NAME/SConscript

echo "making archive..."
tar -czf $NAME.tar.gz ./$NAME
rm -rf ./$NAME
popd
popd
echo "/tmp/$NAME.tar.gz is ready"

exit 0