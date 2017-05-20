#-------------------------------------------------
#
# Project created by QtCreator 2016-03-30T11:59:53
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SerialPort
TEMPLATE = app


SOURCES += main.cpp\
        Widget.cpp \
    qcustomplot.cpp


HEADERS  += Widget.h \
    qcustomplot.h \
    fftreal/Array.h \
    fftreal/Array.hpp \
    fftreal/def.h \
    fftreal/DynArray.h \
    fftreal/DynArray.hpp \
    fftreal/FFTReal.h \
    fftreal/FFTReal.hpp \
    fftreal/FFTRealFixLen.h \
    fftreal/FFTRealFixLen.hpp \
    fftreal/FFTRealFixLenParam.h \
    fftreal/FFTRealPassDirect.h \
    fftreal/FFTRealPassDirect.hpp \
    fftreal/FFTRealPassInverse.h \
    fftreal/FFTRealPassInverse.hpp \
    fftreal/FFTRealSelect.h \
    fftreal/FFTRealSelect.hpp \
    fftreal/FFTRealUseTrigo.h \
    fftreal/FFTRealUseTrigo.hpp \
    fftreal/OscSinCos.h \
    fftreal/OscSinCos.hpp


FORMS    += Widget.ui

QMAKE_CXXFLAGS += -std=gnu++14


