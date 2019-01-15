// Copyright 2019 Vilya Harvey
#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QSurfaceFormat>

//#include "Shader.h"
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


namespace vh {

  void openFile(RenderWidget* target)
  {
    QString initialDir = ".";
    QString filename =  QFileDialog::getOpenFileName(target,
        "Open ShaderToy", initialDir, "ShaderToy JSON Files (*.json)");
    if (filename.isNull()) {
      return;
    }

    ShaderToyDocument* doc = nullptr;
    try {
      doc = loadShaderToyJSONFile(filename);
    }
    catch (const std::runtime_error& err) {
      qCritical("JSON parsing error in %s: %s", qPrintable(filename), err.what());
      return;
    }

    target->setShaderToyDocument(doc);
  }


  QMenu* setupInputMenu(QMenu* menu, RenderWidget* renderWidget)
  {
    QAction* keyboardAction = menu->addAction("&Keyboard");
    QAction* mouseAction = menu->addAction("&Mouse");

    // Setup the keyboard action.
    keyboardAction->setCheckable(true);
    keyboardAction->setChecked(renderWidget->keyboardShaderInput());
    keyboardAction->setShortcut(QKeySequence("F2"));

    QObject::connect(keyboardAction, &QAction::toggled, renderWidget, &RenderWidget::setKeyboardShaderInput);
    QObject::connect(renderWidget, &RenderWidget::keyboardShaderInputChanged, keyboardAction, &QAction::setChecked);

    // Setup the mouse action.
    mouseAction->setCheckable(true);
    mouseAction->setChecked(renderWidget->mouseShaderInput());
    mouseAction->setShortcut(QKeySequence("F3"));

    QObject::connect(mouseAction, &QAction::toggled, renderWidget, &RenderWidget::setMouseShaderInput);
    QObject::connect(renderWidget, &RenderWidget::mouseShaderInputChanged, mouseAction, &QAction::setChecked);

    return menu;
  }


