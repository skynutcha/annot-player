# tr.pri
# Since 9/16/2011

HEADERS += \
    tr.h \
    translator.h

SOURCES += \
    translator.cc

TRANSLATIONS += \
    $$PWD/tr_en.ts \
    $$PWD/tr_ja.ts \
    $$PWD/tr_zh.ts

CODECFORTR = UTF-8

OTHER_FILES += $$TRANSLATIONS \
    $$PWD/lrelease_tr.cmd \
    $$PWD/lrelease_tr.sh \
    $$PWD/lupdate_tr.cmd \
    $$PWD/lupdate_tr.sh

QT += core

# EOF
