// Copyright 2019 Vilya Harvey
#include "FileCache.h"

#include "ShaderToy.h"

#include <QMessageBox>
#include <QMessageLogger>
#include <QStandardPaths>

namespace vh {

  //
  // Constants
  //

  static const QString kShaderToyShaderURL("https://www.shadertoy.com/api/v1/shaders/%1?key=%2");
  static const QString kShaderToyAssetURL("https://www.shadertoy.com/%1?key=%2");


  static const QString kStandardShaderToyAssets[] = {
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_1.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_2.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_3.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_4.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_5.png"),
    QString::fromUtf8("/media/a/0a40562379b63dfb89227e6d172f39fdce9022cba76623f1054a2c83d6c0ba5d.png"),
    QString::fromUtf8("/media/a/0c7bf5fe9462d5bffbd11126e82908e39be3ce56220d900f633d58fb432e56f5.png"),
    QString::fromUtf8("/media/a/585f9546c092f53ded45332b343144396c0b2d70d9965f585ebc172080d8aa58.jpg"),
    QString::fromUtf8("/media/a/585f9546c092f53ded45332b343144396c0b2d70d9965f585ebc172080d8aa58_1.jpg"),
    QString::fromUtf8("/media/a/585f9546c092f53ded45332b343144396c0b2d70d9965f585ebc172080d8aa58_2.jpg"),
    QString::fromUtf8("/media/a/585f9546c092f53ded45332b343144396c0b2d70d9965f585ebc172080d8aa58_3.jpg"),
    QString::fromUtf8("/media/a/585f9546c092f53ded45332b343144396c0b2d70d9965f585ebc172080d8aa58_4.jpg"),
    QString::fromUtf8("/media/a/585f9546c092f53ded45332b343144396c0b2d70d9965f585ebc172080d8aa58_5.jpg"),
    QString::fromUtf8("/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232.png"),
    QString::fromUtf8("/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232_1.png"),
    QString::fromUtf8("/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232_2.png"),
    QString::fromUtf8("/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232_3.png"),
    QString::fromUtf8("/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232_4.png"),
    QString::fromUtf8("/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232_5.png"),
    QString::fromUtf8("/media/a/92d7758c402f0927011ca8d0a7e40251439fba3a1dac26f5b8b62026323501aa.jpg"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_1.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_2.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_3.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_4.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_5.png"),
    QString::fromUtf8("/media/a/ad56fba948dfba9ae698198c109e71f118a54d209c0ea50d77ea546abad89c57.png"),
    QString::fromUtf8("/media/a/cd4c518bc6ef165c39d4405b347b51ba40f8d7a065ab0e8d2e4f422cbc1e8a43.jpg"),
    QString::fromUtf8("/media/a/f735bee5b64ef98879dc618b016ecf7939a5756040c2cde21ccb15e69a6e1cfb.png"),
  };


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


  QDir FileCache::cacheDir() const
  {
    return _cacheDir;
  }


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

    QString urlStr = kShaderToyShaderURL.arg(id).arg(kShaderToyAppKey);
    QNetworkReply* reply = _networkAccess->get(QNetworkRequest(QUrl(urlStr)));
    connect(reply, &QNetworkReply::finished, this, &FileCache::shaderDownloaded);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &FileCache::shaderDownloadFailed);
  }


  void FileCache::fetchShaderToyStandardAssets()
  {
    if (_networkAccess == nullptr) {
      _networkAccess = new QNetworkAccessManager(this);
    }

    _assetsToDownloadLock.lock();
    _assetsToDownload.clear();
    _downloadedShaderFile.clear();
    for (const QString& path : kStandardShaderToyAssets) {
      if (isCached(path)) {
        qDebug("Already have %s", qPrintable(path));
        continue;
      }

      if (_assetsToDownload.contains(path)) {
        continue;
      }

      qDebug("Need to download %s", qPrintable(path));
      _assetsToDownload.insert(path);

      QString urlStr = kShaderToyAssetURL.arg(path.mid(1)).arg(kShaderToyAppKey);
      QNetworkReply* reply = _networkAccess->get(QNetworkRequest(QUrl(urlStr)));
      connect(reply, &QNetworkReply::finished, this, &FileCache::assetDownloaded);
      connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &FileCache::assetDownloadFailed);
    }
    bool ready = _assetsToDownload.empty();
    _assetsToDownloadLock.unlock();
    if (ready) {
      emit standardAssetsReady();
    }
  }


  void FileCache::deleteCache()
  {
    qDebug("Removing all files and subdirectories from the cache");

    QDir dir = _cacheDir;

    // Remove all files inside the directory.
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
    for (QString filename : dir.entryList()) {
      qDebug("Removing %s", qPrintable(filename));
      dir.remove(filename);
    }

    // Remove all subdirectories inside the directory.
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    for (QString dirname : dir.entryList()) {
      qDebug("Removing %s", qPrintable(dirname));
      QDir subdir = QDir(dir.absoluteFilePath(dirname));
      subdir.removeRecursively();
    }
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

    qDebug("Successfully downloaded %s", qPrintable(path));
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

      QString urlStr = kShaderToyAssetURL.arg(src.mid(1)).arg(kShaderToyAppKey);
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
      qDebug("Successfully downloaded asset %s", qPrintable(path));
      ready = _assetsToDownload.isEmpty();
    }
    _assetsToDownloadLock.unlock();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    saveFileToCache(path, data, false);

    if (ready) {
      if (_downloadedShaderFile.isEmpty()) {
        emit standardAssetsReady();
      }
      else {
        emit shaderReady(_downloadedShaderFile);
        _downloadedShaderFile = QString();
      }
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
      if (_downloadedShaderFile.isEmpty()) {
        emit standardAssetsReady();
      }
      else {
        emit shaderReady(_downloadedShaderFile);
        _downloadedShaderFile = QString();
      }
    }
  }

} // namespace vh

