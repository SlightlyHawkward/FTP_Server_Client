# Makefile for Computer Networks and Internets TCP API
# Solaris Version

DEFS	= -DSOLARIS
INC_DIR	= 
INCLUDE = -I$(INC_DIR)
LD	= ld
CC	= gcc
LIBS	= -lsocket -lnsl -lpthread
FLAGS   = -c -g

APP_DIR = ../apps
APP_SRC = 	ftpserver.c		\
		ftpclient.c		\
		readln.c

APP_OBJ = $(APP_SRC:.c=.o)
APP_EXE = ftpserver ftpclient 

CFILES = $(APP_DIR)/*.c

apps: $(APP_EXE)

ftpserver: ftpserver.o readln.o
	$(CC) -o $@ $(LIBS) ftpserver.o readln.o
ftpclient: ftpclient.o readln.o
	$(CC) -o $@ $(LIBS) ftpclient.o readln.o

$(APP_OBJ):
	$(CC) -c $(INCLUDE) $(FLAGS) $(DEFS) $(APP_DIR)/$(@:.o=.c)

clean:
	\rm -rf *.o core  $(APP_EXE)

# Application Dependancies
chatclient.o: $(APP_DIR)/ftpclient.c 
chatserver.o: $(APP_DIR)/ftpserver.c 
readln.o: $(APP_DIR)/readln.c 
