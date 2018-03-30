TEMPLATE = app

CONFIG += console
CONFIG += c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS *= -std=c++1z -fdiagnostics-color=always -Wfatal-errors

QMAKE_CXXFLAGS_DEBUG *= -g -Og

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -Ofast

INCLUDEPATH *= json/include rapidjson/include

LIBS *= -lboost_timer

linux {
    version = $$system(git describe --tags --always --dirty)
    build_timestamp = $$system(date -u '+%FT%T')
}
else {
    version = 0.0.1
    build_timestamp = 2018-02-25T09:17
}

DEFINES *= VERSION=\\\"$$version\\\" BUILD_TIMESTAMP=\\\"$$build_timestamp\\\"

debug {
    DEFINES *= #TBB_USE_DEBUG=1 SPDLOG_DEBUG_ON SPDLOG_TRACE_ON
}

#message(QMAKESPEC: $$QMAKESPEC)
message(QMAKE_CXXFLAGS: $$QMAKE_CXXFLAGS)
debug {
    message(QMAKE_CXXFLAGS_DEBUG: $$QMAKE_CXXFLAGS_DEBUG)
}else {
    message(QMAKE_CXXFLAGS_RELEASE: $$QMAKE_CXXFLAGS_RELEASE)
}
message(CONFIG: $$CONFIG)

SOURCES += \
    main.cpp \
    jsonWrapper.cpp

HEADERS += \
    getmember.h \
    getvalue.h \
    jsonWrapper.h
