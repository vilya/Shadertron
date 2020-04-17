// Copyright 2019 Vilya Harvey
#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

#include "Preferences.h"

#include <QLabel>
#include <QListWidget>

namespace vh {

  //
  // PreferencesDialog public methods
  //

  PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
  {
    ui->setupUi(this);
    ui->sectionChooser->setFixedWidth(ui->sectionChooser->sizeHintForColumn(0) + 2 * ui->sectionChooser->frameWidth());

    loadSettings();
    connectWidgetsToSettings();

    connect(ui->sectionChooser, &QListWidget::currentRowChanged, this, &PreferencesDialog::selectedSectionChanged);
  }


  PreferencesDialog::~PreferencesDialog()
  {
    delete ui;
  }


  void PreferencesDialog::closeEvent(QCloseEvent* /*event*/)
  {
    deleteLater();
  }


  //
  // PreferencesDialog private slots
  //

  void PreferencesDialog::selectedSectionChanged(int newSection)
  {
    if (newSection < 0 || newSection >= ui->sectionChooser->count()) {
      return;
    }

    QString newTitle = ui->sectionChooser->item(newSection)->text();

    ui->sectionContainer->setCurrentIndex(newSection);
    ui->sectionNameField->setText(newTitle);
  }


  //
  // PreferencesDialog private methods
  //

  void PreferencesDialog::loadSettings()
  {
    // Load settings for the startup section
    ui->reopenLastShaderField->setChecked(_prefs.reopenLastShaderOnStartup());
  }


  void PreferencesDialog::connectWidgetsToSettings()
  {
    // Connections for the Startup section
    connect(ui->reopenLastShaderField, &QCheckBox::toggled, &_prefs, &Preferences::setReopenLastShaderOnStartup);

    // Connections for the View section
  }

} // namespace vh
