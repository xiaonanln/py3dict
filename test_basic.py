import sys
from py3dict import py3dict
import unittest 
import sys 

class TestPy3Dict(unittest.TestCase):
    def setUp(self):
        # self.d = py3dict()
        pass
        
    def testLen(self):
        d = py3dict()
        assert len(d) == 0
        d[1] = 1
        assert len(d) == 1
        for i in range(10):
            d[i] = i*i
        assert len(d) == 10

    def testSetItem(self):
        d = py3dict()
        for i in range(10):
            d[i] = i*i
        
        assert len(d) == 10
        for i in range(10):
            assert d[i] == i*i
        
        return 

    def testKeyError(self):
        d = py3dict()
        for i in range(10):
            try:
                d[i]
                assert False, d 
            except KeyError, e:
                assert str(e) == str(i)

    def testGet(self):
        d = py3dict()  
        for i in range(10):
            d[i] = i +1
        for i in range(10):
            assert d.get(i) == i+1
        for i in range(10, 20):
            assert d.get(i) is None

    def test__setitem__(self):
        d = py3dict()
        for i in range(10):
            d.__setitem__(i, i)
            assert d[i] == i

    def test__getitem__(self):
        d = py3dict()
        for i in range(10):
            d[i] = i
            assert d.__getitem__(i) == i

        try:
            d.__getitem__(1023123)
            assert False, d 
        except KeyError:
            pass

    def testSizeOf(self):
        d = py3dict()
        s0 = sys.getsizeof(d)
        assert s0  > 0, (d, s0)
        
        for i in range(10):
            d[i] = i
        assert sys.getsizeof(d) > s0, (s0, sys.getsizeof(d))

    def testGC(self):
        d = py3dict()
        d[1] = d 

        import gc 
        gc.collect()

    def testResize(self):
        d = py3dict()
        for i in range(1000):
            d[i] = i
        for i in range(1000):
            del d[i]

    def tearDown(self):
        # self.d.clear()
        pass
        
if __name__ == '__main__':
    unittest.main()
