#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QtEndian>
#include <QTimer>
#include "modbus.h"
/*
 * Class providing Modbus Client. All data recived from Modbus Server, and data prepared to be send
 * are stored in internal array holdingRegisters, which is 127 registers long. We acces from program
 * to those registers using setRegister, getRegister functions.
 *
 * To establish connection to modbus server we need to call connectToModbusServer function passing server
 * ip and port. When disconnected class emits signal modbusClientDisconnected;
 *
 * When we use function to send data, it will format output frame, and set timeoutTiemer, then client will wait
 * for signal from timeoutTimer (no response) or from tcpSocket (data ready). Then Client will analyze data, and
 * proceed operations on holding registers. It's imposible to send another message before receiving back frame or
 * timer timeout
 *
 * To send data over TCP/IP using this class we must:
 * 1.Create ModbusClient
 * 2.Connect ModbusClient to ModbusServer
 * 3.(option) Write Data into holding registers
 * 4.Use one of the communication functions
 * 5.Wait for signal modbusDataReady or signal timeout
 * 6.(option) Read Data from holding registers
 * 7. Repeat from (3)
 */
class ModbusClient : public QObject
{
  Q_OBJECT
public:
  explicit ModbusClient(QObject *parent = 0);
  ~ModbusClient();

  //Modbus requests
  bool writeSingleRegister(quint16 registerAddress);
  bool writeMultipleRegisters(quint16 startingAddress, quint16 quantityOfRegisters);
  bool readHoldingRegisters(quint16 startingAddress, quint16 quantityOfRegisters);

  bool setRegister(int index, quint16 value);
  bool getRegister(int index, quint16 &value) const;

  void setResponseTimerTimeout(int ms){ responseTimerTimeout = ms;}

public slots:
  bool connectToModbusServer(QString ip, int port);
  bool disconnectFromModbusServer();
signals:
  void statusConnectedChanged(bool);
  
  // Bad response, Good response or Timeout
  void transactionFinished(bool, qint8 errorCode);
  
private slots:
  void transactionTimeout();
  void connected();
  void disconnected();
  void readDataAndCheck();
protected:
  bool isWaitingForResponse;
private:

  QTcpSocket *mySocket;
  bool statusConnected;
  bool proceedResponse();
  bool proceedWriteSingleRegisterResponse();
  bool proceedWriteMultipleRegistersResponse();
  bool proceedReadHoldingRegistersResponse();



  QTimer *responseTimer;
  int holdingRegistersSize;
  int responseTimerTimeout;
  quint8 transactionID;


  quint16 *holdingRegisters;
  quint8 waitingFunctionCode;

  void formatHeader(quint16 length);
  QByteArray byteArrayInput;
  QByteArray byteArrayOutput;

  bool getQInt16(QByteArray array, int index, quint16 &output);
  bool setQInt16(QByteArray &array, int index, quint16 input );

  bool getQInt8(QByteArray array, int index, quint8 &output);
  bool setQInt8(QByteArray &array, int index, quint8 input );

  bool proceedIncomingError();
};

#endif // MODBUSCLIENT_H
