#
# The MIT License
#
# Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
#                                      http://flusspferd.org/contributors.txt)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

.PHONY: all clean install uninstall test

all: build
	$(MAKE) -C build

clean: build
	$(MAKE) -C build clean

install: build
	$(MAKE) -C build install

distclean:
	rm -rf build/

build:
	@echo "Please run ./configure (with appropriate parameters)!"
	@exit 1

uninstall: build
	@if [ -f build/install_manifest.txt ]; then \
	echo 'Uninstalling' ;                       \
	xargs rm < build/install_manifest.txt ;     \
	else 					    \
	echo 'Flusspferd does not seem to be installed.'; \
	fi

test: all
	@./util/run_tests.sh
