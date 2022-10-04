#!/bin/bash

function usage() {
    cat <<EOF
Usage: runsim.sh <n_tasks> <util> <util cpu>
         : tasks are generated by gasgen
        - or -
       runsim.sh <tasks.txt> <task set multiplier>
         : tasks are multiply populated by multiplier
EOF
}

function cleanup() {
    rm -f $gastask_conf $simrts_conf
    rm -f /tmp/report.txt /tmp/task.txt /tmp/task_generated.txt
}

if [ $# -lt 2 ]; then
    usage
    exit 1
fi

if [ $# -eq 2 ]; then
    if [ ! -r $1 ]; then
	echo "there's no task list: $1"
	exit 2
    fi
    path_tasks=`realpath $1`
    tasks_multiplier=$2
else
    n_tasks=$1
    util_target=$2
    util_cpu=$3
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

if [[ -n $path_tasks ]]; then
    repeat=$tasks_multiplier
    while [[ $repeat > 0 ]]; do
	cat $path_tasks >> $simrts_conf
	cat $path_tasks >> $gastask_conf
	repeat=$((repeat - 1))
    done
else
    $GASGEN $gastask_conf
    if [ $? -ne 0 ]; then
	cleanup
	exit 1
    fi
    cat /tmp/task_generated.txt >> $simrts_conf
    cat /tmp/task_generated.txt >> $gastask_conf
fi

$GASTASK $gastask_conf
if [ $? -ne 0 ]; then
    echo "gastask failed!!"
    cleanup
    exit 1
fi

$SIMRTS -t 1000000 $simrts_conf > $simrts_res
if [ $? -ne 0 ]; then
    echo "simrts failed!!"
    cleanup
    exit 1
fi

if [[ -n $path_tasks ]]; then
    result=result_`basename $path_tasks`_${tasks_multiplier}.$$
else
    result=result_${n_tasks}_${util_target}_${util_cpu}.$$
fi

cd -
echo "results are moved to $result"

mkdir $result
mv $simrts_res $result/simrts_res.txt
mv /tmp/report.txt $result
mv $gastask_conf $result/gastask.conf
mv $simrts_conf $result/simrts.conf
mv /tmp/task.txt $result/

cleanup
