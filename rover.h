#ifndef ROVER_H
#define ROVER_H

#define CONNECTION_INDICATOR 7


#include <QObject>
#include "modbusclient.h"
#include "qmath.h"
#include <limits>
/*How data is organized in HoldingRegisters for Rover Unit
 *
 * Motor Diagnostic
  * Motor 16
    1	5V measurement
    2	12V measurement
    3	24V measurement
    4	Motor supply measurement
    5	Motor current measurement
  * Motor 17
    6	5V measurement
    7	12V measurement
    8	24V measurement
    9	Motor supply measurement
    10	Motor current measurement
  * Motor 18
    11	5V measurement
    12	12V measurement
    13	24V measurement
    14	Motor supply measurement
    15	Motor current measurement
  * Motor 19
    16	5V measurement
    17	12V measurement
    18	24V measurement
    19	Motor supply measurement
    20	Motor current measurement
  * Motor 20
    21	5V measurement
    22	12V measurement
    23	24V measurement
    24	Motor supply measurement
    25	Motor current measurement
  * Motor 21
    26	5V measurement
    27	12V measurement
    28	24V measurement
    29	Motor supply measurement
    30	Motor current measurement
 * Motor Control
    31  Drive Mode
    32	Motor 1,2,3 PWM
    33	Motor 4,5,6 PWM

 * GPS Receiver
  * Time
    40	GPS On/Off	0..1
    41	Hours	0..23	Time
    42	Minutes	0..59
    43	Seconds	0..59
    44	Miliseconds*10000	0..9999
    45	Data valid indicator	0..1

  * Latitude
    46	Degrees	0..90
    47	Minutes	0..59
    48	Minutes*10000	0..9999
    49	Designator	'N', 'S'
  * Longitude
    50	Degrees	0..90
    51	Minutes	0..59
    52	Minutes*10000	0..9999
    53	Designator	'W', 'S'

    54	Speed*10 [km/h]	0..1854
    55	COG*10 [degrees]	?
  * Date
    56	Day	0..31
    57	Month	0..12
    58	Year (last two digit)	0..99
    59	Positioning mode	some char :P
    60	Checksum (tbdl)	0
 */

class Rover : public ModbusClient
{
  Q_OBJECT
public:

  static int const leftAxis;
  static int const rightAxis;
  static int const numberOfSpeedButtons;
  static int const speedButtonsIDTable[];
  static int const speedButtonsSpeedsTable[];

  explicit Rover(QObject *parent = 0);
  int getGoodResponseNumber() const {return goodResponseCounter;}
  int getRequestNumber() const { return requestCounter;}

  int getLeftPWM() const;
  int getRightPWM() const;
signals:
  void roverDataUpdated();
public slots:
  //Sends and reads RoverData
  void updateRoverData();
  void interpretJoypadButton(int id, bool status);
  void interpretJoypadAxis(int id, qint16 value);

private slots:
  void proceedResponse(bool status, qint8 errorCode);
private:
  int goodResponseCounter;
  int requestCounter;
  void setLeftPWM(qint16 value);
  void setRightPWM(qint16 value);
  void sendRoverData();
  void readRoverData();
  void readGPSData();
};

#endif // ROVER_H
