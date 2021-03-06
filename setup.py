from distutils.core import setup, Extension

DEBUG = False
WARN_UNUSED_VARIABLES = False

undef_macros = []
extra_compile_args = ["-Wno-comments", "-std=c99"]

if not DEBUG:
    extra_compile_args.append('-DNDEBUG')
else:
    undef_macros.append('NDEBUG')
    extra_compile_args.append('-O0') 

if not WARN_UNUSED_VARIABLES:
    extra_compile_args.append('-Wno-unused-variable')

tbdictmodule = Extension(
    'tbdict', 
    sources = ['tbdict.c', 'dictimpl_tb.c'], 
    extra_compile_args = extra_compile_args, 
    undef_macros = undef_macros, 
    )

py3dictmodule = Extension(
    'py3dict', 
    sources = ['py3dict.c', 'dictimpl_py3.c'], 
    extra_compile_args = extra_compile_args, 
    undef_macros = undef_macros, 
    )

setup (name = 'py3dict',
       version = '0.1',
       description = 'py3dict is a dict implementation just like dict in Python3',
       ext_modules = [
           tbdictmodule, 
           py3dictmodule, 
       ])
