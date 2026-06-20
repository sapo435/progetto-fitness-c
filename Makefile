CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
LDFLAGS = -lm
TARGET  = New_Fitness

SRCS = main_logic.c logica.c interfaccia.c server.c client_socket.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
