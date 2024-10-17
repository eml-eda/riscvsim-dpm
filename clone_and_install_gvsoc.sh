#!/bin/bash

#clone the gvsoc repository
git clone https://github.com/eml-eda/gvsoc.git

#install prerequirements
sudo apt-get install -y build-essential git doxygen python3-pip libsdl2-dev curl cmake gtkwave libsndfile1-dev rsync autoconf automake texinfo libtool pkg-config libsdl2-ttf-dev

#move to gvsoc home directory, install dependencies and simulator
cd gvsoc
git submodule update --init --recursive --jobs 4
pip3 install -r core/requirements.txt
pip3 install -r gapy/requirements.txt
make all

#install dramsys5 library
source dramsys_pushbutton.sh

