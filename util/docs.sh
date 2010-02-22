#!/bin/sh

doxygen=doxygen
builddir=./build
sourcedir=.

# This is a hack because CMake can't set environment vars for commands called via add_custom_command

if [ "$1" ]; then
    doxygen="$1"
fi
if [ "$2" ]; then
    builddir="$2"
fi
if [ "$3" ]; then
    sourcedir="$3"
fi

$doxygen help/Doxyfile

# this is a hack. find sth. better
$sourcedir/util/jsrepl.sh \
    $sourcedir/plugins/curl/gen-doc.js \
    $sourcedir/plugins/curl/get_options.cpp \
    > $sourcedir/plugins/curl/options.pdoc

$sourcedir/util/build_pdocs.rb
rm -f $sourcedir/plugins/curl/options.pdoc

groff -man -Thtml $builddir/flusspferd.1 > $builddir/html/flusspferd.1.html
