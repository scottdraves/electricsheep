Project has moved to https://github.com/e-dream-ai/client
This repository is now an archive only.

# Table of Contents
-Introduction
-How to Use
-Flame Algorithm 
-Building and Installing from Source
-Running from the Command Line
-Copyright and License
-Creator and Founder of Electric Sheep

# Introduction
The Electric Sheep is a cyborg mind. It harnesses the collective intelligence of 450,000 computers and people to create abstract art with mathematics and Darwinian evolution. The result is seamless, organic, and infinite. 

Electric Sheep is an open source project for anyone to use to customize and personalize their screen savers. By compiling and running the instructions of this project, you will have access to different artworks, or Sheeps which will appear on your computer after it falls asleep. Visit the official main website at http://electricsheep.org or the founder's website at http://scottdraves.comfor more detailed information on this project! 

# How to Use
In this current Github, you will find some executable files an
The rendering engine is a separate project at https://github.com/scottdraves/flam3
Here, at this link, you can find the actual instructions for Electric Sheep. The README file of the linked github provides detailed instructions with the necessary commands you will need for the project. This file contains instructions for Linux users. 
If you are a Windows Users looking to run this project, however, you can install the cygwin package. From here on, you will use the env command to follow the directions or do it manually through the command prompt. 
The example run in the github instructions is for fractal frames, however there are different fractal figures and animations available. After compiling and running the source code given, you can edit the code yourself with the different variations. The initial source code, however, will help get everything set up and running on your local device. You can also visit the website linked above to download or install the app directly to your computer if you may find that easier than looking at the code.
After you have installed and ran the main source code, you can get comfortable with using the different variations of screen savers they offer. Following this, you can also edit the code to create your own Sheep or screensavers. There is also the possibility of submitting your creation to the database of all the sheep. 

# Flame Algorithm 
You can also visit the following website: https://flam3.com/. This website is separate from the main Electric Sheep website and talks about the flame algorithm that you are installing from the source code more in the depth. 

# Building and Installing from Source

## Flame

It's recommended to first build and install flam3 before you build and install the Electric Sheep client.

For those using Debian/Ubuntu, you'll first need to install all dependencies:

```
sudo apt-get install make automake build-essential subversion autoconf libtool libgtk2.0-dev libgl1-mesa-dev libavcodec-dev libavformat-dev libswscale-dev liblua5.1-0-dev libcurl4-openssl-dev libxml2-dev libjpeg8-dev libgtop2-dev libboost-dev libboost-filesystem-dev libboost-thread-dev libtinyxml-dev freeglut3-dev libglew-dev libwxgtk3.2-dev
```

Next, clone the repo, then build and install.  Note that the configure command expects older versions of aclocal and automake and so we'll work around this by making symlinks.

```
git clone https://github.com/scottdraves/flam3.git
cd flam3
sudo ln -s $(which aclocal) /usr/local/bin/aclocal-1.15
sudo ln -s $(which automake) /usr/local/bin/automake-1.15
./configure
automake --add-missing --copy
make
sudo make install
```

## Electric Sheep

```
git clone https://github.com/scottdraves/electricsheep.git
cd electricsheep/client_generic
./autogen.sh
./configure
make
sudo make install
```

# Running from the Command Line

Once built, the main executable is:
```
electricsheep
```

You can also configure settings by running this (which is built with the main executable):
```
electricsheep-preferences
```

# Copyright and License
Copyright Spotworks LLC
GPL2 Licensed see https://github.com/scottdraves/electricsheep/blob/master/client_generic/COPYING

2015.05     moved from code.google.com repo
2011.01.30  based on revision 1546 on sf.net

#Creator and Founder of Electric Sheep
Scott Draves
