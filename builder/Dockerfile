FROM ubuntu:20.04

RUN apt-get -y update && apt-get -y upgrade

#wget, gcc, python3, git, rapidjson
RUN apt-get install -y wget build-essential python3 git rapidjson-dev

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

#mongo-c
WORKDIR /app
RUN wget https://github.com/mongodb/mongo-c-driver/releases/download/1.23.0/mongo-c-driver-1.23.0.tar.gz
RUN tar xzf mongo-c-driver-1.23.0.tar.gz
RUN apt-get install -y libssl-dev libsasl2-dev
WORKDIR /app/mongo-c-driver-1.23.0/cmake-build
RUN cmake .. -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Debug
RUN cmake --build . --target install

#mongo-cxx
WORKDIR /app
RUN wget https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.7.0/mongo-cxx-driver-r3.7.0.tar.gz
RUN tar -xzf mongo-cxx-driver-r3.7.0.tar.gz
WORKDIR /app/mongo-cxx-driver-r3.7.0/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local
RUN cmake --build . --target install

#httplib
WORKDIR /app
RUN wget https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.2.tar.gz
RUN tar xzf v0.11.2.tar.gz
WORKDIR /app/cpp-httplib-0.11.2/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug
RUN cmake --build . --target install
