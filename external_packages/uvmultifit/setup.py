from distutils.core import setup, Extension
import numpy as np

#include_gsl_dir = "/opt/local/include/gsl"
include_gsl_dir = "/usr/include"

c_ext = Extension("_uvmultimodel", ["_uvmultimodel.cpp"],libraries=['gsl','gslcblas'],extra_compile_args=["-Wno-deprecated","-O3"])
setup(
    name='uvmultifit',
    py_modules=['uvmultifit'],
    ext_modules=[c_ext],
    include_dirs=[include_gsl_dir] + [np.get_include()]
)
