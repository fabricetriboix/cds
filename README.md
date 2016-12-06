CDS: Data structures for C
==========================


Introduction
------------

cds is dual-licensed under the GPLv3 and a commercial license. If you
require the commercial license, please contact me:
"Fabrice Triboix" <ftriboix-at-gmail-dot-com>.

This project has been created for two reasons:
 - Help me improve my skils on CS, which are lacking
 - Provide some foundations for my other projects


Getting started
---------------

To build and install on Linux, a Makefile is provided. For other
targets, you will have to come up with your own build system.

First install [rtsys](https://github.com/fabricetriboix/rtsys). Then
follow the steps:

    $ vim Makefile                    # Adjust your settings
    $ make                            # Build
    $ make test                       # Run unit tests
    $ make install PREFIX=/your/path  # Install into PREFIX

The Makefile will use ccache if available. You can disable it by adding
`CCACHE=` on the command line, like so:

    $ make CCACHE=

Read the doxygen documentation to learn how to use cds.


No warranties
-------------

I wrote these pieces of code on my spare time in the hope that they will
be useful. I make no warranty at all on their suitability for use in
any type of software application.


Copyright
---------

This software is dual-license under the GPLv3 and a commercial license.
The text of the GPLv3 is available in the [LICENSE](LICENSE) file.

A commercial license can be provided if you do not wish to be bound by
the terms of the GPLv3, or for other reasons. Please contact me for more
details.


Coverity status
---------------

![Coverity scan build status](https://scan.coverity.com/projects/9308/badge.svg)

This project is regularily scanned through Coverity.
[Click here](https://scan.coverity.com/projects/fabricetriboix-cds)
for its status.
