// Copyright 2019 Vilya Harvey
#include "Preferences.h"

#include <QStandardPaths>

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

  static const QString kLastOpenDir = "lastOpenDir";
  static const QString kLastSaveDir = "lastSaveDir";
  static const QString kLastScreenshotDir = "lastScreenshotDir";


  //
  // Preferences public methods
  //

  Preferences::Preferences()
  {
  }


  QString Preferences::lastOpenDir() const
  {
    QString dirname = _settings.value(kLastOpenDir).toString();
    if (dirname.isEmpty()) {
      dirname = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    return dirname;
  }


  QString Preferences::lastSaveDir() const
  {
    QString dirname = _settings.value(kLastSaveDir).toString();
    if (dirname.isEmpty()) {
      dirname = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    return dirname;
  }


  QString Preferences::lastScreenshotDir() const
  {
    QString dirname = _settings.value(kLastScreenshotDir).toString();
    if (dirname.isEmpty()) {
      dirname = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    return dirname;
  }


  QList<QString> Preferences::recentFileList() const
  {
    QList<QString> recentFiles;

    for (int i = 0; i < kMaxRecentFiles; i++) {
      QString key = kRecentFileX.arg(i);
      QString filename = _settings.value(key).toString();
      if (filename.isNull() || filename.isEmpty()) {
        break;
      }
      recentFiles.push_back(filename);
    }

    return recentFiles;
  }


  QList<Download> Preferences::recentDownloadList() const
  {
    QList<Download> recentDownloads;

    for (int i = 0; i < kMaxRecentDownloads; i++) {
      QString idKey   = kRecentDownloadIDX.arg(i);
      QString nameKey = kRecentDownloadNameX.arg(i);
      Download download;
      download.id   = _settings.value(idKey).toString();
      download.name = _settings.value(nameKey).toString();
      if (download.id.isNull() || download.id.isEmpty() || download.name.isNull() || download.name.isEmpty()) {
        break;
      }
      recentDownloads.push_back(download);
    }

    return recentDownloads;
  }


  int Preferences::desktopWindowVersion() const
  {
    return _settings.value(kDesktopWindowVersion, -1).toInt();
  }


  QByteArray Preferences::desktopWindowGeometry() const
  {
    return _settings.value(kDesktopWindowGeometry).toByteArray();
  }


  QByteArray Preferences::desktopWindowState() const
  {
    return _settings.value(kDesktopWindowState).toByteArray();
  }


  //
  // Preferences public slots
  //

  void Preferences::setLastOpenDir(const QString& dirname)
  {
    if (dirname.isEmpty()) {
      _settings.remove(kLastOpenDir);
    }
    else {
      _settings.setValue(kLastOpenDir, dirname);
    }
  }


  void Preferences::setLastSaveDir(const QString& dirname)
  {
    if (dirname.isEmpty()) {
      _settings.remove(kLastSaveDir);
    }
    else {
      _settings.setValue(kLastSaveDir, dirname);
    }
  }


  void Preferences::setLastScreenshotDir(const QString& dirname)
  {
    if (dirname.isEmpty()) {
      _settings.remove(kLastScreenshotDir);
    }
    else {
      _settings.setValue(kLastScreenshotDir, dirname);
    }
  }


  void Preferences::setRecentFileList(const QList<QString>& recentFiles)
  {
    int numFiles = qMin(kMaxRecentFiles, recentFiles.size());
    for (int i = 0; i < numFiles; i++) {
      QString key = kRecentFileX.arg(i);
      _settings.setValue(key, recentFiles[i]);
    }
    for (int i = numFiles; i < kMaxRecentDownloads; i++) {
      _settings.remove(kRecentFileX.arg(i));
    }
  }


  void Preferences::setRecentDownloadList(const QList<Download>& recentDownloads)
  {
    for (int i = 0, end = qMin(recentDownloads.size(), kMaxRecentDownloads); i < end; i++) {
      QString idKey   = kRecentDownloadIDX.arg(i);
      QString nameKey = kRecentDownloadNameX.arg(i);
      _settings.setValue(idKey,   recentDownloads[i].id);
      _settings.setValue(nameKey, recentDownloads[i].name);
    }
  }


  void Preferences::saveDesktopWindowData(const QByteArray &geometry, const QByteArray &state, int version)
  {
    _settings.setValue(kDesktopWindowGeometry, geometry);
    _settings.setValue(kDesktopWindowState, state);
    _settings.setValue(kDesktopWindowVersion, version);
  }


  void Preferences::removeDesktopWindowData()
  {
    _settings.remove(kDesktopWindowGeometry);
    _settings.remove(kDesktopWindowState);
    _settings.remove(kDesktopWindowVersion);
  }

} // namespace vh
