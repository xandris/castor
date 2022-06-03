CFLAGS=-std=c++20 -pipe -Werror -Og -ggdb -fstack-protector-strong -fsanitize=address -fsanitize=undefined
LDFLAGS=$(CFLAGS) -Wl,--as-needed
CC=g++
LD=g++
OBJS=server.o client.o uri.o response.o handler/dir.o
SRCS=main.cpp server.cpp client.cpp uri.cpp test_uri.cpp request.cpp response.cpp handler/dir.cpp
TESTS=test_uri

.PHONY: clean all

all: main

clean:
	rm -rf *.o handler/*.o .deps/ main $(TESTS)

main: main.o $(OBJS)
	$(LD) -luring -lssl -lcrypto $(LDFLAGS) $+ -o $@

test_cancel: test_cancel.o
	$(LD) -lssl -lcrypto $(LDFLAGS) $+ -o $@

tests: $(TESTS)
	for i in $(TESTS); do ./"$$i" || exit $$?; done

test_uri : test_uri.o uri.o
	$(LD) $(CFLAGS) $(LDFLAGS) $+ -o $@

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$(patsubst %.cpp,%.d,$<)

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

handler/%.o : handler/%.cpp
handler/%.o : handler/%.cpp $(DEPDIR)/handler/%.d | $(DEPDIR)/handler
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(DEPDIR) $(DEPDIR)/handler: ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
