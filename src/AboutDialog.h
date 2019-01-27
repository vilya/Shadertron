// Copyright 2019 Vilya Harvey
#ifndef VH_ABOUTDIALOG_H
#define VH_ABOUTDIALOG_H

#include <QDialog>
#include <QEvent>
#include <QObject>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QWidget>


namespace vh {

  class AboutDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit AboutDialog(QWidget* parent = nullptr);
    virtual ~AboutDialog();

    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual bool eventFilter(QObject* target, QEvent* event);

  private:
    QString buildDateText() const;

  private slots:
    void showAboutQt();

  private:
    QPixmap _background;
  };

} // namespace vh

#endif // VH_ABOUTDIALOG_H
