# coding: utf8
import py3dict 
import random 
import time 
import sys 

BENCH_N = 1000

def main():
    random.seed(0)
    benchmark('dict', dict)
    random.seed(0)
    benchmark('textbook dict', py3dict.py3dict)

def benchmark(name, dictClass):
    dicts = []
    t0 = time.time()
    for i in xrange(BENCH_N):
        d = dictClass()
        size = random.randint(4, 10)
        for _ in range(size):
            d[_] = _ * _
        
        dicts.append(d)
    
    t1 = time.time()
    # for d in dicts:
    #     size = len(d)
    #     for i in range(size):
    #         d[i]
    t2 = time.time()
    totalSize = sum( sys.getsizeof(d) for d in dicts )
    print '%-32s %.3f %.3f %dB' % (name, t1 - t0, t2 - t1, totalSize / len(dicts))

if __name__ == '__main__':
    main()
