include(QIronCommon/qironcommon.pri)

INCLUDEPATH += $${PWD}/

HEADERS += \
    $$PWD/private/qirbreadcrumbbarstyle_p.h \
    $$PWD/qirstyleoptionbreadcrumbbar.h \
    $$PWD/ui/qirbreadcrumbbar_ui.h \
    $$PWD/qirabstractbreadcrumbmodel.h \
    $$PWD/qirbreadcrumbbar.h \
    $$PWD/qirbreadcrumbbarstyle.h \
    $$PWD/breadcrumbsaagharmodel.h \
    $$PWD/breadcrumbbarsaagharstyle.h


SOURCES += \
    $$PWD/qirstyleoptionbreadcrumbbar.cpp \
    $$PWD/qirabstractbreadscrumbmodel.cpp \
    $$PWD/qirbreadcrumbbar.cpp \
    $$PWD/qirbreadcrumbbarstyle.cpp \
    $$PWD/breadcrumbsaagharmodel.cpp \
    $$PWD/breadcrumbbarsaagharstyle.cpp

