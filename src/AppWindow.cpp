// Copyright 2019 Vilya Harvey
#include "AppWindow.h"

#include "RenderWidget.h"
#include "ShaderToy.h"

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>

namespace vh {

  //
  // AppWindow public methods
  //

  AppWindow::AppWindow(QWidget* parent) :
    QMainWindow(parent)
  {
    createWidgets();
    createMenus();
    _renderWidget->setFocus();
  }


  AppWindow::~AppWindow()
  {
    delete _document;
  }


  //
  // AppWindow public slots
  //

  void AppWindow::newFile()
  {
    closeFile();

    _document = defaultShaderToyDocument();

    _renderWidget->setShaderToyDocument(_document);
  }


  void AppWindow::openFile()
  {
    QString initialDir = ".";
    QString filename =  QFileDialog::getOpenFileName(this,
        "Open ShaderToy", initialDir, "ShaderToy JSON Files (*.json)");
    if (filename.isNull()) {
      return;
    }

    openNamedFile(filename);
  }


  void AppWindow::downloadFromShaderToy()
  {
    fetchShaderToyByID("3dXGWB"); // ID is for "Stop Motion Fox"
  }


  void AppWindow::closeFile()
  {
    if (_document == nullptr) {
      return;
    }

    delete _watcher;
    _watcher = nullptr;

    _oldDocument = _document;
    _document = nullptr;

    _renderWidget->setShaderToyDocument(nullptr);
  }


  void AppWindow::saveFile()
  {
    if (_document == nullptr) {
      return;
    }

    if (_document->src.isEmpty()) {
      saveFileAs();
      return;
    }

    bool watcherSignals = (_watcher != nullptr) ? _watcher->blockSignals(true) : false;

    try {
      saveShaderToyJSONFile(_document, _document->src);
    }
    catch (const std::runtime_error& err) {
      qCritical("Failed to save %s: %s", qPrintable(_document->src), err.what());
    }

    if (_watcher) {
      _watcher->blockSignals(watcherSignals);
    }
  }


  void AppWindow::saveFileAs()
  {
    if (_document == nullptr) {
      return;
    }

    QString initialDir = _document->refDir.absolutePath();
    QString filename = QFileDialog::getSaveFileName(this,
          "Save ShaderToy", initialDir, "ShaderToy JSON Files (*.json)");
    if (filename.isNull()) {
      return;
    }

    bool watcherSignals = (_watcher != nullptr) ? _watcher->blockSignals(true) : false;

    try {
      saveShaderToyJSONFile(_document, filename);
    }
    catch (const std::runtime_error& err) {
      qCritical("Failed to save %s: %s", qPrintable(filename), err.what());
      if (_watcher) {
        _watcher->blockSignals(watcherSignals);
      }
      return;
    }

    if (_watcher) {
      _watcher->blockSignals(watcherSignals);
    }

    _document->src = filename;
  }


  void AppWindow::extractGLSL()
  {
    if (_document == nullptr) {
      return;
    }

    try {
      extractGLSLToFiles(_document, false);
    }
    catch (const std::runtime_error& err) {
      qCritical("Failed to extract GLSL: %s", err.what());
      return;
    }

    reloadFile();
  }


  void AppWindow::inlineGLSL()
  {
    if (_document == nullptr) {
      return;
    }

    try {
      inlineGLSLFromFiles(_document);
    }
    catch (const std::runtime_error& err) {
      qCritical("Failed to extract GLSL: %s", err.what());
      return;
    }

    reloadFile();
  }


  void AppWindow::openNamedFile(const QString& filename)
  {
    // If there's any file currently open, close it.
    closeFile();

    try {
      _document = loadShaderToyJSONFile(filename);
    }
    catch (const std::runtime_error& err) {
      qCritical("JSON parsing error in %s: %s", qPrintable(filename), err.what());
      // Restore the old document.
      _document = _oldDocument;
      _oldDocument = nullptr;
      return;
    }

    _renderWidget->setShaderToyDocument(_document);
  }


  void AppWindow::fetchShaderToyByID(const QString &id)
  {
    if (_networkAccess == nullptr) {
      _networkAccess = new QNetworkAccessManager(this);
      connect(_networkAccess, &QNetworkAccessManager::finished, this, &AppWindow::fetchComplete);
    }

    QString urlStr = QString("https://www.shadertoy.com/api/v1/shaders/%1?key=fdHtWW").arg(id);
    _networkAccess->get(QNetworkRequest(QUrl(urlStr)));
  }


  //
  // AppWindow private methods
  //

