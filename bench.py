# coding: utf8
import py3dict 
import tbdict 
import random 
import time 
import sys 

BENCH_N = 100000
SIZE_RANGE = 4, 10
AVG_SIZE = sum(SIZE_RANGE) // 2
US_PER_SECOND = 1000000

def main():
    for dictClass in (dict, tbdict.tbdict, py3dict.py3dict):
        random.seed(0)
        benchmark('dict', dictClass)

def benchmark(name, dictClass):
    dicts = []
    t0 = time.time()
    for i in xrange(BENCH_N):
        d = dictClass()
        size = random.randint(*SIZE_RANGE)
        for _ in xrange(size):
            d[_] = _ * _
        
        dicts.append(d)
    
    t1 = time.time()
    for d in dicts:
        size = len(d)
        for i in xrange(size):
            d[i]
    t2 = time.time()
    totalSize = sum( sys.getsizeof(d) for d in dicts )
    print '%-32s %.3fus %.3fus %dB' % (name, (t1 - t0) / AVG_SIZE / BENCH_N * US_PER_SECOND, 
        (t2 - t1) / AVG_SIZE / BENCH_N * US_PER_SECOND , totalSize / len(dicts))

if __name__ == '__main__':
    main()
