#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QActionGroup>
#include "dialogsettings.h"
#include <QTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  QSerialPort m_SerialPort;
  QActionGroup* m_pComPortActionGroup;

  bool m_bUpdateValues;
  DialogSettings m_ds;
  int m_dPlotTimeSec;
  QTime m_startTime;


private slots:
  void readyRead();
  void updateComportList();
  void comPortOpen(bool ctrl);
  void disConnectComPort();
  void settingsSlot();
  void freezeButtonSlot(bool ctrl);
  void clearPlot();

};

#endif // MAINWINDOW_H
