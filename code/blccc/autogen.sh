#!/bin/sh

# This script does all the magic calls to automake/autoconf and
# friends that are needed to configure a cvs checkout. You need a
# couple of extra development tools to run this script successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.


PROJECT="The Blinkenlights Chaos Control Center"
TEST_TYPE=-f
FILE=src/blccc.c

LIBTOOL_REQUIRED_VERSION=1.3.4
AUTOCONF_REQUIRED_VERSION=2.13
AUTOMAKE_REQUIRED_VERSION=1.4


srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd $srcdir


function check_version ()
{
    if expr $1 \>= $2 > /dev/null; then
	echo "yes (version $1)"
    else
	echo "Too old (found version $1)!"
	DIE=1
    fi
}

echo
echo "I am testing that you have the required versions of libtool, autoconf," 
echo "and automake."
echo

DIE=0

# check if libtool is called libtool or glibtool for Mac OS X compatibility

if (libtoolize --version) < /dev/null > /dev/null 2>&1; then
	LIBTOOLIZE=libtoolize
else
	if (glibtoolize --version) < /dev/null > /dev/null 2>&1; then
		LIBTOOLIZE=glibtoolize
	fi
fi

echo -n "checking for libtool >= $LIBTOOL_REQUIRED_VERSION ... "
if ($LIBTOOLIZE --version) < /dev/null > /dev/null 2>&1; then
    VER=`$LIBTOOLIZE --version \
         | grep libtool | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $LIBTOOL_REQUIRED_VERSION
else
    echo
    echo "  You must have libtool installed to compile $PROJECT."
    echo "  Install the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version \
         | grep -iw autoconf | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if (automake --version) < /dev/null > /dev/null 2>&1; then
    VER=`automake --version \
         | grep automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
else
    echo
    echo "  You must have automake installed to compile $PROJECT."
    echo "  Get ftp://ftp.cygnus.com/pub/home/tromey/automake-1.4p1.tar.gz"
    echo "  (or a newer version if it is available)"
    DIE=1
fi

if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo	
    exit 1
fi


test $TEST_TYPE $FILE || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}


if test -z "$*"; then
    echo
    echo "I am going to run ./configure with no arguments - if you wish "
    echo "to pass any to it, please specify them on the $0 command line."
    echo
fi


case $CC in
    *xlc | *xlc\ * | *lcc | *lcc\ *)
	am_opt=--include-deps
    ;;
esac

if test -z "$ACLOCAL_FLAGS"; then

    acdir=`aclocal --print-ac-dir`
    m4list="glib-2.0.m4"

    for file in $m4list
    do
        if [ ! -f "$acdir/$file" ]; then
            echo
            echo "WARNING: aclocal's directory is $acdir, but..."
            echo "         no file $acdir/$file"
            echo "         You may see fatal macro warnings below."
            echo "         If these files are installed in /some/dir, set the ACLOCAL_FLAGS "
            echo "         environment variable to \"-I /some/dir\", or install"            echo "         $acdir/$file."
            echo
        fi
    done
fi

aclocal $ACLOCAL_FLAGS

# optionally feature autoheader
(autoheader --version)  < /dev/null > /dev/null 2>&1 && autoheader

$LIBTOOLIZE --copy --force

automake --add-missing $am_opt
autoconf

cd $ORIGDIR

$srcdir/configure --enable-maintainer-mode "$@"

echo 
echo "Now type 'make' to compile $PROJECT."
