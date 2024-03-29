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

TARGET = $(WD)/nscm

include Makefile.in

ENTRY_ADDR = 0x00001000
ENTRY_FUNC = _main

all: $(TARGET)
	for I in $(^F); do \
		$(WRITER) $(INITRD) $(BLKSIZE) create /bin/$$I $(WD)/$$I; \
		$(WRITER) $(INITRD) $(BLKSIZE) chmod 755 /bin/$$I; \
	done

$(TARGET): $(WD)/nscm.o $(WD)/parse.o $(WD)/interpret.o $(WD)/base.o \
		$(WD)/repl.o $(WD)/pcompile.o \
		lib/libc.a lib/libnc.a
	$(LD) -static -e $(ENTRY_FUNC) -Ttext=$(ENTRY_ADDR) -o $@ $^ \
		lib/libc.a lib/libnc.a
	$(STRIP) $@

$(WD)/nscm.o: $(WD)/nscm.c $(WD)/nscm.h $(WD)/parse.h $(WD)/vm.h
$(WD)/parse.o: $(WD)/parse.c $(WD)/parse.h $(WD)/nscm.h
$(WD)/interpret.o: $(WD)/interpret.c $(WD)/nscm.h $(WD)/vm.h
$(WD)/base.o: $(WD)/base.c $(WD)/nscm.h $(WD)/vm.h
$(WD)/repl.o: $(WD)/repl.c $(WD)/nscm.h $(WD)/vm.h
$(WD)/pcompile.o: $(WD)/pcompile.c $(WD)/nscm.h $(WD)/vm.h
