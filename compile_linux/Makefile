# Makefile for Comer CNAI Simple TCP API
# Linux Version

DEFS	= -DLINUX
INC_DIR	= 
INCLUDE = -I$(INC_DIR)
LD	= ld
CC	= gcc
LIBS	= 
FLAGS   = -c -g

APP_DIR = ../apps
APP_SRC = 	ftpserver.c		\
		ftpclient.c		\
		readln.c

APP_OBJ = $(APP_SRC:.c=.o)
APP_EXE = ftpserver ftpclient 

CFILES = $(API_DIR)/*.c $(APP_DIR)/*.c

apps: $(APP_EXE)

ftpserver: ftpserver.o readln.o
	$(CC) -o $@ $(LIBS)  ftpserver.o readln.o
ftpclient: ftpclient.o readln.o
	$(CC) -o $@ $(LIBS)  ftpclient.o readln.o

$(APP_OBJ):
	$(CC) -c $(INCLUDE) $(FLAGS) $(DEFS) $(APP_DIR)/$(@:.o=.c)

clean:
	\rm -rf *.o core $(APP_EXE)

# Application Dependancies
ftpclient.o: $(APP_DIR)/ftpclient.c 
ftpserver.o: $(APP_DIR)/ftpserver.c 
readln.o: $(APP_DIR)/readln.c 
