// Copyright 2019 Vilya Harvey
#include "AppWindow.h"

#include "FileCache.h"
#include "RenderWidget.h"
#include "ShaderToy.h"
#include "ShaderToyDownloadForm.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

namespace vh {

  //
  // Constants
  //

  // Keys for use with a QSettings object.
  static const QString kDesktopWindowGeometry = "desktopWindowSize";
  static const QString kDesktopWindowState = "desktopWindowState";
  static const QString kDesktopWindowVersion = "desktopWindowVersion"; // Used to decide whether to restore saved window geometry & state or not.

  static const QString kRecentFileX         = "recentFile%1";         // Use this with .arg(x), where x is an int from 0 up to kMaxRecentFiles-1.
  static const QString kRecentDownloadIDX   = "recentDownloadID%1";   // Use this with .arg(x), where x is an int from 0 up to kMaxRecentDownloads-1.
  static const QString kRecentDownloadNameX = "recentDownloadName%1"; // Use this with .arg(x), where x is an int from 0 up to kMaxRecentDownloads-1.

  static constexpr int kMaxRecentFiles     = 10;
  static constexpr int kMaxRecentDownloads = 10;


  // Increment this when you make changes to the set of available windows and
  // dockable panels.
  //
  // We save this value alongside the window geometry and state in Settings.
  // When restoring the windows at startup time, we check this value first
  // to see whether the saved window data is compatible with this verison of
  // the program.
  static const int kDesktopUIVersion = 1;


  //
  // AppWindow public methods
  //

  AppWindow::AppWindow(QWidget* parent) :
    QMainWindow(parent)
  {
    _cache = new FileCache(this);
    connect(_cache, &FileCache::shaderReady, this, &AppWindow::openDownloadedFile);
    connect(_cache, &FileCache::standardAssetsReady, this, &AppWindow::standardAssetsReady);

    createWidgets();
    createMenus();

    _renderWidget->setFocus();
  }


  AppWindow::~AppWindow()
  {
    delete _document;
  }


  bool AppWindow::openNamedFile(const QString& filename)
  {
    // If there's any file currently open, close it.
    closeFile();

    try {
      _document = loadShaderToyJSONFile(filename);
    }
    catch (const std::runtime_error& err) {
      qCritical("Unable to load %s: %s", qPrintable(filename), err.what());
      QMessageBox::critical(this, "Load failed", QString("Unable to load %1: %2").arg(filename).arg(err.what()));
      // Restore the old document.
      _document = _oldDocument;
      _oldDocument = nullptr;
      return false;
    }

    _renderWidget->setShaderToyDocument(_document);
    return true;
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

    if (!openNamedFile(filename)) {
      return;
    }

    addRecentFile(filename);
  }


