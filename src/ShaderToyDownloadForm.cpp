// Copyright 2019 Vilya Harvey
#include "ShaderToyDownloadForm.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

namespace vh {

  //
  // ShaderToyDownloadForm public methods
  //

  ShaderToyDownloadForm::ShaderToyDownloadForm(QWidget *parent) :
    QDialog(parent)
  {
    setSizeGripEnabled(true);

    QGridLayout* topLayout = new QGridLayout();
    setLayout(topLayout);

    _shaderIDField      = new QLineEdit();
    _forceDownloadField = new QCheckBox();

    QLabel* shaderIDLabel       = new QLabel("Shader ID or URL");
    QLabel* forceDownloadLabel  = new QLabel("Force download");

    QDialogButtonBox* buttons = new QDialogButtonBox(Qt::Horizontal);
    buttons->addButton(QString("Download"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QDialogButtonBox::Cancel);

    topLayout->addWidget(shaderIDLabel,       0, 0);
    topLayout->addWidget(_shaderIDField,      0, 1);
    topLayout->addWidget(forceDownloadLabel,  1, 0);
    topLayout->addWidget(_forceDownloadField, 1, 1);
    topLayout->addWidget(buttons,             2, 0, 1, 2);

    connect(buttons, &QDialogButtonBox::accepted, this, &ShaderToyDownloadForm::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ShaderToyDownloadForm::reject);

//    connect(downloadButton, &QPushButton::clicked, this, &ShaderToyDownloadForm::onDownloadButtonClicked);
  }


  ShaderToyDownloadForm::~ShaderToyDownloadForm()
  {
  }


  QString ShaderToyDownloadForm::selectedShaderID() const
  {
    return _shaderIDField->text();
  }


  void ShaderToyDownloadForm::setSelectedShaderID(const QString& id)
  {
    _shaderIDField->setText(id);
  }


  bool ShaderToyDownloadForm::forceDownload() const
  {
    return _forceDownloadField->isChecked();
  }


  void ShaderToyDownloadForm::setForceDownload(bool val)
  {
    _forceDownloadField->setChecked(val);
  }


  //
  // ShaderToyDownloadForm private slots
  //

  void ShaderToyDownloadForm::onDownloadButtonClicked()
  {
//    emit downloadShader(selectedShaderID(), forceDownload());
    accept();
  }


} // namespace vh
