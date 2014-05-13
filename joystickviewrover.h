#ifndef JOYSTICKVIEWROVER_H
#define JOYSTICKVIEWROVER_H

#include <QWidget>

namespace Ui {
  class JoystickViewRover;
}

class JoystickViewRover : public QWidget
{
  Q_OBJECT

public:
  explicit JoystickViewRover(QWidget *parent = 0);
  ~JoystickViewRover();

public slots:

  void changeButtonState(int id, bool status);
  void changeAxisStat(int id, qint16 value);

private:
  Ui::JoystickViewRover *ui;
};

#endif // JOYSTICKVIEWROVER_H
