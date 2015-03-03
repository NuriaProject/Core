# CMake file for NuriaCore
# Source: https://github.com/NuriaProject/Core
cmake_minimum_required(VERSION 2.8.8)

# Base configuration
PROJECT(NuriaCore)

cmake_policy(SET CMP0020 NEW)

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
    src/logger.cpp
    src/nuria/logger.hpp
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
    src/nuria/threadlocal.hpp
    src/variant.cpp
    src/nuria/variant.hpp
    src/tokenizer.cpp
    src/nuria/tokenizer.hpp
    src/stringutils.cpp
    src/nuria/stringutils.hpp
    src/nuria/bitutils.hpp
    src/resource.cpp
    src/nuria/resource.hpp
    src/directoryresource.cpp
    src/nuria/directoryresource.hpp
    src/invocation.cpp
    src/nuria/invocation.hpp
    src/private/invocationwaitobject.hpp
    src/objectwrapperresource.cpp
    src/nuria/objectwrapperresource.hpp
    src/jsonstreamreader.cpp
    src/nuria/jsonstreamreader.hpp
    src/private/streamingjsonhelper.cpp
    src/private/streamingjsonhelper.hpp
)

if (UNIX)
  SET(NuriaCore_SRC ${NuriaCore_SRC}
    src/unixsignalhandler.cpp
    src/nuria/unixsignalhandler.hpp
  )
endif (UNIX)

# Create build target
ADD_LIBRARY(NuriaCore SHARED ${NuriaCore_SRC})
QT5_USE_MODULES(NuriaCore Core)

# 
set_target_properties(
    NuriaCore PROPERTIES
    VERSION ${NURIA_VERSION}
    SOVERSION ${NURIA_SOVERSION}
    COMPILE_FLAGS "-msse2"
)

# Add public include directories to target
target_include_directories(NuriaCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

# Depend on Tria if enabled.
if(HasTria)
  add_dependencies(NuriaCore tria)
endif()

# Install target
INSTALL(TARGETS NuriaCore EXPORT NuriaCoreConfig
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
INSTALL(DIRECTORY src/nuria/ DESTINATION include/nuria
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "private" EXCLUDE
)

INSTALL(EXPORT NuriaCoreConfig DESTINATION lib/cmake/NuriaCore)

export(TARGETS NuriaCore FILE "${NURIA_CMAKE_PREFIX}/NuriaCoreConfig.cmake")

# Add Tests
enable_testing()
add_unittest(NAME tst_callback)
add_unittest(NAME tst_runtimemetaobject)
add_unittest(NAME tst_logger)
add_unittest(NAME tst_serializer SOURCES structures.hpp)
add_unittest(NAME tst_dependencymanager)
add_unittest(NAME tst_session)
add_unittest(NAME tst_jsonmetaobjectreader RESOURCES tests/tst_jsonmetaobjectreader_resources.qrc)
add_unittest(NAME tst_sessionmanager)
add_unittest(NAME tst_lazyconditionwalker)
add_unittest(NAME tst_temporarybufferdevice)
add_unittest(NAME tst_lazyevaluation)
add_unittest(NAME tst_qtmetaobjectwrapper)
add_unittest(NAME tst_variant)
add_unittest(NAME tst_stringutils)
add_unittest(NAME tst_referencedevice)
add_unittest(NAME tst_tokenizer)
add_unittest(NAME tst_resource)
add_unittest(NAME tst_directoryresource)
add_unittest(NAME tst_objectwrapperresource)
add_unittest(NAME tst_jsonstreamreader)

if(NOT WIN32)
  add_unittest(NAME tst_streamingjsonhelper)
else()
  add_unittest(NAME tst_streamingjsonhelper)
endif()

if (UNIX)
  add_unittest(NAME tst_unixsignalhandler)
endif (UNIX)

if (HasTria)
    add_unittest(NAME tst_tria SOURCES tst_header.hpp)
endif (HasTria)
