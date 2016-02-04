# ClickOS Control

ClickOS is a port of the popular modular router Click, to the MiniOS operative
system, that enables Click to run under the Xen hypervisor. ClickOS Control
implements the necessary functionality to control a ClickOS instance.

## Basic functionality

ClickOS Control supports the following operations:

* Install and remove a router from a running ClickOS instance;
* Start, stop and pause installed routers;
* Read and write routers' element handlers.

## Controlling mechanisms

There are multiple communications channels to access a ClickOS instance and
configure it. Currently only xenstore is supported both on ClickOS and ClickOS
Control, but others, like vchan, are planned.


# Development

ClickOS control is currently developed as a separated project, but it's planned
to upstream it to the Click repository at a certain point.

## Limitations

The current version doesn't support element handlers.


# Build

ClickOS Control is written in C++11. It can be built with any modern C++
compiler. Tested with gcc and clang.

The only dependency is `libxenstore` from xen.

To build, simply run `make`.
