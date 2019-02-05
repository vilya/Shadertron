#-------------------------------------------------
#
# Project created by QtCreator 2019-01-08T11:39:17
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Shadertron
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

macx {
  CONFIG += gl41
}

gl41 {
  DEFINES += SHADERTOOL_USE_GL41
}


SOURCES += \
    src/main.cpp \
    src/RenderWidget.cpp \
    src/Timer.cpp \
    src/FPSCounter.cpp \
    src/ShaderToy.cpp \
    src/RenderData.cpp \
    src/AppWindow.cpp \
    src/FileCache.cpp \
    src/TextureVideoSurface.cpp \
    src/ShaderToyDownloadForm.cpp \
    src/AboutDialog.cpp \
    src/LogWidget.cpp \
    src/TextureAudioSurface.cpp \
    src/Preferences.cpp

HEADERS += \
    src/RenderWidget.h \
    src/Timer.h \
    src/FPSCounter.h \
    src/ShaderToy.h \
    src/RenderData.h \
    src/AppWindow.h \
    src/FileCache.h \
    src/TextureVideoSurface.h \
    src/ShaderToyDownloadForm.h \
    src/AboutDialog.h \
    src/LogWidget.h \
    src/TextureAudioSurface.h \
    src/Preferences.h

FORMS +=

RESOURCES += \
    resources/resources.qrc

macx {
  QMAKE_INFO_PLIST = macos/Info.plist
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
