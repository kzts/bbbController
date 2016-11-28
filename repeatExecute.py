#import os
import sys
import subprocess

if len(sys.argv) != 3:
    print 'input: execute file and repeat number'
    sys.exit()
#if len(sys.argv) < 4:
#    print 'input: execute file, repeat number and comamnd line arguments.'
#    sys.exit()

cmd = sys.argv[1] + ' '
num = int( sys.argv[2] )

#for i in range( 3, len(sys.argv) ):
#    cmd = cmd + ' ' + sys.argv[i]

for i in range( 0, num ):
    print "#" + "{0:03d}".format(i+1) + ": " + cmd    
    #proc = subprocess.call( cmd, shell=True )    
    #os.system(cmd)
    #proc = subprocess.Popen( cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    proc = subprocess.Popen( cmd, shell=True )
    #out   = proc1.communicate()
    #lines = out.split('\n')    
    #line = proc.stdout.readline()
    #print line
    proc.wait()
    #for j in range( 0, len(lines) ):
    #    print lines[j]    
    proc.terminate()
    
    #proc = subprocess.call( cmd, shell=True )

    



