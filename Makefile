CFLAGS=-std=gnu++20 -pipe -Werror -Og -ggdb -lstdc++
CC=g++
LD=g++
OBJS=main.o server.o client.o
SRCS=main.cpp server.cpp client.cpp

.PHONY: clean all

all: main

clean:
	rm -f *.o main

main: $(OBJS)
	$(LD) -lssl -lcrypto $(LDFLAGS) $+ -o $@

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
