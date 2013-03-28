################
# Example project file to build
# the OSDaB Zip code as a shared library
################

INCLUDEPATH += $${PWD}/


# DEFINES += OSDAB_ZIP_LIB OSDAB_ZIP_BUILD_LIB

# Input
HEADERS += $${PWD}/zipglobal.h\
                $${PWD}/unzip.h\
                $${PWD}/unzip_p.h\
                $${PWD}/zipentry_p.h
SOURCES += $${PWD}/zipglobal.cpp\
                $${PWD}/unzip.cpp
