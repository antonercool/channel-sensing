CONTIKI_PROJECT = channel_sensing
all: $(CONTIKI_PROJECT)

CONTIKI = ../contiki-ng
TARGET_LIBFILES += -lm
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
