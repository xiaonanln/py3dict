from distutils.core import setup, Extension

tbdictmodule = Extension(
    'tbdict', 
    sources = ['tbdict.c', 'dictimpl_tb.c'], 
    extra_compile_args = ["-Wno-comments"], 
    )

py3dictmodule = Extension(
    'py3dict', 
    sources = ['py3dict.c', 'dictimpl_py3.c'], 
    extra_compile_args = ["-Wno-comments"], 
    )

setup (name = 'py3dict',
       version = '0.1',
       description = 'py3dict is a dict implementation just like dict in Python3',
       ext_modules = [
           tbdictmodule, 
           py3dictmodule, 
       ])
