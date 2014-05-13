#include "joystickviewrover.h"
#include "ui_joystickviewrover.h"

JoystickViewRover::JoystickViewRover(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::JoystickViewRover)
{
  ui->setupUi(this);
}

JoystickViewRover::~JoystickViewRover()
{
  delete ui;
}

void JoystickViewRover::changeButtonState(int id, bool status){
  switch (id) {
    case 1:
      ui->checkBoxButton1->setChecked(status);
      break;
    case 2:
      ui->checkBoxButton2->setChecked(status);
      break;
    case 3:
      ui->checkBoxButton3->setChecked(status);
      break;
    case 4:
      ui->checkBoxButton4->setChecked(status);
      break;
    case 5:
      ui->checkBoxButton5->setChecked(status);
      break;
    case 6:
      ui->checkBoxButton6->setChecked(status);
      break;
    case 7:
      ui->checkBoxButton7->setChecked(status);
      break;
    case 8:
      ui->checkBoxButton8->setChecked(status);
      break;
    case 9:
      ui->checkBoxButton9->setChecked(status);
      break;
    case 10:
      ui->checkBoxButton10->setChecked(status);
      break;
    case 11:
      ui->checkBoxButton11->setChecked(status);
      break;
    case 12:
      ui->checkBoxButton12->setChecked(status);
      break;
    case 13:
      ui->checkBoxButton13->setChecked(status);
      break;
    case 14:
      ui->checkBoxButton14->setChecked(status);
      break;
    case 15:
      ui->checkBoxButton15->setChecked(status);
      break;
    default:
      break;
    }
}

void JoystickViewRover::changeAxisStat(int id, qint16 value){
  switch (id) {
    case 0:
      ui->progressBarAxis0->setValue(value);
      break;
    case 1:
      ui->progressBarAxis1->setValue(value);
      break;
    case 2:
      ui->progressBarAxis2->setValue(value);
      break;
    case 3:
      ui->progressBarAxis3->setValue(value);
      break;
    case 4:
      ui->progressBarAxis4->setValue(value);
      break;
    case 5:
      ui->progressBarAxis5->setValue(value);
      break;
    default:
      break;
    }
}
