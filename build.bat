set VS90COMNTOOLS=%VS141COMNTOOLS%
python setup.py build && copy build\lib.win-amd64-2.7\*.pyd .\* && python test.py
