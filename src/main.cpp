// Copyright 2019 Vilya Harvey
#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QSurfaceFormat>

#include "AppWindow.h"
#include "ShaderToy.h"
#include "RenderWidget.h"


#ifdef _WIN32
// If we're on a machine with both an integrated GPU and a discrete GPU,
// these magic symbols tell the nvidia & amd drivers respectively to please
// use the discrete GPU for this program.
extern "C" {
  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x1;
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x1;
}
#endif

using namespace vh;

int main(int argc, char *argv[])
{
  QSurfaceFormat format;
  format.setMajorVersion(4);
  format.setMinorVersion(5);
  format.setOption(QSurfaceFormat::DebugContext);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app(argc, argv);
  app.setApplicationName("ShaderToolQt");
  app.setApplicationVersion("");
  app.setOrganizationName("The ShaderTool Developers");
  app.setOrganizationDomain("shader.tool");
  app.setQuitOnLastWindowClosed(true);
  app.setAttribute(Qt::AA_UseDesktopOpenGL);
  app.setAttribute(Qt::AA_ShareOpenGLContexts);

  AppWindow mainWindow;
  if (argc > 1) {
    QString filename = QString::fromLocal8Bit(argv[1]);
    mainWindow.openNamedFile(filename);
  }

  mainWindow.show();

  // Important to call this *after* show, otherwise layout won't have been
  // performed and any calculations we do based on widgets sizes will be wrong.
  mainWindow.restoreWindowState();

  return app.exec();
}
