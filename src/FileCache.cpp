// Copyright 2019 Vilya Harvey
#include "FileCache.h"

#include "ShaderToy.h"

#include <QMessageBox>
#include <QMessageLogger>
#include <QStandardPaths>

namespace vh {

  //
  // FileCache public methods
  //

  FileCache::FileCache(QObject* parent) :
    QObject(parent)
  {
    _cacheDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    qDebug("cache dir is %s", qPrintable(_cacheDir.absolutePath()));
  }


  FileCache::~FileCache()
  {
  }


  //
  // FileCache public methods
  //

  bool FileCache::saveFileToCache(const QString& path, const QByteArray& data, QWidget* parentForErrorDialogs)
  {
    QString cachePath = pathForCachedFile(path);

    // Don't allow paths that would cause us to write outside the cache dir.
    if (!cachePath.startsWith(_cacheDir.canonicalPath())) {
      qDebug("Cannot save %s to the cache because it would write to a location outside the cache directory", qPrintable(path));
      qDebug("    Cache dir:   %s", qPrintable(_cacheDir.canonicalPath()));
      qDebug("    Target file: %s", qPrintable(cachePath));
      if (parentForErrorDialogs != nullptr) {
        QMessageBox::critical(parentForErrorDialogs, "Save failed", QString("Cannot save %s to the cache because it would write to a location outside the cache directory").arg(path));
      }
      return false;
    }

    // If the file is inside a subdirectory of the cache dir and that
    // subdirectory doesn't exist yet, create it.
    QDir dir = QFileInfo(cachePath).absoluteDir();
    if (!dir.exists()) {
      QString relDir = _cacheDir.relativeFilePath(dir.absolutePath());
      if (!_cacheDir.mkpath(relDir)) {
        qDebug("Failed to create directory %s in the cache", qPrintable(relDir));
        if (parentForErrorDialogs) {
          QMessageBox::critical(parentForErrorDialogs, "Save failed", QString("Failed to create directory %1 in the cache").arg(relDir));
        }
        return false;
      }
    }

    // Save the file.
    QFile file(cachePath);
    if (!file.open(QIODevice::WriteOnly)) {
      qCritical("Unable to save downloaded file to %s", qPrintable(cachePath));
      if (parentForErrorDialogs) {
        QMessageBox::critical(parentForErrorDialogs, "Save failed", QString("Unable to save downloaded file to %1").arg(cachePath));
      }
      return false;
    }

    file.write(data);
    file.close();
    return true;
  }


  QString FileCache::pathForCachedFile(const QString& path)
  {
    if (path.startsWith("/")) {
      return _cacheDir.absoluteFilePath(path.mid(1));
    }
    else {
      return _cacheDir.absoluteFilePath(path);
    }
  }


  bool FileCache::isResource(const QString& path)
  {
    return path.startsWith("/media") && QFileInfo(QString(":%1").arg(path)).exists();
  }


  bool FileCache::isCached(const QString& path)
  {
    QFileInfo info(pathForCachedFile(path));
    return info.exists();
  }


  //
  // FileCache public slots
  //

