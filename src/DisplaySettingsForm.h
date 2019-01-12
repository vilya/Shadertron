// Copyright 2019 The Foundry
#ifndef DISPLAYSETTINGSFORM_H
#define DISPLAYSETTINGSFORM_H

#include <QWidget>

namespace Ui {
  class DisplaySettingsForm;
}

class DisplaySettingsForm : public QWidget
{
  Q_OBJECT

public:
  explicit DisplaySettingsForm(QWidget *parent = nullptr);
  ~DisplaySettingsForm();

private:
  Ui::DisplaySettingsForm *ui;
};

#endif // DISPLAYSETTINGSFORM_H
