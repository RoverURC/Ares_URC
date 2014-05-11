#include "modbusserver.h"

ModbusServer::ModbusServer(int modbusPort, QObject *parent) :
  QObject(parent)
{
  mySocket = NULL;
  myServer = new QTcpServer(this);

  this->modbusPort = modbusPort;

  holdingRegistersSize = 127;

  holdingRegisters = new quint16[holdingRegistersSize];

  for(int i=0 ; i<holdingRegistersSize ; i++)
    holdingRegisters[i]=0;

  connect (myServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}
ModbusServer::~ModbusServer(){
  delete[] holdingRegisters;
}

bool ModbusServer::startListening(){
  if(!myServer->listen(QHostAddress::Any, modbusPort))
    return false;

  return true;
}
void ModbusServer::newConnection(){
  if(mySocket == NULL){
    mySocket=myServer->nextPendingConnection();
    connect(mySocket, SIGNAL(readyRead()), this, SLOT(readDataAndRespond()));
    connect(mySocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  }
  else{
    mySocket->deleteLater();
    mySocket = myServer->nextPendingConnection();
  }
}
void ModbusServer::readDataAndRespond(){
  byteArrayInput.clear();
  byteArrayInput = mySocket->readAll();

  if(proceedData()){
    emit newData();
  }

  mySocket->write(byteArrayOutput,byteArrayOutput.size());
  mySocket->flush();
}
bool ModbusServer::proceedData(){
  //First clear data which will be send later
  byteArrayOutput.clear();

//Procesing Header
  //All errors for which frame isnt properly formated will be ignored

  //Check Minimal header size
  if(byteArrayInput.size()<8){
    return false;
  }

  //Check if protocol Identifier is equal '0' (Modbus)
  quint16 protocolIdentifier;
  getQInt16(byteArrayInput, 2, protocolIdentifier);
  if(!protocolIdentifier==0)
    return false;

  //Unit identyfier should be equal 255 in ModbusTCP
  quint8 unitIdentifier;
  getQInt8(byteArrayInput, 6, unitIdentifier);

  if(unitIdentifier!=255)
    return false;
  //Get basic info
  quint16 lenght;
  getQInt16(byteArrayInput, 4, lenght);
  if(byteArrayInput.size()!=(lenght+6))
    return false;

  quint16 dataSize = lenght-2;

  quint8 functionCode;
  getQInt8(byteArrayInput, 7, functionCode);

//Procesing Body
  //Interpret Function Code
  switch (functionCode){
    case MODBUS_FC_READ_HOLDING_REGISTERS :{
        qDebug()<<"ReadHoldingRegistrs Server";

      if(dataSize!=4)
        return false;
      if(byteArrayInput.size()!=12)
        return false;

      quint16 index;
      getQInt16(byteArrayInput, 8, index);

      quint16 quantity;
      getQInt16(byteArrayInput, 10, quantity);

  //Format output
  //Quantity is too big or less than 1
      if (quantity>holdingRegistersSize || quantity<1){
        formatOutputHeader(3);
        byteArrayOutput.push_back(functionCode+0x80);
        byteArrayOutput.push_back(0x03);
        return true;
      }
      //Illegal data adress
      if (index>=holdingRegistersSize || (index+quantity)>=holdingRegistersSize){
        formatOutputHeader(3);
        byteArrayOutput.push_back(functionCode+0x80);
        byteArrayOutput.push_back(0x02);
        return true;
      }
        //Legal data adress
      formatOutputHeader(quantity*2+3); //2Bytes for each register + 1byte UnitIdentifier +
                                        //1byte FC + 1 byte number of bytes
      byteArrayOutput.push_back(functionCode); //Function Code
      byteArrayOutput.push_back(quint8(quantity*2)); //Number of following bytes

      char temp = 0;
      for(int i=0 ; i<quantity ; i++){
        byteArrayOutput.push_back(temp);
        byteArrayOutput.push_back(temp);
        setQInt16(byteArrayOutput, (i*2+9), holdingRegisters[index+i]);
      }
      return true;
    }

    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS :{
      qDebug()<<"Write Multiple Registers";
      if(dataSize<7)
        return false;
      if(byteArrayInput.size()<15)
        return false;

      quint16 startingAdress;
      getQInt16(byteArrayInput, 8, startingAdress);

      if(startingAdress>=holdingRegistersSize){
        formatOutputHeader(3);
        byteArrayOutput.push_back(byteArrayInput[7]+0x80);
        byteArrayOutput.push_back(0x02);
        return true;
      }

      quint16 quantityOfRegisters;
      getQInt16(byteArrayInput, 10, quantityOfRegisters);

      if(quantityOfRegisters>holdingRegistersSize){
        formatOutputHeader(3);
        byteArrayOutput.push_back(functionCode+0x80);
        byteArrayOutput.push_back(0x03);
        return true;
      }
      if((startingAdress+quantityOfRegisters)>=holdingRegistersSize){
        formatOutputHeader(3);
        byteArrayOutput.push_back(byteArrayInput[7]+0x80);
        byteArrayOutput.push_back(0x02);
        return true;
      }

      quint8 bytesCount;
      getQInt8(byteArrayInput, 12, bytesCount);

      if(bytesCount!=(quantityOfRegisters*2)){
        formatOutputHeader(3);
        byteArrayOutput.push_back(functionCode+0x80);
        byteArrayOutput.push_back(0x03);
        return true;
      }
      if(byteArrayInput.size()!=bytesCount+13)
        return false;

      quint16 value;

      for(int i=0 ; i<quantityOfRegisters ; i++){
        getQInt16(byteArrayInput, (13+2*i), value);
        holdingRegisters[startingAdress+i]=value;
      }

      char temp = 0;
      formatOutputHeader(6);
      byteArrayOutput.push_back(functionCode);
      byteArrayOutput.push_back(temp);
      byteArrayOutput.push_back(temp);
      byteArrayOutput.push_back(temp);
      byteArrayOutput.push_back(temp);
      setQInt16(byteArrayOutput, 8, startingAdress);
      setQInt16(byteArrayOutput, 10, quantityOfRegisters);
      return true;
    }

    case MODBUS_FC_WRITE_SINGLE_REGISTER :{
      qDebug()<<"WriteSingleRegister";

      if(dataSize!=4)
        return false;
      if(byteArrayInput.size()!=12)
        return false;

      quint16 index;
      getQInt16(byteArrayInput, 8, index);

      //Illegal Data Adress
      if(index>=holdingRegistersSize){
        formatOutputHeader(3);
        byteArrayOutput.push_back(byteArrayInput[7]+0x80);
        byteArrayOutput.push_back(0x02);
        return true;
      }
      //Legal Data Adress
      quint16 value;
      getQInt16(byteArrayInput, 10, value);
      holdingRegisters[index]=value;

      formatOutputHeader(6);
      for(int i=7 ; i<12 ; i++)
        byteArrayOutput.push_back(byteArrayInput[i]);

      return true;
    }
    default:{
        formatOutputHeader(3);
        byteArrayOutput.push_back(byteArrayInput[7]+0x80);
        byteArrayOutput.push_back(0x01);
        qDebug()<<"Not supported function";
        return true;
    }
  }
}
bool ModbusServer::getQInt16(QByteArray array, int index, quint16 &output){
  if((index+1)>=array.size())
    return false;
  output = qFromBigEndian<quint16>(reinterpret_cast<uchar*>(array.data()+index));
    return true;
}
bool ModbusServer::setQInt16(QByteArray &array, int index, quint16 input){
  if((index+1)>=array.size())
    return false;
  qToBigEndian<quint16>(input, ((uchar*)array.data())+index);
    return true;
}
bool ModbusServer::getQInt8(QByteArray array, int index, quint8 &output){
  if(index>=array.size())
    return false;
  output = *reinterpret_cast<quint8*>(array.data()+index);
    return true;
}
bool ModbusServer::setQInt8(QByteArray &array, int index, quint8 input){
  if(index>=array.size())
    return false;

  *((quint8*)array.data()+index) = input;
    return true;
}
bool ModbusServer::formatOutputHeader(quint16 lenght){
  byteArrayOutput.clear();
  if(byteArrayInput.size()<8)
    return false;
  for(int i=0 ; i<7 ; i++)
    byteArrayOutput.push_back(byteArrayInput[i]);
  setQInt16(byteArrayOutput, 4, lenght);
  return true;
}
bool ModbusServer::setRegister(int index, quint16 value){
  if(index<0 || index>=holdingRegistersSize)
    return false;
  holdingRegisters[index] = value;
  return true;
}
bool ModbusServer::getRegister(int index, quint16 &value){
  if(index<0 || index>=holdingRegistersSize)
    return false;
  value = holdingRegisters[index];
  return true;
}
