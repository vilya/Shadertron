// Copyright 2019 Vilya Harve
#ifndef VH_APPWINDOW_H
#define VH_APPWINDOW_H

#include <QActionGroup>
#include <QDir>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSet>
#include <QString>

#include <QTreeWidget>
#include <QDockWidget>

namespace vh {

  //
  // Forward declarations
  //

  class FileCache;
  class LogWidget;
  class RenderWidget;

  struct ShaderToyDocument;


  //
  // AppWindow class
  //

  class AppWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    AppWindow(QWidget* parent=nullptr);
    virtual ~AppWindow();

    bool openNamedFile(const QString& filename);

    void handleLogMessage(QtMsgType type, const QMessageLogContext& context, const QString& message);

  public slots:
    void newFile();
    void openFile();
    void downloadFromShaderToy();
    void closeFile();
    void saveFile();
    void saveFileAs();

    void extractGLSL();
    void inlineGLSL();

    void toggleFullscreen();

    void deleteCache();

    void restoreWindowState();
    void resizeToRenderWidgetDisplayRect();

    void showAboutDialog();
    void showPreferencesDialog();

  protected:
    virtual void closeEvent(QCloseEvent* event);

  private:
    void createWidgets();
    void createMenus();

    void setupFileMenu(QMenu* menu);
    void setupEditMenu(QMenu* menu);
    void setupPlaybackMenu(QMenu* menu);
    void setupRecordingMenu(QMenu* menu);
    void setupInputMenu(QMenu* menu);
    void setupViewMenu(QMenu* menu);
    void setupCacheMenu(QMenu* menu);
    void setupWindowMenu(QMenu* menu);
    void setupHelpMenu(QMenu* menu);

    void setupRecentFilesMenu(QMenu* menu);
    void setupRecentDownloadsMenu(QMenu* menu);

    void setupViewRenderMenu(QMenu* menu);
    void setupViewZoomMenu(QMenu* menu);
    void setupViewPassMenu(QMenu* menu);
    void setupViewHUDContentsMenu(QMenu* menu);

    void populateDocTree();

  private slots:
    void reloadFile();
    void renderWidgetDocumentChanged();
    void watchedfileChanged(const QString& path);
    void standardAssetsReady();

    void saveWindowState();
    void removeSavedWindowState();

    void openDownloadedFile(const QString& filename);

    void loadRecentFile(int idx);
    void loadRecentDownload(int idx);

    void saveScreenshot(const QImage& img);

  private:
    void addRecentFile(const QString& filename);
    void addRecentDownload(const QString& id, const QString& displayName);

  private:
    QMenuBar* _menubar = nullptr;
    QMenu* _recentFilesMenu = nullptr;
    QMenu* _recentDownloadsMenu = nullptr;
    QMenu* _viewPassMenu = nullptr;
    QActionGroup* _viewPassGroup = nullptr;

    QAction* _viewImageAction = nullptr;
    QAction* _viewBufAAction = nullptr;
    QAction* _viewBufBAction = nullptr;
    QAction* _viewBufCAction = nullptr;
    QAction* _viewBufDAction = nullptr;
    QAction* _viewCubeAAction = nullptr;
    QAction* _viewSoundAction = nullptr;

    RenderWidget* _renderWidget = nullptr;

    QTreeWidget* _docTree = nullptr;
    QDockWidget* _docTreeDockable = nullptr;

    LogWidget* _logWidget = nullptr;
    QDockWidget* _logWidgetDockable = nullptr;

    ShaderToyDocument* _document = nullptr;
    QFileSystemWatcher* _watcher = nullptr;

    ShaderToyDocument* _oldDocument = nullptr;

    FileCache* _cache = nullptr;

    bool _saveWindowState = true;
  };


  extern AppWindow* gAppWindow;

} // namespace vh

#endif // VH_APPWINDOW_H