  void AppWindow::createWidgets()
  {
    _renderWidget = new RenderWidget(this);
    setCentralWidget(_renderWidget);

    QObject::connect(_renderWidget, &RenderWidget::closeRequested, this, &QMainWindow::close);
    QObject::connect(_renderWidget, &RenderWidget::currentShaderToyDocumentChanged, this, &AppWindow::renderWidgetDocumentChanged);
  }


  void AppWindow::createMenus()
  {
    QMenuBar* menubar = new QMenuBar();
    setMenuBar(menubar);

    QMenu* fileMenu     = menubar->addMenu("&File");
    QMenu* playbackMenu = menubar->addMenu("&Playback");
    QMenu* inputMenu    = menubar->addMenu("&Input");
    QMenu* viewMenu     = menubar->addMenu("&View");

    setupFileMenu(fileMenu);
    setupPlaybackMenu(playbackMenu);
    setupInputMenu(inputMenu);
    setupViewMenu(viewMenu);
  }


  void AppWindow::setupFileMenu(QMenu* menu)
  {
    menu->addAction("&New",        this, &AppWindow::newFile,    QKeySequence(QKeySequence::New));
    menu->addAction("&Open...",    this, &AppWindow::openFile,   QKeySequence(QKeySequence::Open));
    menu->addAction("&Download from ShaderToy...", this, &AppWindow::downloadFromShaderToy);
    menu->addAction("&Close",      this, &AppWindow::closeFile,  QKeySequence(QKeySequence::Close));
    menu->addAction("&Save",       this, &AppWindow::saveFile,   QKeySequence(QKeySequence::Save));
    menu->addAction("&Save As...", this, &AppWindow::saveFileAs, QKeySequence(QKeySequence::SaveAs));
    menu->addSeparator();
    menu->addAction("&Extract GLSL", this, &AppWindow::extractGLSL);
    menu->addAction("&Inline GLSL",  this, &AppWindow::inlineGLSL);
    menu->addSeparator();
    menu->addAction("E&xit", this, &QMainWindow::close, QKeySequence(QKeySequence::Quit));
  }


  void AppWindow::setupPlaybackMenu(QMenu* menu)
  {
    RenderWidget* renderWidget = _renderWidget;

    menu->addAction("&Play/Pause",     [renderWidget](){ renderWidget->doAction(Action::eTogglePlayback); });
    menu->addAction("&Restart",        [renderWidget](){ renderWidget->doAction(Action::eRestartPlayback); });
    menu->addSeparator();
    menu->addAction("Forward 100 ms",  [renderWidget](){ renderWidget->doAction(Action::eFastForward_Small); });
    menu->addAction("Forward 1 sec",   [renderWidget](){ renderWidget->doAction(Action::eFastForward_Medium); });
    menu->addAction("Forward 10 secs", [renderWidget](){ renderWidget->doAction(Action::eFastForward_Large); });
    menu->addAction("Back 100 ms",     [renderWidget](){ renderWidget->doAction(Action::eRewind_Small); });
    menu->addAction("Back 1 sec",      [renderWidget](){ renderWidget->doAction(Action::eRewind_Medium); });
    menu->addAction("Back 10 secs",    [renderWidget](){ renderWidget->doAction(Action::eRewind_Large); });
  }


  void AppWindow::setupInputMenu(QMenu* menu)
  {
    QAction* keyboardAction = menu->addAction("&Keyboard");
    QAction* mouseAction = menu->addAction("&Mouse");

    // Setup the keyboard action.
    keyboardAction->setCheckable(true);
    keyboardAction->setChecked(_renderWidget->keyboardShaderInput());
    keyboardAction->setShortcut(QKeySequence("F2"));

    QObject::connect(keyboardAction, &QAction::toggled, _renderWidget, &RenderWidget::setKeyboardShaderInput);
    QObject::connect(_renderWidget, &RenderWidget::keyboardShaderInputChanged, keyboardAction, &QAction::setChecked);

    // Setup the mouse action.
    mouseAction->setCheckable(true);
    mouseAction->setChecked(_renderWidget->mouseShaderInput());
    mouseAction->setShortcut(QKeySequence("F3"));

    QObject::connect(mouseAction, &QAction::toggled, _renderWidget, &RenderWidget::setMouseShaderInput);
    QObject::connect(_renderWidget, &RenderWidget::mouseShaderInputChanged, mouseAction, &QAction::setChecked);
  }


  void AppWindow::setupViewMenu(QMenu* menu)
  {
    RenderWidget* renderWidget = _renderWidget;

    QMenu* viewRenderMenu = menu->addMenu("&Render");
    QMenu* viewZoomMenu   = menu->addMenu("&Zoom");
    menu->addSeparator();
    QMenu* viewPassMenu   = menu->addMenu("&Pass");
    menu->addSeparator();
    menu->addAction("&Overlay on/off",       [renderWidget](){ renderWidget->doAction(Action::eToggleOverlay); });
    menu->addAction("&Intermediates on/off", [renderWidget](){ renderWidget->doAction(Action::eToggleIntermediates); });

    setupViewRenderMenu(viewRenderMenu);
    setupViewZoomMenu(viewZoomMenu);
    setupViewPassMenu(viewPassMenu);
  }


