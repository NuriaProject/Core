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
    src/abstractsessionmanager.hpp
    src/argumentmanager.cpp
    src/argumentmanager.hpp
    src/callback.cpp
    src/callback.hpp
    src/conditionevaluator.cpp
    src/conditionevaluator.hpp
    src/core_global.hpp
    src/debug.cpp
    src/debug.hpp
    src/dependencymanager.cpp
    src/dependencymanager.hpp
    src/essentials.hpp
    src/jsonmetaobjectreader.cpp
    src/jsonmetaobjectreader.hpp
    src/lazyconditionwalker.cpp
    src/lazyconditionwalker.hpp
    src/lazyevaluation.cpp
    src/lazyevaluation.hpp
    src/metaobject.cpp
    src/metaobject.hpp
    src/minilexer.cpp
    src/minilexer.hpp
    src/qtmetaobjectwrapper.cpp
    src/qtmetaobjectwrapper.hpp
    src/referencedevice.cpp
    src/referencedevice.hpp
    src/runtimemetaobject.cpp
    src/runtimemetaobject.hpp
    src/serializer.cpp
    src/serializer.hpp
    src/session.cpp
    src/session.hpp
    src/sessionmanager.cpp
    src/sessionmanager.hpp
    src/templateengine.cpp
    src/templateengine.hpp
    src/temporarybufferdevice.cpp
    src/temporarybufferdevice.hpp
    src/test.cpp
    src/test.hpp
    src/threadlocal.hpp
    src/variant.cpp
    src/variant.hpp
)

if (UNIX)
  SET(NuriaCore_SRC ${NuriaCore_SRC}
    src/unixsignalhandler.cpp
    src/unixsignalhandler.hpp
  )
endif (UNIX)

# C++11
LIST(APPEND CMAKE_CXX_FLAGS "-std=c++11")
add_definitions(-DNURIA_CORE_LIBRARY)

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
INSTALL(DIRECTORY src/ DESTINATION include/nuria
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "private" EXCLUDE
)
