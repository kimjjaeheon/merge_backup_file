TARGET=merge_backup_file
CC=cc
INC=-I$(TRAN_SVR_HOME)/include -I$(TRAN_SVR_HOME)/lib/include -I$(TRAN_SVR_HOME)/lib/util
MY_LIB=-L$(TRAN_SVR_HOME)/lib

DEFINES = 
OPTIMIZE =
CFLAGS= $(OPTIMIZE) $(DEFINES) -g
LFLAGS=

HDRS=   main.h \
	init.h \
	find_file_list.h \
	append_file.h \
	create_MERGE_FILE.h \
	sep.h \

OBJS=   main.o \
	init.o 	\
	find_file_list.o \
	append_file.o \
	create_MERGE_FILE.o \
	sep.o \

LIBS=-lcommon -lrt -lusrutil

all:$(TARGET)
debug: CFLAGS += -D_DEBUG
debug: all

$(TARGET):$(OBJS) $(HDRS)
	$(CC) -o $(TARGET) $(INC) $(OBJS) $(MY_LIB) $(LFLAGS) $(LIBS)
	@cp -f *.c ./backup
	@cp -f *.h ./backup
	@echo "-> Successfully compiled $(TARGET)"
.c.o:
	$(CC) -c $(CFLAGS) $(INC) $<
clean:
	@rm -f core $(TARGET) $(OBJS)
install:
	@if [ -f $(TRAN_SVR_HOME)/bin/$(TARGET) ] ; then \
		mv $(TRAN_SVR_HOME)/bin/$(TARGET) $(TRAN_SVR_HOME)/bin/$(TARGET).old ; \
		cp $(TARGET) $(TRAN_SVR_HOME)/bin ; \
	else \
		cp $(TARGET) $(TRAN_SVR_HOME)/bin ; \
	fi
	@echo "-> Successfully installed $(TARGET)"; 
