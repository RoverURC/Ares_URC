#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include "modbus.h"
#include <QtEndian>
/* Class providing modbus TCP interface, data is stored in holding registers array(type quint16)
 * Holding Registers Array has 127 registers, each one is 2 bytes long
 * To communicate with this class we need ModbusClient class, communication is over TCP IP protocol
 * Only three modbus Functions are implemented :
 * MODBUS_FC_READ_HOLDING_REGISTERS,
 * MODBUS_FC_WRITE_MULTIPLE_REGISTERS
 * MODBUS_FC_WRITE_SINGLE_REGISTER
 *
 * To use this class we need to create object and set it into startListening state. In object constructor we pass
 * Modbus TCP Port number which is by default 502
 *
 * You can directly read/write into holding registers array using functions setRegister, getRegister
 *
 * */
class ModbusServer : public QObject
{
    Q_OBJECT
public:
  explicit ModbusServer(int modbusPort=502, QObject *parent = 0);
  ~ModbusServer();

  bool startListening();

  bool setRegister(int index, quint16 value);
  bool getRegister(int index, quint16 &value);

signals:
  void newData();

private slots:
  void newConnection();
  void readDataAndRespond();

private:
  QTcpServer *myServer;
  QTcpSocket *mySocket;
  int modbusPort;

  QByteArray byteArrayInput;
  QByteArray byteArrayOutput;

  quint16 *holdingRegisters;
  int holdingRegistersSize;

  bool getQInt16(QByteArray array, int index, quint16 &output);
  bool setQInt16(QByteArray &array, int index, quint16 input );

  bool getQInt8(QByteArray array, int index, quint8 &output);
  bool setQInt8(QByteArray &array, int index, quint8 input );

  bool proceedData();
  bool formatOutputHeader(quint16 lenght);

  void readHoldingRegister();
  void writeSingleRegister();
  void writeMultipleRegisters();
};

#endif // MODBUSSERVER_H
