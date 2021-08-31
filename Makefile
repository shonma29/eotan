# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

TARGET = boot.iso

MKFS = app/tfs/mkfs
WRITER = app/tfs/writer
INITRD = initrd.img
BLKSIZE = 512

.PHONY: tools libs kern core drivers servers \
	apps bin rclib test data starter initrd

all: libs tools kern initrd apps data starter

tools:
	$(MAKE) -f app/tfs/Makefile WD=app/tfs
	$(MAKE) -f lib/librc/Makefile WD=lib/librc

libs:
	$(MAKE) -f lib/Makefile

kern: core drivers servers

core:
	$(MAKE) -f kernel/Makefile WD=kernel

drivers:
	$(MAKE) -f drivers/Makefile

servers:
	$(MAKE) -f servers/Makefile

apps: bin rclib test nscm ne

bin:
	$(WRITER) $(INITRD) $(BLKSIZE) mkdir /bin
	$(MAKE) -f app/bin/Makefile WD=app/bin

rclib:
	$(WRITER) $(INITRD) $(BLKSIZE) mkdir /rc
	$(WRITER) $(INITRD) $(BLKSIZE) mkdir /rc/lib
	$(WRITER) $(INITRD) $(BLKSIZE) create /rc/lib/rcmain rcmain

test:
	$(MAKE) -f app/test/Makefile WD=app/test

nscm:
	$(MAKE) -f app/nscm/Makefile WD=app/nscm

ne:
	$(MAKE) -f app/ne/Makefile WD=app/ne

data: motd bees.p6
	./motd.sh > motd
	for I in $^; do \
		$(WRITER) $(INITRD) $(BLKSIZE) create /$$I $$I; \
		$(WRITER) $(INITRD) $(BLKSIZE) chmod 644 /$$I; \
	done

starter:
	$(WRITER) $(INITRD) $(BLKSIZE) ls /
	mkdir -p build
	$(MAKE) -f starter/arch/Makefile WD=starter/arch

initrd:
	$(RM) $(INITRD)
	$(MKFS) $(INITRD) 512 1024
	$(WRITER) $(INITRD) $(BLKSIZE) chmod 755 /

clean:
	$(MAKE) -f app/tfs/Makefile WD=app/tfs clean
	$(MAKE) -f lib/librc/Makefile WD=lib/librc clean
	$(MAKE) -f lib/Makefile clean
	$(MAKE) -f kernel/Makefile WD=kernel clean
	$(MAKE) -f drivers/Makefile clean
	$(MAKE) -f servers/Makefile clean
	$(MAKE) -f app/bin/Makefile WD=app/bin clean
	$(MAKE) -f app/test/Makefile WD=app/test clean
	$(MAKE) -f app/nscm/Makefile WD=app/nscm clean
	$(MAKE) -f app/ne/Makefile WD=app/ne clean
	$(RM) $(INITRD) motd
	$(MAKE) -f starter/arch/Makefile WD=starter/arch clean
	$(RM) -rf build
