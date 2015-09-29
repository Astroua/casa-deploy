#!/bin/bash

# License under the MIT License - see LICENSE

cd $HOME

# Install casa-pip
git clone https://github.com/radio-astro-tools/casa-python.git

python casa-python/setup_casapy_pip.py

echo "# casa-pip" >> $HOME/.bashrc
echo 'export PATH=$PATH:$HOME/.casa/bin' >> $HOME/.bashrc

sh $HOME/.bashrc
