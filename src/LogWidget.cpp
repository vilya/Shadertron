// Copyright 2019 Vilya Harvey
#include "LogWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace vh {

  //
  // LogWidget public methods
  //

  LogWidget::LogWidget(QWidget *parent) :
    QWidget(parent)
  {
    setObjectName("logWidget");

    QVBoxLayout* topLevelLayout = new QVBoxLayout(this);
    topLevelLayout->setObjectName("topLevelLayout");
    topLevelLayout->setContentsMargins(0, 0, 0, 0);

//    QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
//    horizontalLayout->setObjectName("horizontalLayout");
//    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    _messageList = new QListWidget(this);

    topLevelLayout->addWidget(_messageList);
  }


  int LogWidget::maxMessages() const
  {
    return _maxMessages;
  }


  //
  // LogWidget public slots
  //

  void LogWidget::addMessage(QtMsgType type, const QMessageLogContext& /*context*/, const QString &message)
  {
    QIcon icon;
    switch (type) {
    case QtDebugMsg:
      icon = style()->standardIcon(QStyle::SP_DialogNoButton);
      break;
    case QtWarningMsg:
      icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
      break;
    case QtCriticalMsg:
      icon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
      break;
    case QtFatalMsg:
      icon = style()->standardIcon(QStyle::SP_DialogCancelButton);
      break;
    case QtInfoMsg:
      icon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
      break;
    default:
      break;
    }

    QListWidgetItem* messageItem = new QListWidgetItem(icon, message);
    _messageList->addItem(messageItem);

    if (_maxMessages > 0) {
      while (_messageList->count() > _maxMessages) {
        delete _messageList->takeItem(0);
      }
    }

    _messageList->scrollToBottom();
  }


  void LogWidget::setMaxMessages(int maximum)
  {
    _maxMessages = maximum;
    if (_maxMessages > 0) {
      while (_messageList->count() > _maxMessages) {
        delete _messageList->takeItem(0);
      }
    }
  }

} // namespace vh