  void FileCache::fetchShaderToyByID(const QString &id)
  {
    if (_networkAccess == nullptr) {
      _networkAccess = new QNetworkAccessManager(this);
    }

    QString urlStr = QString("https://www.shadertoy.com/api/v1/shaders/%1?key=%2").arg(id).arg(kShaderToyAppKey);
    QNetworkReply* reply = _networkAccess->get(QNetworkRequest(QUrl(urlStr)));
    connect(reply, &QNetworkReply::finished, this, &FileCache::shaderDownloaded);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &FileCache::shaderDownloadFailed);
  }


  //
  // FileCache private slots
  //

  void FileCache::shaderDownloaded()
  {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString path = reply->url().path();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QWidget* parentForErrorDialogs = qobject_cast<QWidget*>(parent());

    // Save data to a local file, then open it
    if (!saveFileToCache(path, data, parentForErrorDialogs)) {
      return;
    }

    _downloadedShaderFile = pathForCachedFile(path);

    // Download successful, now let's parse the document so we can also grab
    // any required assets for it.
    ShaderToyDocument* document = nullptr;
    try {
      document = loadShaderToyJSONFile(_downloadedShaderFile);
    }
    catch (const std::runtime_error& err) {
      qCritical("Unable to load %s: %s", qPrintable(_downloadedShaderFile), err.what());
      QMessageBox::critical(parentForErrorDialogs, "Load failed", QString("Unable to load %1: %2").arg(_downloadedShaderFile).arg(err.what()));
      return;
    }

    QSet<QString> requiredAssets;
    for (int passIdx = 0; passIdx < document->renderpasses.size(); passIdx++) {
      ShaderToyRenderPass& pass = document->renderpasses[passIdx];
      for (int i = 0; i < pass.inputs.size(); i++) {
        if (pass.inputs[i].ctype == kInputType_Texture) {
          requiredAssets.insert(pass.inputs[i].src);
        }
        else if (pass.inputs[i].ctype == kInputType_CubeMap) {
          QFileInfo fileInfo(pass.inputs[i].src);
          QString path = fileInfo.path();
          QString basename = fileInfo.completeBaseName();
          QString suffix = fileInfo.suffix();
          requiredAssets.insert(QString("%1/%2.%3"  ).arg(path).arg(basename).arg(suffix));
          requiredAssets.insert(QString("%1/%2_1.%3").arg(path).arg(basename).arg(suffix));
          requiredAssets.insert(QString("%1/%2_2.%3").arg(path).arg(basename).arg(suffix));
          requiredAssets.insert(QString("%1/%2_3.%3").arg(path).arg(basename).arg(suffix));
          requiredAssets.insert(QString("%1/%2_4.%3").arg(path).arg(basename).arg(suffix));
          requiredAssets.insert(QString("%1/%2_5.%3").arg(path).arg(basename).arg(suffix));
        }
      }
    }

    delete document;

    _assetsToDownloadLock.lock();
    _assetsToDownload.clear();
    for (QString src : requiredAssets) {
      // All ShaderToy assets are stored under their media folder.
      if (!src.startsWith("/media/")) {
        continue;
      }

      // Check whether we have a built-in copy of the asset.
      if (isResource(src)) {
        qDebug("Shader uses %s which we already have as a built-in resource", qPrintable(src));
        continue;
      }

      // If we don't have it as a resource, maybe we've downloaded it already?
      if (isCached(src)) {
        qDebug("Shader uses %s which we already have in our cache", qPrintable(src));
        continue;
      }

      // If this asset has already been marked for downloading, no need to do anything more.
      if (_assetsToDownload.contains(src)) {
        continue;
      }

      qDebug("Need to download %s", qPrintable(src));
      _assetsToDownload.insert(src);

      QString urlStr = QString("https://www.shadertoy.com/%1?key=%2").arg(src.mid(1)).arg(kShaderToyAppKey);
      QNetworkReply* reply = _networkAccess->get(QNetworkRequest(QUrl(urlStr)));
      connect(reply, &QNetworkReply::finished, this, &FileCache::assetDownloaded);
      connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &FileCache::assetDownloadFailed);
    }
    bool ready = _assetsToDownload.empty();
    _assetsToDownloadLock.unlock();
    if (ready) {
      emit shaderReady(_downloadedShaderFile);
      _downloadedShaderFile = QString();
    }
  }


  void FileCache::shaderDownloadFailed(QNetworkReply::NetworkError /*err*/)
  {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    QWidget* parentForErrorDialogs = qobject_cast<QWidget*>(parent());

    qCritical("Failed to download shader from ShaderToy (url was %s)", qPrintable(reply->url().toDisplayString()));
    QMessageBox::critical(parentForErrorDialogs, "Download failed", "Failed to download shader from ShaderToy");
  }


  void FileCache::assetDownloaded()
  {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString path = reply->url().path();

    bool ready = false;
    _assetsToDownloadLock.lock();
    if (_assetsToDownload.remove(path)) {
      ready = _assetsToDownload.isEmpty();
    }
    _assetsToDownloadLock.unlock();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    saveFileToCache(path, data, false);

    if (ready) {
      emit shaderReady(_downloadedShaderFile);
      _downloadedShaderFile = QString();
    }
  }


  void FileCache::assetDownloadFailed(QNetworkReply::NetworkError /*err*/)
  {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString path = reply->url().path();

    bool ready = false;
    _assetsToDownloadLock.lock();
    if (_assetsToDownload.remove(path)) {
      ready = _assetsToDownload.isEmpty();
    }
    _assetsToDownloadLock.unlock();

    reply->deleteLater();
    qCritical("Failed to download asset from ShaderToy (url was %s)", qPrintable(reply->url().toDisplayString()));

    if (ready) {
      emit shaderReady(_downloadedShaderFile);
      _downloadedShaderFile = QString();
    }
  }

} // namespace vh

