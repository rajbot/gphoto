# Environmental settings
QTDIR=/usr/local/qt
KDEDIR=/opt/kde
INCLUDES=-I$(QTDIR)/include -I$(KDEDIR)/include -I$(QTDIR)/include \
         -I/usr/local/include/gphoto2
LIBRARIES=-L$(KDEDIR)/lib -L$(QTDIR)/lib/ -L/usr/local/lib -lgphoto2 -lkdecore -lkdeui -lkhtml -lqt 

# Compile options. 
CCFLAGS=-W -Wall -g -ansi -pedantic -DDEBUG # -fno-rtti
CC=g++

# Objects
OBJECTS=main.o MainWindow.o moc_MainWindow.o \
			SelectCameraDialog.o moc_SelectCameraDialog.o \
            GPInterface.o 

PROGNAME=qtkam

all: $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) $(LIBRARIES) -o $(PROGNAME)

clean:
	-rm -f $(OBJECTS) moc_* $(PROGNAME) core *~

%.o: %.cpp
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ -c $<

moc_%.cpp: %.h
	$(QTDIR)/bin/moc $< -o $@
