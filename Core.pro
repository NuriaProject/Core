#-------------------------------------------------
#
# Project created by QtCreator 2012-08-25T17:27:51
#
#-------------------------------------------------

QT       -= gui

DESTDIR = ../lib
TARGET = NuriaCoreQt$$QT_MAJOR_VERSION
TEMPLATE = lib

isEmpty(INCDIR):INCDIR=..

CONFIG += create_prl c++11
DEFINES += CORE_LIBRARY
DEFINES += NURIA_MODULE="\\\"NuriaCore\\\""

SOURCES += \
    debug.cpp \
    minilexer.cpp \
    argumentmanager.cpp \
    templateengine.cpp \
    variant.cpp \
    future.cpp \
    futurewatcher.cpp \
    callback.cpp \
    dependencymanager.cpp \
    serializer.cpp \
    conditionevaluator.cpp \
    lazyevaluation.cpp \
    lazyconditionwalker.cpp \
    metaobject.cpp \
    jsonmetaobjectreader.cpp \
    runtimemetaobject.cpp \
    qtmetaobjectwrapper.cpp \
    test.cpp \
    temporarybufferdevice.cpp \
    referencedevice.cpp \
    abstractsessionmanager.cpp \
    session.cpp \
    sessionmanager.cpp

unix: SOURCES += unixsignalhandler.cpp

HEADERS +=\
    debug.hpp \
    minilexer.hpp \
    threadlocal.hpp \
    core_global.hpp \
    argumentmanager.hpp \
    templateengine.hpp \
    variant.hpp \
    essentials.hpp \
    future.hpp \
    futurewatcher.hpp \
    callback.hpp \
    dependencymanager.hpp \
    serializer.hpp \
    conditionevaluator.hpp \
    lazyevaluation.hpp \
    lazyconditionwalker.hpp \
    metaobject.hpp \
    jsonmetaobjectreader.hpp \
    runtimemetaobject.hpp \
    qtmetaobjectwrapper.hpp \
    test.hpp \
    temporarybufferdevice.hpp \
    referencedevice.hpp \
    abstractsessionmanager.hpp \
    session.hpp \
    sessionmanager.hpp

unix: HEADERS += unixsignalhandler.hpp

# Install stuff
includes.path = $$INCDIR/nuria
includes.files = $$HEADERS

INSTALLS += includes
