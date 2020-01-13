CC=gcc
CFLAGS=-Wall

DEFS = -DLINUX
CFLAGS += $(DEFS)

SRCS = P2PServer.cpp

INC = -I../

OBJS = $(SRCS:.cpp=.o)

TARGET=server

$(TARGET):$(OBJS)
	$(CC) -o $@ $^

%.o:%.cpp
	$(CC $(CFLAGS) $(INC) -o $@ -c $<

clean:
	rm -rf $(TARGET) $(OBJS)