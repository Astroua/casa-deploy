#!/bin/bash

# License under the MIT License - see LICENSE

# Pass the location where uvmultifit is
# Also requires casa-python be installed

sudo apt-get install -y gcc g++ libgsl0ldbl gsl-bin libgsl0-dev

path_to_multifit=${1}

cd ${path_to_multifit}

casa-python setup.py install

cd $HOME
