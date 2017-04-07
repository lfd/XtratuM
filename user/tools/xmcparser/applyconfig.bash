#!/bin/bash
# replace CONFIG occurences in the <schema> with it's CONFIG=VALUE

usage(){
    echo "usage: $(basename $0) <.config> <schema> <output>"
    if ! test $# -eq 0; then
    	echo $*
    fi
    exit
}

if test $# -eq 0 || test $# -gt 3; then
    usage
fi

config=$1
if ! test -e $config; then
    usage "error: config file $config does not exist"
fi

schema=$2
if ! test -e $schema; then
    usage "error: schema file $schema does not exist"
fi

output=$3
#if test -e $output; then
#    usage "error: output file $output does exist"
#fi

. $config

echo "<?xml version=\"1.0\"?>" > $output
cat $schema >> $output
for name in `sed -n "s/.*# \(CONFIG_.*\)* #.*/\1/p" $schema`; do
    eval value=\$$name
#    echo $name = $value
    sed -i 's|# '$name' #|'$value'|' $output
done
