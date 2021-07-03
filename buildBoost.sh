cd  download
if [ ! -f boost_1_76_0.tar.bz2 ]; then
    echo "File not found!"
    wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.bz2
fi

if [ ! -d "./boost_1_76_0 " ]; then 
    echo "Folder not found!"
    tar --bzip2 -xf boost_1_76_0.tar.bz2
fi
cd boost_1_76_0
./bootstrap.sh --prefix=../../boost/
./b2  --with-filesystem --with-system install