// Copyright 2019 Vilya Harvey
#ifndef VH_LOGWIDGET_H
#define VH_LOGWIDGET_H

#include <QListWidget>
#include <QWidget>

namespace vh {

  class LogWidget : public QWidget
  {
    Q_OBJECT
  public:
    explicit LogWidget(QWidget *parent = nullptr);

    // If return value is <= 0, there's no maximum.
    int maxMessages() const;

  public slots:
    void addMessage(QtMsgType type, const QMessageLogContext& context, const QString& message);
    void setMaxMessages(int maximum);

  private:
    QListWidget* _messageList = nullptr;
    int _maxMessages = 1000;
  };

} // namespace vh

#endif // VH_LOGWIDGET_H
