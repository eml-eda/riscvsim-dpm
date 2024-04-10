#!/bin/bash

git clone https://github.com/gvsoc/gvsoc.git
sudo apt-get install -y build-essential git doxygen python3-pip libsdl2-dev curl cmake gtkwave libsndfile1-dev rsync autoconf automake texinfo libtool pkg-config libsdl2-ttf-dev
cd gvsoc
git submodule update --init --recursive --jobs 8
pip3 install -r core/requirements.txt
pip3 install -r gapy/requirements.txt
make all

