CPPFLAGS = -Iinclude
CFLAGS = -g -Werror -O3
CC = gcc
LDLIBS = -lncurses
OBJ_DIR = obj/

OBJ = $(OBJ_DIR)client.o $(OBJ_DIR)server.o

vpath %.c src/
vpath %.h include/

main : $(OBJ)
	$(CC) $(CFLAGS) -o client $(OBJ_DIR)client.o $(LDLIBS)
	$(CC) $(CFLAGS) -o server $(OBJ_DIR)server.o

$(OBJ_DIR)%.o : %.c common.h %.h | $(OBJ_DIR) log
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(LDLIBS) $< -o $@

clean : 
	@rm -r client server $(OBJ_DIR) log/

$(OBJ_DIR):
	@mkdir $@

log:
	@mkdir $@