PC_CC=g++
PC_CFLAGS=-I/usr/include
PC_LFLAGS=-L/usr/lib -lcurl -lcrypto -lz

BFIN_CC=bfin-uclinux-g++
BFIN_CFLAGS=-I/home/allard/buildroot/output/staging/usr/include
BFIN_LFLAGS=-L/home/allard/buildroot/output/staging/usr/lib -lcurl -lcrypto -lz

fritz-curl-pc: fritz-curl.cpp
	$(PC_CC) $(PC_CFLAGS) -o bin/fritz-curl-pc fritz-curl.cpp $(PC_LFLAGS)

fritz-curl-bfin: fritz-curl.cpp
	$(BFIN_CC) $(BFIN_CFLAGS) -o bin/fritz-curl-bfin fritz-curl.cpp $(BFIN_LFLAGS)
