CFLAGS := -I. -I/opt/vc/include
SRC := pc80s31.c
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.c,%.d,$(SRC))
PROG := $(patsubst %.c,%,$(SRC))

CFLAGS+=`pkg-config --cflags libusb-1.0`
LDFLAGS+=`pkg-config --libs libusb-1.0`

CFLAGS+=-Wno-deprecated-declarations -Wunused-variable -O3 -march=native
LDFLAGS+=-L/opt/vc/lib -lm -lpthread -lbcm_host

all: $(DEP)
	@$(MAKE) $(PROG)

clean:
	@$(RM) $(DEP) $(OBJ) $(PROG)

ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
-include $(DEP)
endif

%.d: %.c
	$(info GEN $@)
	@$(CC) -MM $(CFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@

%: %.d