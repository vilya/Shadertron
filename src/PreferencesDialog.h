// Copyright 2019 Vilya Harvey
#ifndef VH_PREFERENCESDIALOG_H
#define VH_PREFERENCESDIALOG_H

#include "Preferences.h"

#include <QDialog>


namespace Ui {
  class PreferencesDialog;
}


namespace vh {

  class PreferencesDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    virtual void closeEvent(QCloseEvent* event);

  private slots:
    void selectedSectionChanged(int newSection);

  private:
    void loadSettings();
    void connectWidgetsToSettings();

  private:
    Ui::PreferencesDialog *ui;
    Preferences _prefs;
  };

} // namespace vh

#endif // VH_PREFERENCESDIALOG_H

