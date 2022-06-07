CFLAGS=-std=c++20 -pipe -Werror -Og -ggdb -fstack-protector-strong -fsanitize=address -fsanitize=undefined
CFLAGS=-std=c++20 -pipe -Werror -O3 -march=native -fgcse-sm -fgcse-las -fgraphite -fgraphite-identity -floop-nest-optimize -ftree-vectorize -fvariable-expansion-in-unroller -fstack-protector-strong -flto
LDFLAGS=$(CFLAGS) -Wl,--as-needed
CC=g++
LD=g++
OBJS=server.o client.o uri.o response.o handler/dir.o
SRCS=main.cpp server.cpp client.cpp uri.cpp test_uri.cpp request.cpp response.cpp handler/dir.cpp
TESTS=test_uri
USE_PCH=1
.PRECIOUS: 

ifdef USE_PCH
PCH=net-types.hpp.gch
else
PCH=
endif

.PHONY: clean all

all: main

clean:
	rm -rf *.o *.gch handler/*.o .deps/ handler/.deps/ main $(TESTS)

main: main.o $(OBJS)
	$(LD) -luring -lssl -lcrypto $(LDFLAGS) $+ -o $@

test_cancel: test_cancel.o
	$(LD) -lssl -lcrypto $(LDFLAGS) $+ -o $@

tests: $(TESTS)
	for i in $(TESTS); do ./"$$i" || exit $$?; done

test_uri : test_uri.o uri.o
	$(LD) $(CFLAGS) $(LDFLAGS) $+ -o $@

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(dir $<)/$(DEPDIR)/$(patsubst %.hpp,%.hpp.d,$(patsubst %.cpp,%.d,$(notdir $<)))

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

.PRECIOUS: %.hpp.gch

%.hpp.gch: %.hpp | $(DEPDIR)
	$(COMPILE.c) $<

%.o : %.cpp
%.o : %.cpp $(PCH) | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(DEPDIR) handler/$(DEPDIR):
	@mkdir -p $@

DEPFILES := $(foreach src,$(SRCS),$(dir $(src)).deps/$(patsubst %.cpp,%.d,$(notdir $(src))))
# $(DEPFILES):

include $(wildcard $(DEPFILES))
