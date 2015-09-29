# License under the MIT License - see LICENSE

# Install CASA 4.3 on a debian based system

# General software install
# sh $HOME/casa-deploy/general_install.sh

# CASA specifics install
sudo apt-get update
sudo apt-get install -y  \
    libglib2.0-0 libxext6 libsm6 libxrender1 libpng-dev libfreetype6 libfreetype6-dev libxi6 \
    libxrandr2 libxfixes3 libxcursor1 libxinerama1 libfontconfig1 libsqlite3-0 libxslt1.1 \
    bsdmainutils libcurl4-openssl-dev libxft2

wget --quiet https://svn.cv.nrao.edu/casa/linux_distro/release/el6/old/casa-release-4.3.1-el6.tar.gz

sudo mkdir /usr/local/bin/CASA
sudo mv casa-release-4.3.1-el6.tar.gz /usr/local/bin/CASA
cd /usr/local/bin/CASA
sudo tar zxvf casa-release-4.3.1-el6.tar.gz
sudo rm casa-release-4.3.1-el6.tar.gz

echo 'export PATH=/usr/local/bin/CASA/casa-release-4.3.1-el6:$PATH' >> $HOME/.bashrc

echo "Your username is "$HOME

cd $HOME
