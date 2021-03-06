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

  const QString kShaderToyViewURLPrefix("https://www.shadertoy.com/view/");

  static const QString kShaderToyShaderURL("https://www.shadertoy.com/api/v1/shaders/%1?key=%2");
  static const QString kShaderToyAssetURL("https://www.shadertoy.com/%1?key=%2");

  static const QString kShaderToyShaderPath("api/v1/shaders/%1.json");
  static const QString kShaderToyThumbnailPath("/media/shaders/%1.jpg");


  static const QString kStandardShaderToyAssets[] = {
    // Images
    QString::fromUtf8("/media/a/08b42b43ae9d3c0605da11d0eac86618ea888e62cdd9518ee8b9097488b31560.png"), // SDF font texture
    QString::fromUtf8("/media/a/0a40562379b63dfb89227e6d172f39fdce9022cba76623f1054a2c83d6c0ba5d.png"),
    QString::fromUtf8("/media/a/0c7bf5fe9462d5bffbd11126e82908e39be3ce56220d900f633d58fb432e56f5.png"),
    QString::fromUtf8("/media/a/52d2a8f514c4fd2d9866587f4d7b2a5bfa1a11a0e772077d7682deb8b3b517e5.jpg"),
    QString::fromUtf8("/media/a/85a6d68622b36995ccb98a89bbb119edf167c914660e4450d313de049320005c.png"), // the bayer texture
    QString::fromUtf8("/media/a/95b90082f799f48677b4f206d856ad572f1d178c676269eac6347631d4447258.jpg"),
    QString::fromUtf8("/media/a/92d7758c402f0927011ca8d0a7e40251439fba3a1dac26f5b8b62026323501aa.jpg"),
    QString::fromUtf8("/media/a/ad56fba948dfba9ae698198c109e71f118a54d209c0ea50d77ea546abad89c57.png"),
    QString::fromUtf8("/media/a/cd4c518bc6ef165c39d4405b347b51ba40f8d7a065ab0e8d2e4f422cbc1e8a43.jpg"),
    QString::fromUtf8("/media/a/e6e5631ce1237ae4c05b3563eda686400a401df4548d0f9fad40ecac1659c46c.jpg"),
    QString::fromUtf8("/media/a/f735bee5b64ef98879dc618b016ecf7939a5756040c2cde21ccb15e69a6e1cfb.png"),
    // Cubemaps
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_1.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_2.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_3.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_4.png"),
    QString::fromUtf8("/media/a/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785_5.png"),

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

    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_1.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_2.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_3.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_4.png"),
    QString::fromUtf8("/media/a/94284d43be78f00eb6b298e6d78656a1b34e2b91b34940d02f1ca8b22310e8a0_5.png"),

    // Videos
    QString::fromUtf8("/media/a/3405e48f74815c7baa49133bdc835142948381fbe003ad2f12f5087715731153.ogv"),   // "Lustre-Cream" ad
    QString::fromUtf8("/media/a/e81e818ac76a8983d746784b423178ee9f6cdcdf7f8e8d719341a6fe2d2ab303.webm"),  // Britney Spears green-screen clip
    QString::fromUtf8("/media/a/35c87bcb8d7af24c54d41122dadb619dd920646a0bd0e477e7bdc6d12876df17.webm"),  // Jean Claude Van Damme green-screen clip
    QString::fromUtf8("/media/a/c3a071ecf273428bc72fc72b2dd972671de8da420a2d4f917b75d20e1c24b34c.ogv"),   // The Google Eyeball video

    // Music
    QString::fromUtf8("/media/a/a6a1cf7a09adfed8c362492c88c30d74fb3d2f4f7ba180ba34b98556660fada1.mp3"),   // 8-bit Mentality
    QString::fromUtf8("/media/a/d96b229eeb7a08d53adfcf1ff89e54c9ffeebed193d317d1a01cc8125c0f5cca.mp3"),   // not sure, used by shader wdBGDh
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

  bool FileCache::fetchShaderToyByIDorURL(const QString& idOrURL, bool forceDownload)
  {
    QString id = idOrURL;

    // If we've been given a URL, extract the ID from it. The only URLs that we
    // accept are ShaderToy "view" URLs, i.e. ones that have the form
    // `https://www.shadertoy.com/view/<shader-id>`
    if (id.startsWith(kShaderToyViewURLPrefix)) {
      id = id.mid(kShaderToyViewURLPrefix.size());
    }

    // Validate the id. All ShaderToy IDs are 6 characters long and consist of
    // only digits and upper- or lower-case letters.
    if (id.size() != 6) {
      return false;
    }
    for (int i = 0; i < id.length(); i++) {
      if (!id.at(i).isLetterOrNumber()) {
        return false;
      }
    }

    if (_networkAccess == nullptr) {
      _networkAccess = new QNetworkAccessManager(this);
    }

    QString path = kShaderToyShaderPath.arg(id);
    if (isCached(path) && !forceDownload) {
      qDebug("Using cached copy of shader with id %s", qPrintable(id));
      _downloadedShaderFile = pathForCachedFile(path);
      fetchAssetsForDownloadedShader();
      return true;
    }

    QString urlStr = kShaderToyShaderURL.arg(id).arg(kShaderToyAppKey);
    QNetworkReply* reply = _networkAccess->get(QNetworkRequest(QUrl(urlStr)));
    connect(reply, &QNetworkReply::finished, this, &FileCache::shaderDownloaded);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &FileCache::shaderDownloadFailed);
    return true;
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

    if (QFileInfo(path).suffix() != "json") {
      path += ".json";
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QWidget* parentForErrorDialogs = qobject_cast<QWidget*>(parent());

    // Save data to a local file, then open it
    if (!saveFileToCache(path, data, parentForErrorDialogs)) {
      return;
    }

    qDebug("Successfully downloaded %s", qPrintable(reply->url().toDisplayString()));
    _downloadedShaderFile = pathForCachedFile(path);

    fetchAssetsForDownloadedShader();
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

    saveFileToCache(path, data, nullptr);

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


  //
  // FileCache private methods
  //

  void FileCache::fetchAssetsForDownloadedShader()
  {
    if (_downloadedShaderFile.isEmpty()) {
      return;
    }

    QWidget* parentForErrorDialogs = qobject_cast<QWidget*>(parent());

    // Download successful, now let's parse the document so we can also grab
    // any required assets for it.
    ShaderToyDocument* document = nullptr;
    try {
      document = loadShaderToyJSONFile(_downloadedShaderFile);
    }
    catch (const std::runtime_error& err) {
      qCritical("Unable to load %s: %s", qPrintable(_downloadedShaderFile), err.what());
      QMessageBox::critical(parentForErrorDialogs, "Load failed", QString("Unable to load %1: %2").arg(_downloadedShaderFile).arg(err.what()));      
      QFile::remove(_downloadedShaderFile);
      return;
    }

    QSet<QString> requiredAssets;

    // The thumbnail for this shader
    requiredAssets.insert(kShaderToyThumbnailPath.arg(document->info.id));

    for (int passIdx = 0; passIdx < document->renderpasses.size(); passIdx++) {
      ShaderToyRenderPass& pass = document->renderpasses[passIdx];
      for (int i = 0; i < pass.inputs.size(); i++) {
        if (pass.inputs[i].ctype == kInputType_Texture ||
            pass.inputs[i].ctype == kInputType_Video  ||
            pass.inputs[i].ctype == kInputType_Music) {
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

      // Check whether the file is already in the cache. If so, we assume it's
      // up to date.
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

} // namespace vh

