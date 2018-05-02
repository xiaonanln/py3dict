# coding: utf8
import py3dict 
import tbdict 
import random 
import time 
import sys 
import string
from itertools import izip 

BENCH_N = 50000
SIZE_RANGE = 4, 16
STR_LENGTH_RANGE = 4, 32
AVG_SIZE = sum(SIZE_RANGE) // 2
US_PER_SECOND = 1000000

InsertKeys = []
CheckKeys = InsertKeys

def main():
    random.seed(0)
    print 'generating test keys ...'
    for i in range(BENCH_N):
        size = random.randint(*SIZE_RANGE)
        keys = [ randstr(*STR_LENGTH_RANGE) for _ in xrange(size) ]
        InsertKeys.append(keys)

    print 'testing ...'
    for dictClass in (dict, tbdict.tbdict, py3dict.py3dict):
        benchmark(dictClass.__name__, dictClass)

def benchmark(name, dictClass):
    dicts = [dictClass() for _ in xrange(BENCH_N)]

    t0 = time.time()
    
    for d, keys in izip(dicts, InsertKeys):
        for k in keys:
            d[k] = 1
    
    t1 = time.time()

    for d, keys in izip(dicts, CheckKeys):
        for k in keys:
            d[k]

    t2 = time.time()

    totalSize = sum( sys.getsizeof(d) for d in dicts )
    print '%-16s put %.3fus get %.3fus %dB' % (name, 
        (t1 - t0) / AVG_SIZE / BENCH_N * US_PER_SECOND, 
        (t2 - t1) / AVG_SIZE / BENCH_N * US_PER_SECOND , 
        totalSize / len(dicts))

def randstr(minlen, maxlen):
    l = random.randint(minlen, maxlen)
    return ''.join( random.choice(string.lowercase) for _ in range(l))

if __name__ == '__main__':
    main()
