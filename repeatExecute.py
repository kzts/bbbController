import sys
import subprocess

if len(sys.argv) < 4:
    print 'input: execute file, repeat number and comamnd line arguments.'
    sys.exit()

cmd = sys.argv[1] + ' '
num = int( sys.argv[2] )

for i in range( 3, len(sys.argv) ):
    cmd = cmd + ' ' + sys.argv[i]

for i in range( 0, num ):
    #proc1 = subprocess.Popen( cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    proc1 = subprocess.call( cmd, shell=True )
    print cmd


