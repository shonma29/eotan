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

.PHONY: tools libs kern core drivers servers \
	apps bin test contrib data starter initrd

all: libs tools kern initrd apps data starter

tools:
	$(MAKE) -f app/sfs/Makefile WD=app/sfs
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

apps: bin test contrib

bin:
	app/sfs/statfs initrd.img mkdir /bin
	$(MAKE) -f app/bin/Makefile WD=app/bin

test:
	$(MAKE) -f app/test/Makefile WD=app/test

contrib:
	$(MAKE) -f app/contribution/pager/Makefile WD=app/contribution/pager

data: motd bees.p6
	./motd.sh > motd
	for I in $^; do \
		app/sfs/statfs initrd.img write /$$I $$I; \
		app/sfs/statfs initrd.img chmod 644 /$$I; \
	done
	app/sfs/statfs initrd.img statvfs

starter:
	app/sfs/statfs initrd.img dir /
	lib/librc/encode < initrd.img > initrd.img.rc
	mkdir -p build
	$(MAKE) -f starter/arch/Makefile WD=starter/arch

initrd:
	$(RM) initrd.img
	app/sfs/mkfs initrd.img 1024 4
	app/sfs/statfs initrd.img mkdir /lost+found
	app/sfs/statfs initrd.img chmod 1700 /lost+found

clean:
	$(MAKE) -f app/sfs/Makefile WD=app/sfs clean
	$(MAKE) -f lib/librc/Makefile WD=lib/librc clean
	$(MAKE) -f lib/Makefile clean
	$(MAKE) -f kernel/Makefile WD=kernel clean
	$(MAKE) -f drivers/Makefile clean
	$(MAKE) -f servers/Makefile clean
	$(MAKE) -f app/bin/Makefile WD=app/bin clean
	$(MAKE) -f app/test/Makefile WD=app/test clean
	$(MAKE) -f app/contribution/pager/Makefile WD=app/contribution/pager clean
	$(RM) initrd.img initrd.img.rc motd
	$(MAKE) -f starter/arch/Makefile WD=starter/arch clean
	$(RM) -rf build
