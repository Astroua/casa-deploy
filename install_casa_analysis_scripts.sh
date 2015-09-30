#!/bin/bash

# License under the MIT License - see LICENSE

path_to_scripts=$HOME/analysis_scripts/

wget --quiet ftp://ftp.cv.nrao.edu/pub/casaguides/analysis_scripts.tar
tar xvf analysis_scripts.tar
rm analysis_scripts.tar

echo "# Analysis scripts" >> $HOME/.casa/init.py
echo "import sys" >> $HOME/.casa/init.py
echo "sys.path.append('"${path_to_scripts}"'')"  >> $HOME/.casa/init.py
echo "import analysisUtils as au"  >> $HOME/.casa/init.py
