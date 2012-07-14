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

TARGET = libmpu.a
MAKE = gmake
CC = cc
#CFLAGS = -Wall -Werror -O6
CFLAGS = -Wall -Os -fno-builtin
AS = as
ASFLAGS = 
LD = ld
LDFLAGS = --oformat binary
AR = ar

INCLUDES = -I . -I ../../kernel -I ../../include
OBJS = fpu.o gdt.o interrupt.o reset_intr_mask.o tss.o virtual_mem.o \
	handler.o locore.o startup.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) rv $@ $?

clean:
	rm -f $(TARGET) *.o
