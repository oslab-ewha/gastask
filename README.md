# gastask(GA for Scheduling real-time TASKs) 
This project demonstrates that a GA(genetic algorithm) approach can provide a near-optimized power-efficient scheduling for real-time tasks
who execute with DVS(dynamic voltage scaling) and HM(Hybrid Memory).

Two executables included in this project.
- gasgen: task generation tool based on CPU and total utilization
- gastask: scheduling scheme generator based on GA  

## Build
Just make to build gastask
```
# make
```

## Run
- Create a new configuration file. Refer to gastask.conf.tmpl.
- run gasgen
```
# ./gasgen gastask.conf
```
- Tasks list will be generated into <code>task_generated.txt</code> according to gastask.conf
- paste <code>task_generated.txt</code> into the task section of gastask.conf
- run gastask
```
# ./gastask gastask.conf
```
- scheduling information is generated into <code>task.txt</code>, which can be a input to simrts.

## Batch run
- runsim.sh can do all the procedures in batch including simrts run
