#ifndef MANIPULATOR_H
#define MANIPULATOR_H
#include <QObject>
#include "modbusclient.h"

#include "qmath.h"
#include <limits>
class Manipulator : public ModbusClient
{
  Q_OBJECT
public:
  explicit Manipulator(QObject *parent = 0);
  ~Manipulator();
  void resetManipulator();

  static int const axisNumber;
  static int const axisInit[];
  static int const axisMin[];
  static int const axisMax[];
  static int const axisButtonId[];

  int getAxisValue(int index);
  int getGoodResponseNumber() const {return goodResponseCounter;}
  int getRequestNumber() const { return requestCounter;}
signals:
  void manipulatorDataUpdated();
public slots:
  void interpretJoypadButton(int id, bool status);
  void interpretJoypadAxis(int id, qint16 value);
  void updateManipulatorData();
private slots:
  void incrementManipulatorAxisValues();
  void proceedResponse(bool status, qint8 errorCode);
private:
  void sendManipulatorData();

  qint16 actualJoypadAxisValue;
  qint16 *axisValues;
  bool *axisStatus;
  QTimer *incrementAxisTimer;

  int requestCounter;
  int goodResponseCounter;
};

#endif // MANIPULATOR_H
