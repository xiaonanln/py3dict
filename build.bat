set VS90COMNTOOLS=%VS110COMNTOOLS%
python setup.py build && copy build\lib.win-amd64-2.7\*.pyd .\*