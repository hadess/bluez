#!/bin/sh

if [ $# -lt 1 ] ; then
	echo "Usage: $0 FILE..."
	exit 1
fi

TESTS=$(grep '^[[:space:]]*define_test[_a-z]*("' $* | cut -d \" -f 2 | cut -d " " -f 1)
if [ -z "$TESTS" ] ; then
	TESTS=$(grep '^[[:space:]]*tester_add("' $* | cut -d \" -f 2 | cut -d " " -f 1)
fi
if [ -z "$TESTS" ] ; then
	TESTS=$(grep '^[[:space:]]*g_test_add_*func("' $* | cut -d \" -f 2 | cut -d " " -f 1)
fi
echo $TESTS