  void AppWindow::downloadFromShaderToy()
  {
    ShaderToyDownloadForm* downloadForm = new ShaderToyDownloadForm();
    int option = downloadForm->exec();
    if (option == QDialog::Rejected || downloadForm->selectedShaderID().isEmpty()) {
      delete downloadForm;
      return;
    }

    QString shaderID = downloadForm->selectedShaderID();
    bool forceDownload = downloadForm->forceDownload();
    delete downloadForm;

    bool valid = shaderID.length() == 6;
    if (valid) {
      for (int i = 0; i < shaderID.length(); i++) {
        if (!shaderID.at(i).isLetterOrNumber()) {
          valid = false;
          break;
        }
      }
    }

    if (!valid) {
      QMessageBox::critical(this, "Invalid shader ID",
          "The shader ID you have entered is not valid");
      return;
    }

    _cache->fetchShaderToyByID(shaderID, forceDownload);
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

    addRecentFile(_document->src);
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

    addRecentFile(filename);

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


  void AppWindow::deleteCache()
  {
    if (_cache == nullptr) {
      return;
    }

    int answer = QMessageBox::question(this, "Delete cache?", "Delete all files in the cache directory?");
    if (answer != QMessageBox::Yes) {
      return;
    }

    _cache->deleteCache();
  }


  //
  // AppWindow protected methods
  //

  void AppWindow::closeEvent(QCloseEvent* /*event*/)
  {
    // TODO: Check for changes that need saving before we exit.
    saveWindowState();
  }


  //
  // AppWindow private methods
  //

  void AppWindow::createWidgets()
  {
    _renderWidget = new RenderWidget(this);
    _renderWidget->setFileCache(_cache);
    setCentralWidget(_renderWidget);

    QObject::connect(_renderWidget, &RenderWidget::closeRequested, this, &QMainWindow::close);
    QObject::connect(_renderWidget, &RenderWidget::currentShaderToyDocumentChanged, this, &AppWindow::renderWidgetDocumentChanged);
  }


  void AppWindow::createMenus()
  {
    QMenuBar* menubar = new QMenuBar();
    setMenuBar(menubar);

    QMenu* fileMenu     = menubar->addMenu("&File");
    QMenu* editMenu     = menubar->addMenu("&Edit");
    QMenu* playbackMenu = menubar->addMenu("&Playback");
    QMenu* inputMenu    = menubar->addMenu("&Input");
    QMenu* viewMenu     = menubar->addMenu("&View");
    QMenu* cacheMenu    = menubar->addMenu("&Cache");
    QMenu* windowMenu   = menubar->addMenu("&Window");

    setupFileMenu(fileMenu);
    setupEditMenu(editMenu);
    setupPlaybackMenu(playbackMenu);
    setupInputMenu(inputMenu);
    setupViewMenu(viewMenu);
    setupCacheMenu(cacheMenu);
    setupWindowMenu(windowMenu);
  }


  void AppWindow::setupFileMenu(QMenu* menu)
  {
    menu->addAction("&New",        this, &AppWindow::newFile,    QKeySequence(QKeySequence::New));
    menu->addAction("&Open...",    this, &AppWindow::openFile,   QKeySequence(QKeySequence::Open));
    menu->addAction("&Download from ShaderToy...", this, &AppWindow::downloadFromShaderToy);
    menu->addAction("&Close",      this, &AppWindow::closeFile,  QKeySequence(QKeySequence::Close));
    menu->addAction("&Save",       this, &AppWindow::saveFile,   QKeySequence(QKeySequence::Save));
    menu->addAction("Save &as...", this, &AppWindow::saveFileAs, QKeySequence(QKeySequence::SaveAs));
    menu->addAction("&Revert",     this, &AppWindow::reloadFile);
    menu->addSeparator();
    _recentFilesMenu     = menu->addMenu("Recent &files");
    _recentDownloadsMenu = menu->addMenu("Recent do&wnloads");
    menu->addSeparator();
    menu->addAction("E&xit", this, &QMainWindow::close, QKeySequence(QKeySequence::Quit));

    setupRecentFilesMenu(_recentFilesMenu);
    setupRecentDownloadsMenu(_recentDownloadsMenu);
  }


  void AppWindow::setupEditMenu(QMenu* menu)
  {
    menu->addAction("&Extract GLSL", this, &AppWindow::extractGLSL);
    menu->addAction("&Inline GLSL",  this, &AppWindow::inlineGLSL);
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

    // Setup the keyboard action.
    keyboardAction->setCheckable(true);
    keyboardAction->setChecked(_renderWidget->keyboardShaderInput());
    keyboardAction->setShortcut(QKeySequence("F2"));

    QObject::connect(keyboardAction, &QAction::toggled, _renderWidget, &RenderWidget::setKeyboardShaderInput);
    QObject::connect(_renderWidget, &RenderWidget::keyboardShaderInputChanged, keyboardAction, &QAction::setChecked);
  }


  void AppWindow::setupViewMenu(QMenu* menu)
  {
    RenderWidget* renderWidget = _renderWidget;

    QMenu* viewRenderMenu = menu->addMenu("&Render");
    QMenu* viewZoomMenu   = menu->addMenu("&Zoom");
    menu->addSeparator();
    QMenu* viewPassMenu   = menu->addMenu("&Pass");
    QAction* toggleInputsAction  = menu->addAction("Show &Inputs",  renderWidget, &RenderWidget::toggleInputs);
    QAction* toggleOutputsAction = menu->addAction("Show &Outputs", renderWidget, &RenderWidget::toggleOutputs);
    menu->addSeparator();
    QMenu* viewHUDContentsMenu = menu->addMenu("HUD &Contents");
    QAction* toggleHUDAction     = menu->addAction("Show &HUD",     renderWidget, &RenderWidget::toggleHUD);
    menu->addSeparator();
    menu->addAction("&Center", renderWidget, &RenderWidget::recenterImage);

    toggleHUDAction->setCheckable(true);
    toggleHUDAction->setChecked(true);

    toggleInputsAction->setCheckable(true);
    toggleInputsAction->setChecked(false);

    toggleOutputsAction->setCheckable(true);
    toggleOutputsAction->setChecked(false);

    setupViewRenderMenu(viewRenderMenu);
    setupViewZoomMenu(viewZoomMenu);
    setupViewPassMenu(viewPassMenu);
    setupViewHUDContentsMenu(viewHUDContentsMenu);
  }


  void AppWindow::setupCacheMenu(QMenu* menu)
  {
    FileCache* cache = _cache;

    menu->addAction("Download ShaderToy standard assets", cache, &FileCache::fetchShaderToyStandardAssets);
    menu->addAction("Open cache directory...", [cache](){
      QUrl url;
      url.setScheme("file");
      url.setPath(cache->cacheDir().absolutePath());
      QDesktopServices::openUrl(url);
    });
    menu->addSeparator();
    menu->addAction("Clear cache...", this, &AppWindow::deleteCache);
  }


  void AppWindow::setupWindowMenu(QMenu* menu)
  {
    QAction* saveWindowStateAction = menu->addAction("&Save window state on exit");
    QAction* removeWindowStateAction = menu->addAction("&Remove saved window state", this, &AppWindow::removeSavedWindowState);

    saveWindowStateAction->setCheckable(true);
    saveWindowStateAction->setChecked(_saveWindowState);

    bool* state = &_saveWindowState;
    connect(saveWindowStateAction, &QAction::toggled, [state](bool value) { *state = value; });
    connect(removeWindowStateAction, &QAction::triggered, [saveWindowStateAction](){ saveWindowStateAction->setChecked(false); });
  }


  void AppWindow::setupRecentFilesMenu(QMenu* menu)
  {
    menu->clear();

    QList<QString> recentFiles = loadRecentFileList();
    if (recentFiles.empty()) {
      menu->setEnabled(false);
      return;
    }

    menu->setEnabled(true);
    for (int i = 0; i < recentFiles.size(); i++) {
      menu->addAction(QString("&%1 %2").arg(i).arg(recentFiles[i]), [this, i](){ this->loadRecentFile(i); });
    }
  }


  void AppWindow::setupRecentDownloadsMenu(QMenu* menu)
  {
    menu->clear();

    QList<Download> recentDownloads = loadRecentDownloadList();
    if (recentDownloads.empty()) {
      menu->setEnabled(false);
      return;
    }

    menu->setEnabled(true);
    for (int i = 0; i < recentDownloads.size(); i++) {
      menu->addAction(QString("&%1 %2 (id: %3)").arg(i).arg(recentDownloads[i].name).arg(recentDownloads[i].id),
                      [this, i](){ this->loadRecentDownload(i); });
    }
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
    actions.push_back(menu->addAction("0.5x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(0.5f); }));
    actions.push_back(menu->addAction("1.0x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(1.0f); }));
    actions.push_back(menu->addAction("2.0x window", [renderWidget](){ renderWidget->setRelativeRenderResolution(2.0f); }));

    for (QAction* action : actions) {
      group->addAction(action);
      action->setCheckable(true);
    }
    actions.front()->setChecked(true);
  }


  void AppWindow::setupViewZoomMenu(QMenu* menu)
  {
    RenderWidget* renderWidget = _renderWidget;

    menu->addAction("Fit &width",  [renderWidget](){ renderWidget->setDisplayOptions(true, false, 1.0f); });
    menu->addAction("Fit &height", [renderWidget](){ renderWidget->setDisplayOptions(false, true, 1.0f); });
    menu->addSeparator();
    menu->addAction("25%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.25f); });
    menu->addAction("50%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.5f); });
    menu->addAction("67%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.67f); });
    menu->addAction("75%",  [renderWidget](){ renderWidget->setDisplayOptions(false, false, 0.75f); });
    menu->addAction("100%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.0f); }, QKeySequence("="));
    menu->addAction("125%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.25f); });
    menu->addAction("150%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 1.5f); });
    menu->addAction("200%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 2.0f); });
    menu->addAction("300%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 3.0f); });
    menu->addAction("400%", [renderWidget](){ renderWidget->setDisplayOptions(false, false, 4.0f); });
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


  void AppWindow::setupViewHUDContentsMenu(QMenu* menu)
  {
    RenderWidget* renderWidget = _renderWidget;
    uint hudFlags = renderWidget->hudFlags();

    QList<QAction*> actions;
    actions.push_back(menu->addAction("Frame &number",           [renderWidget](){ renderWidget->toggleHUDFlag(kHUD_FrameNum); }));
    actions.push_back(menu->addAction("&Time",                   [renderWidget](){ renderWidget->toggleHUDFlag(kHUD_Time); }));
    actions.push_back(menu->addAction("&Milliseconds per frame", [renderWidget](){ renderWidget->toggleHUDFlag(kHUD_MillisPerFrame); }));
    actions.push_back(menu->addAction("&Frames per second",      [renderWidget](){ renderWidget->toggleHUDFlag(kHUD_FramesPerSec); }));
    actions.push_back(menu->addAction("&Mouse position",         [renderWidget](){ renderWidget->toggleHUDFlag(kHUD_MousePos); }));
    actions.push_back(menu->addAction("&Mouse down position",    [renderWidget](){ renderWidget->toggleHUDFlag(kHUD_MouseDownPos); }));

    for (int i = 0; i < actions.size(); i++) {
      actions[i]->setCheckable(true);
      actions[i]->setChecked((hudFlags & (1u << i)) != 0);
    }
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


  void AppWindow::watchedfileChanged(const QString& path)
  {
    qDebug("detected a change to file %s, reloading", qPrintable(path));
    reloadFile();
  }


  void AppWindow::standardAssetsReady()
  {
    QMessageBox::information(this, "Download complete", "Finished downloading the standard ShaderToy assets");
  }


  void AppWindow::saveWindowState()
  {
    if (!_saveWindowState) {
      return;
    }

    qDebug("Saving window state");
    QSettings settings;
    settings.setValue(kDesktopWindowGeometry, saveGeometry());
    settings.setValue(kDesktopWindowState, saveState());
    settings.setValue(kDesktopWindowVersion, kDesktopUIVersion);
  }


  void AppWindow::restoreWindowState()
  {
    qDebug("Restoring window state");

    QSettings settings;
    int savedWindowStateVersion = settings.value(kDesktopWindowVersion, -1).toInt();
    bool shouldRestore = (savedWindowStateVersion == kDesktopUIVersion);
    bool restored = false;
    if (!shouldRestore) {
      settings.remove(kDesktopWindowGeometry);
      settings.remove(kDesktopWindowState);
      settings.remove(kDesktopWindowVersion);
    }
    else {
      restored = restoreGeometry(settings.value(kDesktopWindowGeometry).toByteArray());
    }

    if (!restored) {
      qDebug("Window state was not restored, calculating default window rectangle");

      // If this is the first run, or the user has deleted their settings,
      // there may not be any geometry to restore. In this case, fall back
      // to making the window an exact multiple of 640x360 (the default render
      // resolution) and centering it on the screen.
      QScreen* screen = QApplication::primaryScreen();
      if (screen != nullptr) {
        QRect rect = screen->availableGeometry();
        int menuBarHeight = menuBar()->height();
        int scale = qMin(rect.width() / 640, (rect.height() - menuBarHeight) / 360);
        int w = 640 * scale;
        int h = 360 * scale + menuBarHeight;
        int x = rect.x() + (rect.width() - w) / 2;
        int y = rect.y() + (rect.height() - h) / 2;
        setGeometry(x, y, w, h);
      }
    }
    restoreState(settings.value(kDesktopWindowState).toByteArray());
  }


  void AppWindow::removeSavedWindowState()
  {
    QSettings settings;
    settings.remove(kDesktopWindowGeometry);
    settings.remove(kDesktopWindowState);
    settings.remove(kDesktopWindowVersion);
  }


  void AppWindow::openDownloadedFile(const QString& filename)
  {
    if (openNamedFile(filename)) {
      QString id = _document->info.id;
      QString displayName = _document->info.name;
      addRecentDownload(id, displayName);
    }
  }


  void AppWindow::loadRecentFile(int idx)
  {
    QList<QString> recentFiles = loadRecentFileList();
    if (idx < 0 || idx > kMaxRecentFiles || idx > recentFiles.size()) {
      return;
    }

    QString filename = recentFiles[idx];
    if (openNamedFile(filename)) {
      addRecentFile(filename);
    }
  }


  void AppWindow::loadRecentDownload(int idx)
  {
    QList<Download> recentDownloads = loadRecentDownloadList();
    if (idx < 0 || idx > kMaxRecentDownloads || idx > recentDownloads.size()) {
      return;
    }

    QString id = recentDownloads[idx].id;
    _cache->fetchShaderToyByID(id, false);
  }


  //
  // AppWindow private methods
  //

  QList<QString> AppWindow::loadRecentFileList()
  {
    QList<QString> recentFiles;

    QSettings settings;
    for (int i = 0; i < kMaxRecentFiles; i++) {
      QString key = kRecentFileX.arg(i);
      QString filename = settings.value(key).toString();
      if (filename.isNull() || filename.isEmpty()) {
        break;
      }
      recentFiles.push_back(filename);
    }

    return recentFiles;
  }


  void AppWindow::saveRecentFileList(const QList<QString>& recentFiles)
  {
    QSettings settings;
    for (int i = 0, end = qMin(recentFiles.size(), kMaxRecentFiles); i < end; i++) {
      QString key = kRecentFileX.arg(i);
      settings.setValue(key, recentFiles[i]);
    }
  }


  void AppWindow::addRecentFile(const QString& filename)
  {
    QList<QString> recentFiles = loadRecentFileList();

    if (!recentFiles.isEmpty() && recentFiles.front() == filename) {
      return;
    }

    // Make sure that the new filename is the first entry in the list and only appears once.
    recentFiles.removeAll(filename);
    recentFiles.push_front(filename);
    while (recentFiles.size() > kMaxRecentFiles) {
      recentFiles.pop_back();
    }

    saveRecentFileList(recentFiles);
    setupRecentFilesMenu(_recentFilesMenu);
  }


  QList<AppWindow::Download> AppWindow::loadRecentDownloadList()
  {
    QList<Download> recentDownloads;

    QSettings settings;
    for (int i = 0; i < kMaxRecentDownloads; i++) {
      QString idKey   = kRecentDownloadIDX.arg(i);
      QString nameKey = kRecentDownloadNameX.arg(i);
      Download download;
      download.id   = settings.value(idKey).toString();
      download.name = settings.value(nameKey).toString();
      if (download.id.isNull() || download.id.isEmpty() || download.name.isNull() || download.name.isEmpty()) {
        break;
      }
      recentDownloads.push_back(download);
    }

    return recentDownloads;
  }


  void AppWindow::saveRecentDownloadsList(const QList<Download>& recentDownloads)
  {
    QSettings settings;
    for (int i = 0, end = qMin(recentDownloads.size(), kMaxRecentDownloads); i < end; i++) {
      QString idKey   = kRecentDownloadIDX.arg(i);
      QString nameKey = kRecentDownloadNameX.arg(i);
      settings.setValue(idKey,   recentDownloads[i].id);
      settings.setValue(nameKey, recentDownloads[i].name);
    }
  }


  void AppWindow::addRecentDownload(const QString& id, const QString& displayName)
  {
    QList<Download> recentDownloads = loadRecentDownloadList();

    Download download = { id, displayName };

    if (!recentDownloads.isEmpty() && recentDownloads.front() == download) {
      return;
    }

    recentDownloads.removeAll(download);
    recentDownloads.push_front(download);
    while (recentDownloads.size() > kMaxRecentDownloads) {
      recentDownloads.pop_back();
    }

    saveRecentDownloadsList(recentDownloads);
    setupRecentDownloadsMenu(_recentDownloadsMenu);
  }

} // namespace vh