  QMenu* setupViewRenderMenu(QMenu* renderMenu, RenderWidget* renderWidget)
  {
    QActionGroup* group = new QActionGroup(renderMenu);

    QList<QAction*> actions;
    actions.push_back(renderMenu->addAction("640x360 (Default ShaderToy Resolution)",  [renderWidget](){ renderWidget->setFixedRenderResolution(640, 360); }));
    actions.push_back(renderMenu->addAction("1280x720", [renderWidget](){ renderWidget->setFixedRenderResolution(1280, 720); }));
    actions.push_back(renderMenu->addAction("1920x1080", [renderWidget](){ renderWidget->setFixedRenderResolution(1920, 1080); }));
    renderMenu->addSeparator();
    actions.push_back(renderMenu->addAction("0.5x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(0.5f, 0.5f); }));
    actions.push_back(renderMenu->addAction("1.0x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(1.0f, 1.0f); }));
    actions.push_back(renderMenu->addAction("2.0x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(2.0f, 2.0f); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);

    return renderMenu;
  }


  QMenu* setupViewZoomMenu(QMenu* displayMenu, RenderWidget* renderWidget)
  {
    QActionGroup* group = new QActionGroup(displayMenu);

    QList<QAction*> actions;
    actions.push_back(displayMenu->addAction("Fit &width",  [renderWidget](){ renderWidget->setDisplayOptions(true, false, 1.0f); }));
    actions.push_back(displayMenu->addAction("Fit &height", [renderWidget](){ renderWidget->setDisplayOptions(false, true, 1.0f); }));
    displayMenu->addSeparator();
    actions.push_back(displayMenu->addAction("25%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.25f); }));
    actions.push_back(displayMenu->addAction("50%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.5f); }));
    actions.push_back(displayMenu->addAction("75%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.75f); }));
    actions.push_back(displayMenu->addAction("100%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.0f); }, QKeySequence("=")));
    actions.push_back(displayMenu->addAction("150%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.5f); }));
    actions.push_back(displayMenu->addAction("200%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 2.0f); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);

    return displayMenu;
  }


  QMenu* setupViewPassMenu(QMenu* passMenu, RenderWidget* renderWidget)
  {
    QActionGroup* group = new QActionGroup(passMenu);

    QList<QAction*> actions;
    actions.push_back(passMenu->addAction("&Image", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eImage); }));
    passMenu->addSeparator();
    actions.push_back(passMenu->addAction("Buf &A", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 0); }));
    actions.push_back(passMenu->addAction("Buf &B", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 1); }));
    actions.push_back(passMenu->addAction("Buf &C", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 2); }));
    actions.push_back(passMenu->addAction("Buf &D", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 3); }));
    passMenu->addSeparator();
    actions.push_back(passMenu->addAction("C&ube A", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eCubemap); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);

    return passMenu;
  }

} // namespace vh


int main(int argc, char *argv[])
{
  using namespace vh;

  QSurfaceFormat format;
  format.setMajorVersion(4);
  format.setMinorVersion(5);
  format.setOption(QSurfaceFormat::DebugContext);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app(argc, argv);
  app.setApplicationName("ShaderToolQt");
  app.setApplicationVersion("");
  app.setQuitOnLastWindowClosed(true);
  app.setAttribute(Qt::AA_UseDesktopOpenGL);
  app.setAttribute(Qt::AA_ShareOpenGLContexts);

  ShaderToyDocument* doc = nullptr;
  if (argc > 1) {
    try {
      QString filename = QString::fromLocal8Bit(argv[1]);
      doc = loadShaderToyJSONFile(filename);
    }
    catch (const std::runtime_error& err) {
      qCritical("JSON parsing error in %s: %s", argv[1], err.what());
      doc = nullptr;
    }
  }

  QMainWindow mainWindow;
  RenderWidget* renderWidget = new RenderWidget(&mainWindow);
  renderWidget->setShaderToyDocument(doc);

  QMenuBar* menubar = new QMenuBar();
  mainWindow.setMenuBar(menubar);

  QMenu* fileMenu = menubar->addMenu("&File");
  fileMenu->addAction("&Open", [renderWidget](){ openFile(renderWidget); }, QKeySequence(QKeySequence::Open));
  fileMenu->addAction("&Close", [renderWidget](){ renderWidget->setShaderToyDocument(nullptr); }, QKeySequence(QKeySequence::Close));
  fileMenu->addSeparator();
  fileMenu->addAction("E&xit", &mainWindow, &QMainWindow::close, QKeySequence(QKeySequence::Quit));

  QMenu* playbackMenu = menubar->addMenu("&Playback");
  playbackMenu->addAction("&Play/Pause", [renderWidget](){ renderWidget->doAction(Action::eTogglePlayback); });
  playbackMenu->addAction("&Restart",    [renderWidget](){ renderWidget->doAction(Action::eRestartPlayback); });
  playbackMenu->addSeparator();
  playbackMenu->addAction("Forward 100 ms",  [renderWidget](){ renderWidget->doAction(Action::eFastForward_Small); });
  playbackMenu->addAction("Forward 1 sec",   [renderWidget](){ renderWidget->doAction(Action::eFastForward_Medium); });
  playbackMenu->addAction("Forward 10 secs", [renderWidget](){ renderWidget->doAction(Action::eFastForward_Large); });
  playbackMenu->addAction("Back 100 ms",     [renderWidget](){ renderWidget->doAction(Action::eRewind_Small); });
  playbackMenu->addAction("Back 1 sec",      [renderWidget](){ renderWidget->doAction(Action::eRewind_Medium); });
  playbackMenu->addAction("Back 10 secs",    [renderWidget](){ renderWidget->doAction(Action::eRewind_Large); });

  QMenu* inputMenu = menubar->addMenu("&Input");
  setupInputMenu(inputMenu, renderWidget);

  QMenu* viewMenu = menubar->addMenu("&View");
  QMenu* viewRenderMenu = viewMenu->addMenu("&Render");
  QMenu* viewZoomMenu = viewMenu->addMenu("&Zoom");
  viewMenu->addSeparator();
  QMenu* viewPassMenu = viewMenu->addMenu("&Pass");
  viewMenu->addSeparator();
  viewMenu->addAction("&Overlay on/off", [renderWidget](){ renderWidget->doAction(Action::eToggleOverlay); });

  setupViewRenderMenu(viewRenderMenu, renderWidget);
  setupViewZoomMenu(viewZoomMenu, renderWidget);
  setupViewPassMenu(viewPassMenu, renderWidget);

  QObject::connect(renderWidget, &RenderWidget::closeRequested, &mainWindow, &QMainWindow::close);

  mainWindow.setCentralWidget(renderWidget);
  mainWindow.resize(1280, 720);
  mainWindow.show();

  renderWidget->setFocus();
  renderWidget->startPlayback();

  return app.exec();
}
