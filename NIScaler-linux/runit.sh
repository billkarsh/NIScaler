#!/bin/sh

# You can't call NIScaler directly, rather, call it via runit.sh.
# You can call runit.sh two ways:
#
# 1) > runit.sh 'cmd-line-parameters'
# 2a) Edit parameters in runit.sh, then call it ...
# 2b) > runit.sh
#
# This script effectively says:
# "If there are no parameters sent to runit.sh, call NIScaler
# with the parameters hard coded here, else, pass all of the
# parameters through to NIScaler."
#
# Shell notes:
# - This script is provided for "sh" shell. If you need to use
# a different shell feel free to edit the script as required.
# Remember to change the first line to invoke that shell, for
# example, replace /bin/sh with /bin/bash
#
# - In most environments $0 returns the path and name of this
# script, but that is not guaranteed to be true. If using the
# bash shell, it is more reliable to define RUN_DIR like this:
# RUN_DIR=$(dirname $(readlink -f BASH_SOURCE[0]))
#
# - Enclosing whole parameter list in quotes is recommended, like this:
#
#    > runit.sh 'cmd-line-parameters'
#

if [ -z "$1" ]
then
    ARGS="-apply"
    ARGS="$ARGS -cal_dir=/mnt/d/NIFIX"
    ARGS="$ARGS -src_dir=/mnt/d/catgt_test_data/SC024_092319_NP1.0_Midbrain_g0"
    ARGS="$ARGS -dst_dir=/mnt/d/NIFIX"
else
    ARGS=$@
fi

RUN_DIR=$(dirname $(readlink -f $0))
export LD_LIBRARY_PATH=$RUN_DIR/links
$LD_LIBRARY_PATH/ld-linux-x86-64.so.2 --library-path $LD_LIBRARY_PATH $RUN_DIR/NIScaler $ARGS

