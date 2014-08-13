# CMake file for NuriaCore
# Source: https://github.com/NuriaProject/Core
cmake_minimum_required(VERSION 2.8.8)

# Base configuration
PROJECT(NuriaCore)

# Enable moc
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Dependencies
FIND_PACKAGE(Qt5Core REQUIRED)

# 
SET(NuriaCore_SRC
    src/abstractsessionmanager.cpp
    src/nuria/abstractsessionmanager.hpp
    src/argumentmanager.cpp
    src/nuria/argumentmanager.hpp
    src/callback.cpp
    src/nuria/callback.hpp
    src/conditionevaluator.cpp
    src/nuria/conditionevaluator.hpp
    src/nuria/core_global.hpp
    src/debug.cpp
    src/nuria/debug.hpp
    src/dependencymanager.cpp
    src/nuria/dependencymanager.hpp
    src/nuria/essentials.hpp
    src/jsonmetaobjectreader.cpp
    src/nuria/jsonmetaobjectreader.hpp
    src/lazyconditionwalker.cpp
    src/nuria/lazyconditionwalker.hpp
    src/lazyevaluation.cpp
    src/nuria/lazyevaluation.hpp
    src/metaobject.cpp
    src/nuria/metaobject.hpp
    src/minilexer.cpp
    src/nuria/minilexer.hpp
    src/qtmetaobjectwrapper.cpp
    src/nuria/qtmetaobjectwrapper.hpp
    src/referencedevice.cpp
    src/nuria/referencedevice.hpp
    src/runtimemetaobject.cpp
    src/nuria/runtimemetaobject.hpp
    src/serializer.cpp
    src/nuria/serializer.hpp
    src/session.cpp
    src/nuria/session.hpp
    src/sessionmanager.cpp
    src/nuria/sessionmanager.hpp
    src/temporarybufferdevice.cpp
    src/nuria/temporarybufferdevice.hpp
    src/test.cpp
    src/nuria/test.hpp
    src/nuria/threadlocal.hpp
    src/variant.cpp
    src/nuria/variant.hpp
    src/tokenizer.cpp
    src/nuria/tokenizer.hpp
)

if (UNIX)
  SET(NuriaCore_SRC ${NuriaCore_SRC}
    src/unixsignalhandler.cpp
    src/nuria/unixsignalhandler.hpp
  )
endif (UNIX)

# C++11
LIST(APPEND CMAKE_CXX_FLAGS "-std=c++11")

# Create build target
ADD_LIBRARY(NuriaCore SHARED ${NuriaCore_SRC})
QT5_USE_MODULES(NuriaCore Core)

# 
set_target_properties(
    NuriaCore PROPERTIES
    VERSION ${NURIA_VERSION}
    SOVERSION ${NURIA_SOVERSION}
)

# Install target
INSTALL(TARGETS NuriaCore 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib
)
INSTALL(DIRECTORY src/nuria/ DESTINATION include/nuria
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "private" EXCLUDE
)

# Add Tests
enable_testing()
add_unittest(NAME tst_callback)
add_unittest(NAME tst_runtimemetaobject)
add_unittest(NAME tst_debug)
add_unittest(NAME tst_serializer)
add_unittest(NAME tst_dependencymanager)
add_unittest(NAME tst_session)
add_unittest(NAME tst_jsonmetaobjectreader RESOURCES tests/tst_jsonmetaobjectreader_resources.qrc)
add_unittest(NAME tst_sessionmanager)
add_unittest(NAME tst_lazyconditionwalker)
add_unittest(NAME tst_temporarybufferdevice)
add_unittest(NAME tst_lazyevaluation)
add_unittest(NAME tst_qtmetaobjectwrapper)
add_unittest(NAME tst_variant)
add_unittest(NAME tst_referencedevice)

if (UNIX)
    add_unittest(NAME tst_unixsignalhandler)
endif (UNIX)
