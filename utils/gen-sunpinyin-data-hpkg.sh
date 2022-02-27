#!/bin/sh

CURDIR=`pwd`
SCRIPTDIR=`dirname $0`

cd "$SCRIPTDIR"
mkdir -p tmp/install
mkdir -p tmp/download
mkdir -p tmp/data

wget http://mirrors.kernel.org/ubuntu/pool/universe/o/open-gram/sunpinyin-data_0.1.22%2B20170109-2_amd64.deb -O ./tmp/download/sunpinyin-data.deb

cd tmp/download && ar -vx sunpinyin-data.deb && cd -
tar Jxvf ./tmp/download/data.tar.xz -C ./tmp/data

mkdir -p ./tmp/install/lib/
cp -a ./tmp/data/usr/lib/x86_64-linux-gnu/sunpinyin ./tmp/install/lib/

mkdir -p ./tmp/install/documentation/packages/
cp -a ./tmp/data/usr/share/doc/sunpinyin-data ./tmp/install/documentation/packages/

cp ./sunpinyin-data.hpkg.info ./tmp/install/.PackageInfo
cd ./tmp/install && package create -b ~/sunpinyin-data-0.1.22.hpkg && package add ~/sunpinyin-data-0.1.22.hpkg lib documentation && cd -
rm -rf ./tmp

echo ""
echo "Run the following command to install the package:"
printf "\tcp ~/sunpinyin-data-0.1.22.hpkg /boot/system/packages/\n"

cd "$CURDIR"
