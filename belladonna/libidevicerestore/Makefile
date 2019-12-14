
CFLAGS = -I/usr/local/include -I/opt/local/include -I/usr/local/Cellar/openssl/1.0.2t/include -I. -DIDEVICERESTORE_NOMAIN -DHAVE_REALPATH -mmacosx-version-min=10.12
OBJECTS = idevicerestore.o common.o tss.o fls.o mbn.o img3.o img4.o ftab.o ipsw.o normal.o dfu.o recovery.o restore.o asr.o fdr.o limera1n.o download.o locking.o socket.o thread.o jsmn.o json_plist.o

all: libidevicerestore.a

libidevicerestore.a: $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)
	
clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a