#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SettingsWindow)
{
  ui->setupUi(this);
  //Fill comboBoxes with joypad names


  onnect(this->ui->pushButtonConnectRover,SIGNAL(clicked()),this,SLOT(connectButtonPressedRover()));
  connect(this->ui->pushButtonDisconnectRover,SIGNAL(clicked()),this,SLOT(disconnectButtonPressedRover()));

  connect(this->ui->pushButtonConnectManipulator,SIGNAL(clicked()),this,SLOT(connectButtonPressedManipulator()));
  connect(this->ui->pushButtonDisconnectManipulator,SIGNAL(clicked()),this,SLOT(disconnectButtonPressedManipulator()));
}

SettingsWindow::~SettingsWindow()
{
  delete ui;
}
