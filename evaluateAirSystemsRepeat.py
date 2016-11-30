import os
import time
import datetime
import random
import sys

pressure_high  = 0.08
pressure_low   = 0.03
pressure_range = 0.3
pressure1 = pressure_low;
pressure2 = pressure_low;
wait_time = 2

# repeat num
if len(sys.argv) != 2:
    print "input repeat number"
    sys.exit()
else:
    repeat_num = int(sys.argv[1])
    print "repeat number: " + str(repeat_num)

# loop
for i in range(0,repeat_num):
    # execute
    pressure0 = pressure_range* random.random() + pressure_high
    pressure3 = pressure_range* random.random() + pressure_high
    run_motion = "./evaluateAirSystems" + " " + "{0:.3f}".format(pressure0) + " " + "{0:.3f}".format(pressure1) + " " + "{0:.3f}".format(pressure2) + " " + "{0:.3f}".format(pressure3) 
    print run_motion
    os.system(run_motion)

    # copy results
    d        = datetime.datetime.today()
    results1 = "data/results.dat"
    results2 = "data/" + "{0:04d}".format(d.year) + "{0:02d}".format(d.month) + "{0:02d}".format(d.day) + "_" + "{0:02d}".format(d.hour) + "{0:02d}".format(d.minute) + "_" + "{0:02d}".format(d.second) + ".dat"
    run_copy = "cp " + results1 + " " + results2
    print run_copy
    os.system(run_copy)

    print str(i) + " in " + "{0:.3f}".format(repeat_num) + " done: " + "{0:.3f}".format(100*i/repeat_num) + " %"
    time.sleep(wait_time)



