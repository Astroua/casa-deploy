#!/bin/bash

# License under the MIT License - see LICENSE

# Installs the Aegean source finder to the Miniconda environment
# https://github.com/PaulHancock/Aegean
# Miniconda MUST be installed!

cd $HOME

git clone https://github.com/PaulHancock/Aegean.git

$HOME/miniconda/bin/conda install --yes scipy lmfit astropy blist

echo "# Aegean" >> $HOME/.bashrc
echo "PATH="$PATH:$HOME/Aegean/"
