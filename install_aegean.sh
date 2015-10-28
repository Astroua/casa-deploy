#!/bin/bash

# License under the MIT License - see LICENSE

# Installs the Aegean source finder to the Miniconda environment
# https://github.com/PaulHancock/Aegean

cd $HOME

git clone https://github.com/PaulHancock/Aegean.git

echo "# Aegean" >> $HOME/.bashrc
echo "PATH="$PATH:$HOME/Aegean/"
