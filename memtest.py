from py3dict import py3dict 
from tbdict import tbdict 
import sys 
import time
import sys 
import os 

dictClass, N, avgSize = sys.argv[1:]
dictClass = eval(dictClass)
N = int(N)
avgSize = int(avgSize)

print 'Testing memory usage of %s ...' % dictClass.__name__

dicts = []
for i in xrange(N):
    d = dictClass()
    for k in xrange(avgSize):
        d[k] = 1 
    
    dicts.append(d)

totalSize = sum(sys.getsizeof(d) for d in dicts)
print 'Total size: %.1fM' % (totalSize / 1024.0 / 1024.0)
print 'Process ID:', os.getpid()
print 'sleeping ...'
while True:
    time.sleep(0.1) 
