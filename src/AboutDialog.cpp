// Copyright 2019 Vilya Harvey
#include "AboutDialog.h"

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QPainter>
#include <QSpacerItem>

namespace vh {

  //
  // AboutDialog public methods
  //

  AboutDialog::AboutDialog(QWidget* parent) :
    QDialog(parent)
  {
    setObjectName("AboutDialog");
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowTitle(QString("About %1").arg(QApplication::instance()->applicationName()));
    resize(800, 450);

    QSizePolicy newSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    newSizePolicy.setHorizontalStretch(0);
    newSizePolicy.setVerticalStretch(0);
    newSizePolicy.setHeightForWidth(sizePolicy().hasHeightForWidth());
    setSizePolicy(newSizePolicy);

    QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setObjectName("horizontalLayout");

    QWidget* container = new QWidget(this);
    container->setObjectName("container");
    container->setAttribute(Qt::WA_TranslucentBackground);

    QGridLayout* gridLayout = new QGridLayout(container);
    gridLayout->setObjectName("gridLayout");

    QString copyrightText = QString("<html><head/><body><p>Copyright \302\251 %1 2019. All Rights Reserved.</p></body></html>").arg(QApplication::instance()->organizationName());
    QLabel* copyrightInfoLabel = new QLabel(copyrightText, container);
    copyrightInfoLabel->setObjectName("copyrightInfoLabel");
    copyrightInfoLabel->setAttribute(Qt::WA_TranslucentBackground);
    gridLayout->addWidget(copyrightInfoLabel, 2, 0, 1, 1);

    QString buildInfoText = QString("This is <span style=\" font-weight:600;\">%1 %2</span>, built on %3")
                            .arg(QApplication::instance()->applicationName())
                            .arg(QApplication::instance()->applicationVersion())
                            .arg(buildDateText());
    QLabel* buildInfoLabel = new QLabel(buildInfoText, container);
    buildInfoLabel->setObjectName("buildInfoLabel");
    buildInfoLabel->setAttribute(Qt::WA_TranslucentBackground);
    gridLayout->addWidget(buildInfoLabel, 1, 0, 1, 1);

    QPushButton* aboutQtButton = new QPushButton("About Qt...", container);
    aboutQtButton->setObjectName("aboutQtButton");
    gridLayout->addWidget(aboutQtButton, 2, 2, 1, 1);

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    gridLayout->addItem(horizontalSpacer, 2, 1, 1, 1);

    QSpacerItem* verticalSpacer = new QSpacerItem(20 ,40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout->addItem(verticalSpacer, 0, 1, 1, 1);

    horizontalLayout->addWidget(container);

    connect(aboutQtButton, &QPushButton::clicked, this, &AboutDialog::showAboutQt);

    installEventFilter(this);
  }


  AboutDialog::~AboutDialog()
  {
  }


  void AboutDialog::resizeEvent(QResizeEvent* event)
  {
    int w = event->size().width();
    int h = event->size().height();

    float xScale = float(w) / float(_background.width());
    float yScale = float(h) / float(_background.height());
    if (xScale > yScale) {
      h = int(_background.height() * xScale);
    }
    else if (yScale > xScale) {
      w = int(_background.width() * yScale);
    }

    int dw = std::abs(w - event->size().width());
    int dh = std::abs(h - event->size().height());
    if (dw > 1 || dh > 1) {
      resize(w, h);
    }
    _background = QPixmap(":/images/logo-background.jpg").scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    event->accept();
  }


  void AboutDialog::paintEvent(QPaintEvent* /*event*/)
  {
    QPainter painter(this);
    painter.drawPixmap(0, 0, _background);
  }


  bool AboutDialog::eventFilter(QObject *target, QEvent *event)
  {
    if (target == this && event->type() == QEvent::MouseButtonPress) {
      close();
      return true;
    }
    return false;
  }


  //
  // AboutDialog private methods
  //

  QString AboutDialog::buildDateText() const
  {
    QString str = QString("%1 %2").arg(__DATE__).arg(__TIME__);
    return str;
  }


  //
  // AboutDialog private slots
  //

  void AboutDialog::showAboutQt()
  {
    QMessageBox::aboutQt(this);
  }

} // namespace vh
