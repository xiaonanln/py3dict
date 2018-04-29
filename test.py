from py3dict import py3dict
d = py3dict()


assert len(d) == 0, d
d[1] = 1

assert len(d) == 1, d
assert d[1] == 1, d
assert d.get(1) == 1, d 


try:
    print d['s']
    assert False, d
except KeyError:
    pass

d[0] = 1
assert d[0] == 1
assert d.get(0) == 1
assert d.get(4234123, None) is None
