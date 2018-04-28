from distutils.core import setup, Extension

py3dictmodule = Extension('py3dict', sources = ['py3dict.c', 'dictimpl.c'])

setup (name = 'py3dict',
       version = '0.1',
       description = 'py3dict is a dict implementation just like dict in Python3',
       ext_modules = [
           py3dictmodule, 
       ])