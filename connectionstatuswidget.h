#ifndef CONNECTIONSTATUSWIDGET_H
#define CONNECTIONSTATUSWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QBrush>
#include <QPaintEvent>
#include <qmath.h>
#include <QDebug>
/* Class for displaying connection status on UI
 */
class ConnectionStatusWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ConnectionStatusWidget(QWidget *parent = 0);

signals:
  void emitTrue(bool a);
protected:
  void paintEvent(QPaintEvent *);
public slots:
  void setStatus(bool status);
private:
  QColor diodeColor;
  void paintGreenDiode();
  void paintRedDiode();
};
#endif // CONNECTIONSTATUSWIDGET_H
