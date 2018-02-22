#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogSettings)
{
  ui->setupUi(this);
}

DialogSettings::~DialogSettings()
{
  delete ui;
}


bool DialogSettings::isLegendChecked()
{
  return ui->checkBoxLegend->isChecked();
}

bool DialogSettings::isDataMarksChecked()
{
  return ui->checkBoxDataMarks->isChecked();

}

int DialogSettings::getPlotTime()
{
  return ui->spinBoxPlotTime->value();

}

int DialogSettings::getYScale()
{
  return ui->spinBoxYAxesScale->value();
}
