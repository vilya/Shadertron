// Copyright 2019 Vilya Harve
#ifndef VH_APPWINDOW_H
#define VH_APPWINDOW_H

#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QNetworkAccessManager>

namespace vh {

  //
  // Forward declarations
  //

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

    void openNamedFile(const QString& filename);

    void fetchShaderToyByID(const QString& id);

  private:
    void createWidgets();
    void createMenus();

    void setupFileMenu(QMenu* menu);
    void setupPlaybackMenu(QMenu* menu);
    void setupInputMenu(QMenu* menu);
    void setupViewMenu(QMenu* menu);

    void setupViewRenderMenu(QMenu* menu);
    void setupViewZoomMenu(QMenu* menu);
    void setupViewPassMenu(QMenu* menu);

  private slots:
    void reloadFile();
    void renderWidgetDocumentChanged();
    void fetchComplete(QNetworkReply* reply);
    void watchedfileChanged(const QString& path);

  private:
    QMenuBar* _menubar = nullptr;
    RenderWidget* _renderWidget = nullptr;

    ShaderToyDocument* _document = nullptr;
    QFileSystemWatcher* _watcher = nullptr;

    ShaderToyDocument* _oldDocument = nullptr;

    QNetworkAccessManager* _networkAccess = nullptr;
    // TODO: add a cache dir, use it to store downloaded shaders, assets, thumbnails, etc.
  };

} // namespace vh

#endif // VH_APPWINDOW_H
