
# Install CASA 4.3 on a debian based system

sudo apt-get update
sudo apt-get install -y wget bzip2 ca-certificates \
    libglib2.0-0 libxext6 libsm6 libxrender1 libpng-dev libfreetype6 libfreetype6-dev libxi6 libxrandr2 \
    libxfixes3 libxcursor1 libxinerama1 libfontconfig1 libsqlite3-0 libxslt1.1 unzip bsdmainutils \
    libcurl4-openssl-dev libxft2 xorg openbox

wget --quiet https://svn.cv.nrao.edu/casa/linux_distro/release/el6/old/casa-release-4.3.1-el6.tar.gz

sudo mkdir /usr/local/bin/CASA
sudo mv casa-release-4.3.1-el6.tar.gz /usr/local/bin/CASA
cd /usr/local/bin/CASA
sudo tar zxvf casa-release-4.3.1-el6.tar.gz
sudo rm casa-release-4.3.1-el6.tar.gz

export PATH=/usr/local/bin/CASA/casa-release-4.3.1-el6:$PATH

echo "Your username is "$HOME

cd $HOME