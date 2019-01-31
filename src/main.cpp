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


static QtMessageHandler gOldHandler = nullptr;


void appWindowMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
  if (gAppWindow != nullptr) {
    gAppWindow->handleLogMessage(type, context, message);
  }
  if (gOldHandler != nullptr) {
    gOldHandler(type, context, message);
  }
}


int main(int argc, char *argv[])
{
  QSurfaceFormat format;
  format.setMajorVersion(4);
#ifdef SHADERTOOL_USE_GL41
  format.setMinorVersion(1);
#else
  format.setMinorVersion(5);
  format.setOption(QSurfaceFormat::DebugContext);
#endif // SHADERTOOL_USE_GL41
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  gOldHandler = qInstallMessageHandler(appWindowMessageHandler);

  QApplication app(argc, argv);
  app.setApplicationName("Shadertron");
  app.setApplicationVersion("0.1");
  app.setOrganizationName("The Shadertron Developers");
  app.setOrganizationDomain("shader.tool");
  app.setQuitOnLastWindowClosed(true);
  app.setAttribute(Qt::AA_UseDesktopOpenGL);
  app.setAttribute(Qt::AA_ShareOpenGLContexts);

  AppWindow mainWindow;
  gAppWindow = &mainWindow;
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
