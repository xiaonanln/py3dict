from distutils.core import setup, Extension

spammodule = Extension('spam', sources = ['spam.c'])
dictv1module = Extension('dictv1', sources = ['dictv1.c'])

setup (name = 'py3dict',
       version = '0.1',
       description = 'py3dict is a dict implementation just like dict in Python3',
       ext_modules = [
           spammodule, 
           dictv1module, 
       ])