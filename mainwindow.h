#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QActionGroup>
#include "dialogsettings.h"
#include <QTime>
#include <QElapsedTimer>
#include "qcustomplot.h"

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
  bool m_bLockLCD;
  DialogSettings m_ds;
  int m_dPlotTimeSec;
  int m_dYScale;
  bool m_bPollMessage;

  bool m_bDebugOutput;

  float m_fT0;

  QCPItemText* m_textLabelPlot1;
  QCPItemLine* m_arrowPlot1;

  QCPItemText* m_textLabelPlot2;
  QCPItemLine* m_arrowPlot2;

  QElapsedTimer* m_pElapsedTimer;
  bool m_updateTimeValue;
  float m_pElapsedTimerValueD1;
  float m_pElapsedTimerValue;

  QTimer* m_pPollTimer;

private slots:
  void readyRead();
  void updateComportList();
  void comPortOpen(bool ctrl);
  void disConnectComPort();
  void settingsSlot();
  void freezeButtonSlot(bool ctrl);
  void clearPlot();
  void resynchronize();
  void addT0Marker();
  void removeT0Marker();
  void exportToPDF();
  void resetCommunication();
  void pollForMessage();

};

#endif // MAINWINDOW_H


