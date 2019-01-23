// Copyright 2019 Vilya Harvey
#ifndef VH_SHADERTOYDOWNLOADFORM_H
#define VH_SHADERTOYDOWNLOADFORM_H

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>
#include <QWidget>

namespace vh {

  class ShaderToyDownloadForm : public QDialog
  {
    Q_OBJECT
  public:
    explicit ShaderToyDownloadForm(QWidget *parent = nullptr);
    virtual ~ShaderToyDownloadForm();

    QString selectedShaderID() const;
    void setSelectedShaderID(const QString& id);

    bool forceDownload() const;
    void setForceDownload(bool val);

  signals:
    void downloadShader(const QString& shaderID, bool force);

  private slots:
    void onDownloadButtonClicked();

  private:
    QLineEdit* _shaderIDField      = nullptr;
    QCheckBox* _forceDownloadField = nullptr;
  };

} // namespace vh

#endif // VH_SHADERTOYDOWNLOADFORM_H
