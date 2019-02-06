// Copyright 2019 Vilya Harvey
#ifndef VH_PREFERENCES_H
#define VH_PREFERENCES_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>

namespace vh {

  //
  // Constants
  //

  static constexpr int kMaxRecentFiles     = 10;
  static constexpr int kMaxRecentDownloads = 10;

  static constexpr uint kHUD_FrameNum       = 1u << 0;
  static constexpr uint kHUD_Time           = 1u << 1;
  static constexpr uint kHUD_MillisPerFrame = 1u << 2;
  static constexpr uint kHUD_FramesPerSec   = 1u << 3;
  static constexpr uint kHUD_MousePos       = 1u << 4;
  static constexpr uint kHUD_MouseDownPos   = 1u << 5;

  static constexpr uint kHUD_All = kHUD_FrameNum | kHUD_Time | kHUD_MillisPerFrame |
                                   kHUD_FramesPerSec | kHUD_MousePos | kHUD_MouseDownPos;


  //
  // Structs
  //

  struct Download {
    QString id;
    QString name;

    bool operator == (const Download& other) const { return id == other.id; }
    bool operator != (const Download& other) const { return id != other.id; }
  };


  //
  // Preferences class
  //

  class Preferences : public QObject
  {
    Q_OBJECT
  public:
    Preferences();

    QString lastOpenDir() const;
    QString lastSaveDir() const;
    QString lastScreenshotDir() const;
    QList<QString> recentFileList() const;
    QList<Download> recentDownloadList() const;
    int desktopWindowVersion() const;
    QByteArray desktopWindowGeometry() const;
    QByteArray desktopWindowState() const;
    uint hudFlags() const;

  public slots:
    void setLastOpenDir(const QString& dirname);
    void setLastSaveDir(const QString& dirname);
    void setLastScreenshotDir(const QString& dirname);
    void setRecentFileList(const QList<QString>& recentFiles);
    void setRecentDownloadList(const QList<Download>& recentDownloads);
    void saveDesktopWindowData(const QByteArray& geometry, const QByteArray& state, int version);
    void removeDesktopWindowData();
    void setHUDFlags(uint flags);

  private:
    QSettings _settings;
  };

} // namespace vh

#endif // VH_PREFERENCES_H
