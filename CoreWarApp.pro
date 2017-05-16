TEMPLATE = app

TARGET = CoreWar

INCLUDEPATH += .

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += gui/mainwindow.h \
		   src/Assembler.hpp \
		   src/Tokenizer.hpp \
		   src/VirtualMachine.hpp

FORMS += gui/mainwindow.ui

SOURCES += gui/mainwindow.cpp \
		   src/Assembler.cpp \
		   src/Tokenizer.cpp \
		   src/VirtualMachine.cpp \
		   src/main.cpp

CONFIG += c++11
