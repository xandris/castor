CFLAGS=-std=c++20 -pipe -Werror -Og -ggdb
LDFLAGS=-Wl,--as-needed
CC=g++
LD=g++
OBJS=server.o client.o uri.o
SRCS=main.cpp server.cpp client.cpp uri.cpp test_uri.cpp
TESTS=test_uri

.PHONY: clean all

all: main

clean:
	rm -f *.o main $(TESTS)

main: main.o $(OBJS)
	$(LD) -lssl -lcrypto $(LDFLAGS) $+ -o $@

tests: $(TESTS)
	for i in $(TESTS); do ./"$$i" || exit $$?; done

test_uri : test_uri.o uri.o
	$(LD) $(CFLAGS) $(LDFLAGS) $+ -o $@

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
