INCLUDES = $(all_includes) -I/usr/local/include/gphoto2
CXXFLAGS = -fexceptions -W -Wall -g -ansi -DDEBUG

bin_PROGRAMS = qtkam

qtkam_SOURCES = main.cpp MainWindow.cpp SelectCameraDialog.cpp GPInterface.cpp \
                GPMessenger.cpp
qtkam_METASOURCES = AUTO
qtkam_LDFLAGS = $(all_libraries) $(KDE_RPATH)
qtkam_LDADD = $(LIB_KDEUI) $(LIB_KHTML)

noinst_HEADERS = MainWindow.h SelectCameraDialog.h GPInterface.h GPMessenger.h
