tradigitools
============

Copyright 2007 - 2013 FUNHouse Interactive.

Original Design: [Dimos Vrysellas](http://www.circusink.com/), [Charles Wardlaw](http://www.sugarandcyanide.com/)
Original C++ Conversion: [Chris Zurbrigg](http://zurbrigg.com)
Cinema 4D Version: [Bret Bays](http://bretbays.com)


tradigiTOOLs is a helpful plugin for Maya (and now Cinema 4D) that promotes a more traditional workflow for animation with digital tools.  It helps set keys and breakdowns, and has numerous workflow tools which were designed to make an animator's life easier.

It has been used extensively by studios in North America, and by many students of animation schools.

We are open sourcing the plugin in the hopes that it will gain a life of its own and continue to be used by animators, both hobbyist and professional, all over the world.

The source is released under the BSD license.  If you use the source for anything, we'd appreciate an email (and a link to your project!).


### Building

The current build uses [scons](http://www.scons.org).  Any recent version of scons should do. 

You'll need to modify the SConscript to fit your build environment, particularly the _MAYA\_DIRECTORY and INSTALL\_DIRECTORY_ variables.

If you're using bash, cd into the source tree and type:

> export MAYA_VER=2014; scons -j [number of cores]

This will choose the version of Maya you've specified, and build the plugin.

### Cinema 4D

This open source release also includes the Cinema 4D Python plugin.  Please see the cinema4D folder for instructions on installation.  If you're familiar with the Maya version, the Cinema 4D version should be right up your alley


## The Future

If you've made developments and would like to share, please don't hesitate to contact Charles ([kattkieru on github](http://www.gitub.com/kattkieru/)).

Good luck and good animating!



