// Copyright 2019 Vilya Harve
#ifndef VH_APPWINDOW_H
#define VH_APPWINDOW_H

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

  public slots:
    void newFile();
    void openFile();
    void downloadFromShaderToy();
    void closeFile();
    void saveFile();
    void saveFileAs();

    void extractGLSL();
    void inlineGLSL();

    void deleteCache();

    void restoreWindowState();
    void resizeToRenderWidgetDisplayRect();

  protected:
    virtual void closeEvent(QCloseEvent* event);

  private:
    void createWidgets();
    void createMenus();

    void setupFileMenu(QMenu* menu);
    void setupEditMenu(QMenu* menu);
    void setupPlaybackMenu(QMenu* menu);
    void setupInputMenu(QMenu* menu);
    void setupViewMenu(QMenu* menu);
    void setupCacheMenu(QMenu* menu);
    void setupWindowMenu(QMenu* menu);

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

  private:
    QList<QString> loadRecentFileList();
    void saveRecentFileList(const QList<QString>& recentFiles);
    void addRecentFile(const QString& filename);

    struct Download {
      QString id;
      QString name;

      bool operator == (const Download& other) const { return id == other.id; }
      bool operator != (const Download& other) const { return id != other.id; }
    };

    QList<Download> loadRecentDownloadList();
    void saveRecentDownloadsList(const QList<Download>& recentDownloads);
    void addRecentDownload(const QString& id, const QString& displayName);

  private:
    QMenuBar* _menubar = nullptr;
    QMenu* _recentFilesMenu = nullptr;
    QMenu* _recentDownloadsMenu = nullptr;
    RenderWidget* _renderWidget = nullptr;

    QTreeWidget* _docTree = nullptr;
    QDockWidget* _docTreeDockable = nullptr;

    ShaderToyDocument* _document = nullptr;
    QFileSystemWatcher* _watcher = nullptr;

    ShaderToyDocument* _oldDocument = nullptr;

    FileCache* _cache = nullptr;

    bool _saveWindowState = true;
  };

} // namespace vh

#endif // VH_APPWINDOW_H
