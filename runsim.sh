#!/bin/bash

function usage() {
    cat <<EOF
Usage: runsim.sh <n_tasks> <util> <util cpu>
EOF
}

function cleanup() {
    rm -f $gastask_conf $simrts_conf
    rm -f /tmp/report.txt /tmp/task.txt /tmp/task_generated.txt
}

if [ $# -lt 3 ]; then
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
n_tasks=$1
util_target=$2
util_cpu=$3

COMMON_CONF="\
# wcet_sacle power_active power_idle
*cpufreq
1    100    80
0.5  25   2.5
0.25 6.25 0.625
0.125 1.5625 0.15625

# type max_capacity wcet_scale power_active power_idle
*mem
dram  1000 1    0.01   0.009
nvram 1000 0.8  0.01   0.0009

# wcet period memreq mem_active_ratio
*task"
cat <<EOF > $gastask_conf
# max_generations n_populations cutoff penalty
*genetic
3000 100 1.0 1.5

# wcet_min wcet_max mem_total util_cpu util_target n_tasks
*gentask
40 500 2000 $util_cpu $util_target $n_tasks

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

result=result_${n_tasks}_${util_target}_${util_cpu}.$$
mkdir $result
mv $simrts_res $result/simrts_res.txt
mv /tmp/report.txt $result
mv $gastask_conf $result/gastask.conf
mv $simrts_conf $result/simrts.conf
mv /tmp/task.txt $result/

cleanup