  void AppWindow::setupViewRenderMenu(QMenu* menu)
  {
    QActionGroup* group = new QActionGroup(menu);

    RenderWidget* renderWidget = _renderWidget;

    QList<QAction*> actions;
    actions.push_back(menu->addAction("640x360 (Default ShaderToy Resolution)",  [renderWidget](){ renderWidget->setFixedRenderResolution(640, 360); }));
    actions.push_back(menu->addAction("1280x720", [renderWidget](){ renderWidget->setFixedRenderResolution(1280, 720); }));
    actions.push_back(menu->addAction("1920x1080", [renderWidget](){ renderWidget->setFixedRenderResolution(1920, 1080); }));
    menu->addSeparator();
    actions.push_back(menu->addAction("0.5x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(0.5f, 0.5f); }));
    actions.push_back(menu->addAction("1.0x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(1.0f, 1.0f); }));
    actions.push_back(menu->addAction("2.0x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(2.0f, 2.0f); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);
  }


  void AppWindow::setupViewZoomMenu(QMenu* menu)
  {
    QActionGroup* group = new QActionGroup(menu);

    RenderWidget* renderWidget = _renderWidget;

    QList<QAction*> actions;
    actions.push_back(menu->addAction("Fit &width",  [renderWidget](){ renderWidget->setDisplayOptions(true, false, 1.0f); }));
    actions.push_back(menu->addAction("Fit &height", [renderWidget](){ renderWidget->setDisplayOptions(false, true, 1.0f); }));
    menu->addSeparator();
    actions.push_back(menu->addAction("25%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.25f); }));
    actions.push_back(menu->addAction("50%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.5f); }));
    actions.push_back(menu->addAction("75%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.75f); }));
    actions.push_back(menu->addAction("100%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.0f); }, QKeySequence("=")));
    actions.push_back(menu->addAction("150%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.5f); }));
    actions.push_back(menu->addAction("200%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 2.0f); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);
  }


  void AppWindow::setupViewPassMenu(QMenu* menu)
  {
    QActionGroup* group = new QActionGroup(menu);

    RenderWidget* renderWidget = _renderWidget;

    QList<QAction*> actions;
    actions.push_back(menu->addAction("&Image", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eImage); }));
    menu->addSeparator();
    actions.push_back(menu->addAction("Buf &A", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 0); }));
    actions.push_back(menu->addAction("Buf &B", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 1); }));
    actions.push_back(menu->addAction("Buf &C", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 2); }));
    actions.push_back(menu->addAction("Buf &D", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eBuffer, 3); }));
    menu->addSeparator();
    actions.push_back(menu->addAction("C&ube A", [renderWidget](){ renderWidget->setDisplayPassByType(PassType::eCubemap); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);
  }


  //
  // AppWindow private slots
  //

  void AppWindow::reloadFile()
  {
    if (_document == nullptr) {
      return;
    }

    // Copy the current document src into a local string here, because
    // otherwise we'll end up with a dangling reference during the
    // `openNamedFile` call.
    QString filename = _document->src;
    openNamedFile(filename);
  }


  void AppWindow::renderWidgetDocumentChanged()
  {
    if (_oldDocument != nullptr) {
      qDebug("deleting old document %s", qPrintable(_oldDocument->info.name));
      delete _oldDocument;
      _oldDocument = nullptr;
    }

    if (_document != nullptr) {
      _watcher = new QFileSystemWatcher(this);
      watchAllFiles(_document, *_watcher);
      connect(_watcher, &QFileSystemWatcher::fileChanged, this, &AppWindow::watchedfileChanged);
    }
  }


  void AppWindow::fetchComplete(QNetworkReply* reply)
  {
    QByteArray data = reply->readAll();
    reply->deleteLater();

    // Save data to a local file, then open it
    QString localFilename = "C:/Users/vilya/Code/ShaderToolQt/downloaded.json";
    QFile file(localFilename);
    if (!file.open(QIODevice::WriteOnly)) {
      qCritical("Unable to save download to %s", localFilename);
    }
    file.write(data);
    file.close();

    openNamedFile(localFilename);
  }


  void AppWindow::watchedfileChanged(const QString& path)
  {
    qDebug("detected a change to file %s, reloading", qPrintable(path));
    reloadFile();
  }

} // namespace vh
