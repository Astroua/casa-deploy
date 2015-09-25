#!/bin/bash

# License under the MIT License - see LICENSE

wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh
chmod +x miniconda.sh
./miniconda.sh -b
echo "# conda path" >> $HOME/.bashrc
echo "export PATH=/home/travis/miniconda/bin:$PATH" >> $HOME/.bashrc
conda update --yes conda

source $HOME/.bashrc