FROM ubuntu:20.04

RUN apt-get -y update && apt-get -y upgrade

#wget, gcc, python3, git, rapidjson
RUN apt-get install -y wget build-essential python3 git rapidjson-dev

#arrow
RUN apt-get install -y -V ca-certificates lsb-release
RUN wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
RUN apt-get install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
RUN apt-get update
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get install -y -V libarrow-dev

#root requirments
RUN apt-get install -y ca-certificates curl davix-dev dcap-dev fonts-freefont-ttf g++ gcc gfortran libafterimage-dev libcfitsio-dev libfcgi-dev libfftw3-dev libfreetype6-dev libftgl-dev libgfal2-dev libgif-dev libgl2ps-dev libglew-dev libglu-dev libgraphviz-dev libgsl-dev libjpeg-dev liblz4-dev liblzma-dev libmysqlclient-dev libpcre++-dev libpng-dev libpq-dev libsqlite3-dev libssl-dev libtbb-dev libtiff-dev libx11-dev libxext-dev libxft-dev libxml2-dev libxpm-dev libxxhash-dev libz-dev libzstd-dev locales make python3-dev python3-numpy srm-ifce-dev unixodbc-dev python-is-python3

#cmake
WORKDIR /app
RUN wget https://github.com/Kitware/CMake/releases/download/v3.23.4/cmake-3.23.4-linux-x86_64.sh
RUN chmod 777 cmake-3.23.4-linux-x86_64.sh
RUN echo y | ./cmake-3.23.4-linux-x86_64.sh
ENV PATH="${PATH}:/app/cmake-3.23.4-linux-x86_64/bin"
RUN cmake --version

#boost
WORKDIR /app
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.gz
RUN tar xzf boost_1_75_0.tar.gz
WORKDIR /app/boost_1_75_0
RUN ls -l
RUN ./bootstrap.sh
RUN ./b2 install

#root
WORKDIR /app
RUN git clone --branch latest-stable --depth=1 https://github.com/root-project/root.git root_src
WORKDIR /app/root_build
RUN cmake -Dbuiltin_xrootd=ON ../root_src # && check cmake configuration output for warnings or errors
RUN cmake --build . --target install -j8

#mongo-c
WORKDIR /app
RUN wget https://github.com/mongodb/mongo-c-driver/releases/download/1.23.0/mongo-c-driver-1.23.0.tar.gz
RUN tar xzf mongo-c-driver-1.23.0.tar.gz
RUN apt-get install -y libssl-dev libsasl2-dev
WORKDIR /app/mongo-c-driver-1.23.0/cmake-build
RUN cmake .. -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Debug
RUN cmake --build . --target install -j8

#mongo-cxx
WORKDIR /app
RUN wget https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.7.0/mongo-cxx-driver-r3.7.0.tar.gz
RUN tar -xzf mongo-cxx-driver-r3.7.0.tar.gz
WORKDIR /app/mongo-cxx-driver-r3.7.0/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local
RUN cmake --build . --target install -j8

#httplib
WORKDIR /app
RUN wget https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.2.tar.gz
RUN tar xzf v0.11.2.tar.gz
WORKDIR /app/cpp-httplib-0.11.2/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug
RUN cmake --build . --target install -j8

RUN apt install -y curl libcurl4-gnutls-dev build-essential gfortran libmysqlclient-dev xorg-dev libglu1-mesa-dev libfftw3-dev libxml2-dev git unzip autoconf automake autopoint texinfo gettext libtool libtool-bin pkg-config bison flex libperl-dev libbz2-dev swig liblzma-dev libnanomsg-dev rsync lsb-release environment-modules libglfw3-dev libtbb-dev python3-dev python3-venv python3-pip graphviz libncurses-dev software-properties-common gtk-doc-tools
RUN add-apt-repository ppa:alisw/ppa
RUN apt update
RUN apt install -y python3-alibuild
