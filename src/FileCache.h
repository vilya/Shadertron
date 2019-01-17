// Copyright 2019 Vilya Harvey
#ifndef VH_FILECACHE_H
#define VH_FILECACHE_H

#include <QDir>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QSet>
#include <QString>

namespace vh {

  class FileCache : public QObject
  {
    Q_OBJECT
  public:
    explicit FileCache(QObject *parent = nullptr);
    virtual ~FileCache();

    bool saveFileToCache(const QString& path, const QByteArray& data, QWidget* parentForErrorDialogs);
    QString pathForCachedFile(const QString& path);
    bool isCached(const QString& path);
    bool isResource(const QString& path);

  public slots:
    void fetchShaderToyByID(const QString& id);

  signals:
    void shaderReady(const QString& path);

  private slots:
    void shaderDownloaded();
    void shaderDownloadFailed(QNetworkReply::NetworkError err);

    void assetDownloaded();
    void assetDownloadFailed(QNetworkReply::NetworkError err);

  private:
    QNetworkAccessManager* _networkAccess = nullptr;

    QSet<QString> _assetsToDownload;
    QMutex _assetsToDownloadLock;
    QString _downloadedShaderFile;

    QDir _cacheDir;
  };

} // namespace vh

#endif // VH_FILECACHE_H
