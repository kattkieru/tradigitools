tradigitools
============

Copyright 2007 - 2021 Skeletal Studios

Original Design: [Dimos Vrysellas](http://www.circusink.com/), [Charles Wardlaw](http://www.skeletalstudios.com/)  
Original C++ Conversion: [Chris Zurbrigg](http://zurbrigg.com)  
Cinema 4D Version: [Bret Bays](http://bretbays.com)  


### Introduction

Tradigitools is a helpful plugin for Maya (and now Cinema 4D) that promotes a more traditional workflow for animation with digital tools.  It helps set keys and breakdowns, and has numerous workflow tools which were designed to make an animator's life easier.

It has been used extensively by studios in North America, and by many students of animation schools.

We are open sourcing the plugin in the hopes that it will gain a life of its own and continue to be used by animators, both hobbyist and professional, all over the world.

The source is released under the BSD license.  If you use the source for anything, we'd appreciate an email (and a link to your project!).


### Latest Update

The initial release had issues building because I hastily stripped the licensing module from the tree.  I've fixed that.  I've also renamed the MEL file to cie_tradigitools.mel for sourcing; the 2013 file is still there for reference (and, if I recall, no different from the one in 2014), but moving forwards I'll only be supporting Maya 2014 and 
above.

My intent is to make installers for binary builds so that it's easier for people to use the tool.

I've noticed that there's an odd issue linking on Linux (in my case, CentOS 6.2).  I'm looking into it, but if you have
any insight I'd appreciate hints.


### Building

The current build uses [scons](http://www.scons.org).  Any recent version of scons should do. 

You'll need to modify the SConscript to fit your build environment, particularly the _MAYA\_DIRECTORY and INSTALL\_DIRECTORY_ variables.

If you're using bash, cd into the source tree and type:

> export MAYA_VER=2014; scons -j [number of cores]

This will choose the version of Maya you've specified, and build the plugin.


### Cinema 4D

This open source release also includes the Cinema 4D Python plugin.  Please see the cinema4D folder for instructions on installation.  If you're familiar with the Maya version, the Cinema 4D version should be right up your alley.  Thank Bret Bays for his amazing contribution. 


## The Future

If you've made developments and would like to share, please don't hesitate to contact Charles ([kattkieru on github](http://www.github.com/kattkieru/)).

Good luck and good animating!



