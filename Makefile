# Makefile for the Bank program
.PHONY: clean clean_all clean_zip all
all: Bank
CC = g++
CFLAGS = -std=c++11 -Wall -g -pedantic -DNDEBUG
# CFLAGS = -std=c++11 -Wall -Werror -pedantic -pedantic-errors -DNDEBUG
# CFLAGS = -g -Wall
CCLINK = $(CC)
OBJS = Bank.o atm.o locks.o bank_manager.o logger.o
RM = rm -f
ZIP_FILENAME = 316124221_206013914.zip
LOG_FILENAME = log.txt
# Create the zip file for submission
zip:
	zip $(ZIP_FILENAME) *.h *.cpp Makefile README
# Creating the  executable
Bank: $(OBJS)
	$(CCLINK) -o Bank $(OBJS)
# Creating the object files
Bank.o: Bank.cpp
	$(CC) -c $(CFLAGS) $< -o $@
atm.o: atm.cpp
	$(CC) -c $(CFLAGS) $< -o $@
locks.o: locks.cpp
	$(CC) -c $(CFLAGS) $< -o $@
bank_manager.o: bank_manager.cpp
	$(CC) -c $(CFLAGS) $< -o $@
logger.o: logger.cpp
	$(CC) -c $(CFLAGS) $< -o $@
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* Bank
clean_zip:
	$(RM) $(ZIP_FILENAME)
clean_log:
	$(RM) $(LOG_FILENAME)
clean_all: clean clean_zip clean_log
