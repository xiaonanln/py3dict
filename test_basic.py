import sys
from py3dict import py3dict
from tbdict import tbdict 
import unittest 
import sys 

class BaseTestDict(object):
    def __init__(self, dictClass, *args, **kwargs):
        super(BaseTestDict, self).__init__(*args, **kwargs)
        self.dictClass = dictClass

    def setUp(self):
        self.d = self.dictClass()
        
    def testLen(self):
        self.d = self.dictClass()
        assert len(self.d) == 0
        self.d[1] = 1
        assert len(self.d) == 1
        for i in range(10):
            self.d[i] = i*i
        assert len(self.d) == 10

    def testSetItem(self):
        self.d = self.dictClass()
        for i in range(10):
            self.d[i] = i*i
        
        assert len(self.d) == 10
        for i in range(10):
            assert self.d[i] == i*i
        
        return 

    def testKeyError(self):
        self.d = self.dictClass()
        for i in range(10):
            try:
                self.d[i]
                assert False, self.d 
            except KeyError, e:
                assert str(e) == str(i)

    def testGet(self):
        self.d = self.dictClass()  
        for i in range(10):
            self.d[i] = i +1
        for i in range(10):
            assert self.d.get(i) == i+1, (self.d.get(i), i+1)
        for i in range(10, 20):
            assert self.d.get(i) is None

    def test__setitem__(self):
        self.d = self.dictClass()
        for i in range(10):
            self.d.__setitem__(i, i)
            assert self.d[i] == i

    def test__getitem__(self):
        self.d = self.dictClass()
        for i in range(10):
            self.d[i] = i
            assert self.d.__getitem__(i) == i

        try:
            self.d.__getitem__(1023123)
            assert False, self.d 
        except KeyError:
            pass

    # def testSizeOf(self):
    #     self.d = self.dictClass()
    #     s0 = sys.getsizeof(self.d)
    #     assert s0  > 0, (self.d, s0)
        
    #     for i in range(100):
    #         self.d[i] = i
    #     assert sys.getsizeof(self.d) > s0, (s0, sys.getsizeof(self.d))

    def testGC(self):
        self.d = self.dictClass()
        self.d[1] = self.d 

        import gc 
        gc.collect()

    def testResize(self):
        self.d = self.dictClass()
        for i in range(1000):
            self.d[i] = i
        for i in range(1000):
            del self.d[i]

    def tearDown(self):
        # self.self.d.clear()
        pass
        
class TestTBDict(BaseTestDict, unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestTBDict, self).__init__(tbdict, *args, **kwargs)

class TestPy3Dict(BaseTestDict, unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestPy3Dict, self).__init__(py3dict, *args, **kwargs)

if __name__ == '__main__':
    unittest.main()
