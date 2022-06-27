
TEMPLATE = app
TARGET   = NIScaler

win32 {
    DEFINES += HAVE_NIDAQmx

    DESTDIR = C:/Users/labadmin/Desktop/SGLBUILD/FIXU/NIScaler/NIScaler-win
#    DESTDIR = C:/Users/labadmin/Desktop/SGLBUILD/FIXU/NIScaler/Debug
}

unix {
    DESTDIR = /home/billkarsh/Code/NIScaler/NIScaler-linux
}

QT += widgets

HEADERS +=              \
    CGBL.h              \
    Cmdline.h           \
    KVParams.h          \
    NIDAQmx.h           \
    SGLTypes.h          \
    Subset.h            \
    Tool.h              \
    Util.h

SOURCES +=              \
    main.cpp            \
    CGBL.cpp            \
    Cmdline.cpp         \
    KVParams.cpp        \
    Subset.cpp          \
    Tool.cpp            \
    Util.cpp            \
    Util_osdep.cpp

win32 {
    QMAKE_LIBDIR += $${_PRO_FILE_PWD_}
    LIBS    += -lWS2_32 -lUser32 -lwinmm -lNIDAQmx
    DEFINES += _CRT_SECURE_NO_WARNINGS WIN32
}

QMAKE_TARGET_COMPANY = Bill Karsh
QMAKE_TARGET_PRODUCT = NIScaler
QMAKE_TARGET_DESCRIPTION = Corrects SpikeGLX NI voltages
QMAKE_TARGET_COPYRIGHT = (c) 2022, Bill Karsh, All rights reserved
VERSION = 1.1


