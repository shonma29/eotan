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

TARGET = $(WD)/cat $(WD)/shell \
		$(WD)/date $(WD)/echo $(WD)/ls $(WD)/pwd $(WD)/sleep \
		$(WD)/mkdir $(WD)/rm $(WD)/chmod $(WD)/cp $(WD)/mv \
		$(WD)/more $(WD)/cal $(WD)/ncalc $(WD)/init $(WD)/wm \
		$(WD)/nterm

include Makefile.in

ENTRY_ADDR = 0x00001000
ENTRY_FUNC = _main

all: $(TARGET)
	for I in $(^F); do \
		$(WRITER) $(INITRD) $(BLKSIZE) create /bin/$$I $(WD)/$$I; \
		$(WRITER) $(INITRD) $(BLKSIZE) chmod 755 /bin/$$I; \
	done

$(TARGET): $(OBJS) \
		lib/libwin.a lib/libdraw.a lib/libfonts.a lib/libc.a lib/libnc.a
	$(LD) -static -e $(ENTRY_FUNC) -Ttext=$(ENTRY_ADDR) -o $@ $(@).o \
		$(WD)/_perror.o $(WD)/_put_error.o \
		lib/libwin.a lib/libdraw.a lib/libfonts.a lib/libc.a lib/libnc.a
	$(STRIP) $@
