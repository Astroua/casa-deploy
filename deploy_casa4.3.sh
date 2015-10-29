# License under the MIT License - see LICENSE

# Install CASA 4.3 on a debian based system

# General software install
# sh $HOME/casa-deploy/general_install.sh

# One argument must be given to the casa_initialize.py script in this repo.

# CASA specifics install
sudo apt-get update
sudo apt-get install -y  \
    libglib2.0-0 libxext6 libsm6 libxrender1 libpng-dev libfreetype6 libfreetype6-dev libxi6 \
    libxrandr2 libxfixes3 libxcursor1 libxinerama1 libfontconfig1 libsqlite3-0 libxslt1.1 \
    bsdmainutils libcurl4-openssl-dev libxft2

wget --quiet https://svn.cv.nrao.edu/casa/linux_distro/release/el6/casa-release-4.3.1-el6.tar.gz

sudo mkdir /usr/local/bin/CASA
sudo mv casa-release-4.3.1-el6.tar.gz /usr/local/bin/CASA
cd /usr/local/bin/CASA
sudo tar zxvf casa-release-4.3.1-el6.tar.gz
sudo rm casa-release-4.3.1-el6.tar.gz

# Change the ownership of casa to the user
sudo chown -R $USER:$USER /usr/local/bin/CASA

echo 'export PATH=/usr/local/bin/CASA/casa-release-4.3.1-el6/bin/:$PATH' >> $HOME/.profile
echo 'export PATH=/usr/local/bin/CASA/casa-release-4.3.1-el6/bin/:$PATH' >> $HOME/.bashrc

# CASA needs to be initialized, so make it run an empty script
/usr/local/bin/CASA/casa-release-4.3.1-el6/bin/casa --nologger --log2term -c ${1}
