#!/bin/bash

function usage() {
    cat <<EOF
Usage: runsim.sh <util>
EOF
}

function cleanup() {
    rm -f $gastask_conf $simrts_conf
    rm -f /tmp/report.txt /tmp/task.txt /tmp/task_generated.txt
}

if [ $# -lt 1 ]; then
    usage
    exit 1
fi

if [[ -z $GASTASK ]]; then
    GASTASK=`realpath ./gastask`
fi
if [[ -z $GASGEN ]]; then
    GASGEN=`realpath ./gasgen`
fi
if [[ -z $SIMRTS ]]; then
    SIMRTS=`realpath ./simrts`
fi

gastask_conf=/tmp/.gastask_conf.$$
simrts_conf=/tmp/.simrts_conf.$$
simrts_res=/tmp/.simrts.result.$$
util_gen=$1

COMMON_CONF="\
# wcet_sacle power_active power_idle
*cpufreq
1    10    8
0.7  6.7  4.3

# type max_capacity wcet_scale power_active power_idle
*mem
nvram 1000 0.7 0.001  0.0009
dram  1000 1   0.01   0.005

# wcet period memreq mem_active_ratio
*task"
cat <<EOF > $gastask_conf
# max_generations n_populations cutoff penalty
*genetic
100000 100 1.0001 0.5

# wcet_min wcet_max mem_total mem_power util_cpu util_mem n_tasks
*gentask
100 300 2000 2 $util_gen 0.95 100

$COMMON_CONF
EOF

cat <<EOF > $simrts_conf
$COMMON_CONF
EOF

cd /tmp
$GASGEN $gastask_conf
if [ $? -ne 0 ]; then
    cleanup
    exit 1
fi

cat /tmp/task_generated.txt >> $simrts_conf
cat /tmp/task_generated.txt >> $gastask_conf

$GASTASK $gastask_conf
if [ $? -ne 0 ]; then
    cleanup
    exit 1
fi

$SIMRTS -t 1000000 $simrts_conf > $simrts_res
if [ $? -ne 0 ]; then
    cleanup
    exit 1
fi

cd -
echo "results are moved to result.$$"
mkdir result.$$
mv $simrts_res result.$$/simrts_res.txt
mv /tmp/report.txt result.$$/
mv $gastask_conf result.$$/gastask.conf
mv $simrts_conf result.$$/simrts.conf
mv /tmp/task.txt result.$$/

cleanup
