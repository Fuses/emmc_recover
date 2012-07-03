CC      = /usr/bin/gcc
CFLAGS  = -m32 -Wall -O2 -Wno-unused-result
INCDIR	= -I/usr/include/ -I.
LIBDIR  = /usr/lib/
LDFLAGS = -L$(LIBDIR) -static
#LDFLAGS = -L$(LIBDIR)
RM      = rm -f
STRIP   = /usr/bin/strip
CP	= cp

TRG = emmc_recover
OBJ = gopt.o  modhandler.o pbl_reset.o emmc_recover.o

all: $(OBJ)
	$(CC) $(CFLAGS) $(INCDIR) -o $(TRG) $(OBJ) $(LDFLAGS)
	$(STRIP) --strip-all emmc_recover

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) emmc_recover
