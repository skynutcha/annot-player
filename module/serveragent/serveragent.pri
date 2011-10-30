# serveragent.pri
# Since 9/10/2011

include(../../config.pri)
#include($$ROOTDIR/module/gsoap/gsoap.pri)
#include($$ROOTDIR/core/core.pri)
#include($$PWD/serversoap.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/serversoap

#DEFINES += USE_MODULE_CLIENTAGENT

HEADERS += \
    $$PWD/serveragent.h \
    $$PWD/serverproxy.h \
    $$PWD/serverproxy_config.h \
    $$PWD/serversoap.h

SOURCES += \
    $$PWD/serveragent.cc \
    $$PWD/serverproxy.cc \
    $$PWD/serversoap.cc

QT      += core

# EOF
