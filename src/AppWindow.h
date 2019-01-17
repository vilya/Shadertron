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

    void openNamedFile(const QString& filename);

  private:
    void createWidgets();
    void createMenus();

    void setupFileMenu(QMenu* menu);
    void setupPlaybackMenu(QMenu* menu);
    void setupInputMenu(QMenu* menu);
    void setupViewMenu(QMenu* menu);
    void setupCacheMenu(QMenu* menu);

    void setupViewRenderMenu(QMenu* menu);
    void setupViewZoomMenu(QMenu* menu);
    void setupViewPassMenu(QMenu* menu);

  private slots:
    void reloadFile();
    void renderWidgetDocumentChanged();
    void watchedfileChanged(const QString& path);
    void standardAssetsReady();

  private:
    QMenuBar* _menubar = nullptr;
    RenderWidget* _renderWidget = nullptr;

    ShaderToyDocument* _document = nullptr;
    QFileSystemWatcher* _watcher = nullptr;

    ShaderToyDocument* _oldDocument = nullptr;

    FileCache* _cache = nullptr;
  };

} // namespace vh

#endif // VH_APPWINDOW_H
