ROOT=../..

CFLAGS=-O2 -I. -I$(ROOT)/kernel -fno-builtin

LIBS=$(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a

STARTADDR=0x80150000
ENTRY=start

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

OBJS=main.o
SRCS=main.c
TARGET=dumb

all: $(TARGET)

dumb: $(OBJS) 
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

clean:
	rm -f $(OBJS) $(TARGET)



