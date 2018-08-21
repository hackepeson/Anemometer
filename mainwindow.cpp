#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

void processEventQueueSleep(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec,&loop,SLOT(quit()));
    loop.exec();
}


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_bUpdateValues = true;
  m_bLockLCD = false;
  m_dPlotTimeSec = m_ds.getPlotTime();
  m_dYScale = 0;
  m_bDebugOutput = false;

  m_pElapsedTimer = new QElapsedTimer();
  m_pElapsedTimer->start();



  connect(&m_SerialPort, SIGNAL(readyRead()), SLOT(readyRead()));
  //connect(ui->actionPortUpdatePorts, SIGNAL(triggered(bool)), SLOT(updateComportList()));
  connect(ui->pushButtonFreeze, SIGNAL(clicked(bool)), SLOT(freezeButtonSlot(bool)));


  connect(ui->actionPlot, SIGNAL(triggered(bool)), SLOT(settingsSlot()));

  connect(ui->actionClear_Plot, SIGNAL(triggered(bool)), SLOT(clearPlot()));

  connect(ui->actionResynchronize, SIGNAL(triggered(bool)), SLOT(resynchronize()));

  connect(ui->actionPDF, SIGNAL(triggered(bool)), SLOT(exportToPDF()));

  m_pComPortActionGroup = new QActionGroup(this);
  updateComportList();

  // Add 2 curves
  ui->widgetPlotWind1->addGraph();
  ui->widgetPlotWind1->addGraph();
  ui->widgetPlotWind1->xAxis->setLabel("Time [s]");
  ui->widgetPlotWind1->yAxis->setLabel("Windspeed [m/s]");

  ui->widgetPlotWind1->graph(0)->setPen(QPen(QColor(Qt::red)));
  ui->widgetPlotWind1->graph(0)->setName("Cross");

  ui->widgetPlotWind1->graph(1)->setPen(QPen(QColor(Qt::blue)));
  ui->widgetPlotWind1->graph(1)->setName("Head");


  ui->widgetPlotWind2->addGraph();
  ui->widgetPlotWind2->addGraph();
  ui->widgetPlotWind2->xAxis->setLabel("Time [s]");
  ui->widgetPlotWind2->yAxis->setLabel("Windspeed [m/s]");

  ui->widgetPlotWind2->graph(0)->setPen(QPen(QColor(Qt::red)));
  ui->widgetPlotWind2->graph(0)->setName("Cross");

  ui->widgetPlotWind2->graph(1)->setPen(QPen(QColor(Qt::blue)));
  ui->widgetPlotWind2->graph(1)->setName("Head");


  // Set default index for winds to match protocol writing from BTC
  ui->comboBoxWind2IDSelect->setCurrentIndex(0);
  ui->comboBoxWind1IDSelect->setCurrentIndex(1);


  /*
  ui->widgetPlotWind1->xAxis->setAutoTicks(false);
  ui->widgetPlotWind1->xAxis->setAutoSubTicks(false);
  ui->widgetPlotWind1->xAxis->setAutoTickLabels(false);
  ui->widgetPlotWind1->xAxis->setTickStep(5);
  ui->widgetPlotWind1->xAxis->setSubTickCount(4);
  ui->widgetPlotWind1->xAxis->setAntialiased(true);
  ui->widgetPlotWind1->xAxis->setTickVector(QVector<double>() << 20000 << 15000 << 10000 << 5000 );
  ui->widgetPlotWind1->xAxis->setTickVectorLabels(QVector<QString>() << "20" << "15" << "10" << "5");
*/

  ui->widgetPlotWind1->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  ui->widgetPlotWind1->xAxis->setDateTimeFormat("hh:mm:ss");

  ui->widgetPlotWind2->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  ui->widgetPlotWind2->xAxis->setDateTimeFormat("hh:mm:ss");

  if (m_ds.isLegendChecked())
  {
    ui->widgetPlotWind1->legend->setVisible(true);
    ui->widgetPlotWind2->legend->setVisible(true);
  }
  else
  {
    ui->widgetPlotWind1->legend->setVisible(false);
    ui->widgetPlotWind2->legend->setVisible(false);
  }
  ui->widgetPlotWind1->replot();
  ui->widgetPlotWind2->replot();


}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::readyRead()
{
  char data[1024];
  char crossSpeed[8];
  char headSpeed[8];
  char sensorID;
  char status[2];
  char unit[1];

  //while (m_SerialPort.canReadLine())
  if (m_SerialPort.canReadLine())
  {
    QDateTime timeDT(QDateTime::currentDateTimeUtc());
    float t = (float)timeDT.time().msecsSinceStartOfDay()/1000;

    int lineLength = m_SerialPort.readLine(data,1024);
    data[lineLength] = '\0';
    qDebug() << lineLength << " " << data;
    if (lineLength != 29)
    {
        m_SerialPort.readAll();
        QCoreApplication::processEvents();
        qDebug() << "Line length mismatch";
        return;
    }

    // Search for STX in the string
    char* pch;
    pch = strchr(data,'\2');

    while (pch != NULL)
    {
      char tmpStr[128];
      memcpy(tmpStr, &data[pch-data], 28);
      pch=strchr(pch+1,'\2');
      if (tmpStr[0] == '\2')
      {
        memset(crossSpeed, 0, 8);
        memset(headSpeed, 0, 8);
        memcpy(&sensorID, &tmpStr[1],1);
        memcpy(crossSpeed, &tmpStr[3],7);
        memcpy(headSpeed, &tmpStr[11],7);
        memcpy(unit, &tmpStr[19],1);
        memcpy(status, &tmpStr[21],2);

        char WindID1;
        char WindID2;

        switch (ui->comboBoxWind1IDSelect->currentIndex())
        {
        case 0: WindID1 = 'A';break;
        case 1: WindID1 = 'B';break;
        case 2: WindID1 = 'C';break;
        }

        switch (ui->comboBoxWind2IDSelect->currentIndex())
        {
        case 0: WindID2 = 'A';break;
        case 1: WindID2 = 'B';break;
        case 2: WindID2 = 'C';break;
        }

//        m_pElapsedTimerValueD1 = (float)m_pElapsedTimerValue;
//        m_pElapsedTimerValue = (float)m_pElapsedTimer->elapsed();
//        m_pElapsedTimer->restart();
//
//        ui->lcdNumberMessageJitter->display( (int)m_pElapsedTimerValue);
//        m_updateTimeValue = !m_updateTimeValue;
//        qDebug() << m_updateTimeValue;
//        if (m_updateTimeValue)
//        {
//          ui->progressBarPhaseCom->setValue(100.0*m_pElapsedTimerValue/(m_pElapsedTimerValue+m_pElapsedTimerValueD1));
//        }


        if (m_bUpdateValues)
        {
          if (sensorID == WindID1)
          {
            if (!m_bLockLCD)
            {
              float cs = QString(crossSpeed).toFloat();
              float hs = -QString(headSpeed).toFloat();

              ui->lcdNumberHeadWind1->display(QString::number(cs,'f',1));
              ui->lcdNumberCrossWind1->display(QString::number(hs,'f',1));
/*
              ui->lcdNumberHeadWind1->display(QString(crossSpeed).toFloat());
              ui->lcdNumberCrossWind1->display(-QString(headSpeed).toFloat());

*/
            }

            QString ID(sensorID);
            ui->widgetPlotWind1->graph(0)->removeDataBefore(t-m_dPlotTimeSec);
            ui->widgetPlotWind1->graph(1)->removeDataBefore(t-m_dPlotTimeSec);
            ui->labelSensorWind1->setText(ID);
            ui->widgetPlotWind1->graph(0)->addData((float)t, -QString(headSpeed).toFloat());
            ui->widgetPlotWind1->graph(1)->addData((float)t, QString(crossSpeed).toFloat());
            if (m_dYScale == 0)
            {
              ui->widgetPlotWind1->rescaleAxes();
            }
            else
            {
              ui->widgetPlotWind1->yAxis->setRange(-m_dYScale,m_dYScale);
            }
            ui->widgetPlotWind1->xAxis->setRange((float)t,m_dPlotTimeSec, Qt::AlignRight);
            ui->widgetPlotWind1->setTitle(ID);
            ui->widgetPlotWind1->replot();
          }
          if (sensorID == WindID2)
          {
            if (!m_bLockLCD)
            {
              float cs = QString(crossSpeed).toFloat();
              float hs = -QString(headSpeed).toFloat();

              ui->lcdNumberHeadWind2->display(QString::number(cs,'f',1));
              ui->lcdNumberCrossWind2->display(QString::number(hs,'f',1));
/*
              ui->lcdNumberHeadWind2->display(QString(crossSpeed).toFloat());
              ui->lcdNumberCrossWind2->display(-QString(headSpeed).toFloat());
*/
            }

            QString ID(sensorID);
            ui->widgetPlotWind2->graph(0)->removeDataBefore(t-m_dPlotTimeSec);
            ui->widgetPlotWind2->graph(1)->removeDataBefore(t-m_dPlotTimeSec);
            ui->labelSensorWind2->setText(ID);
            ui->widgetPlotWind2->graph(0)->addData((float)t, -QString(headSpeed).toFloat());
            ui->widgetPlotWind2->graph(1)->addData((float)t, QString(crossSpeed).toFloat());

            if (m_dYScale == 0)
            {
              ui->widgetPlotWind2->rescaleAxes();
            }
            else
            {
              ui->widgetPlotWind2->yAxis->setRange(-m_dYScale,m_dYScale);
            }
            ui->widgetPlotWind2->xAxis->setRange((float)t,m_dPlotTimeSec, Qt::AlignRight);
            ui->widgetPlotWind2->setTitle(ID);
            ui->widgetPlotWind2->replot();
          }
        }
        if (m_bDebugOutput)
        {
          QString text = QString(data).simplified();
          ui->textEditSerialInput->append(text);
        }
      }
      else
      {
        m_SerialPort.readAll();
        qDebug() << "Error read";
      }
    }
  }
}

void MainWindow::updateComportList()
{
  // Serial port Init
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  QSerialPortInfo port;

  ui->menuPort->actions().clear();
  ui->menuPort->clear();
  m_pComPortActionGroup->actions().clear();

  QAction* newAction = new QAction("Update list", this);
  newAction->setObjectName("Update");
  m_pComPortActionGroup ->addAction(newAction);
  ui->menuPort->addAction(newAction);
  connect(newAction, SIGNAL(triggered(bool)), SLOT(comPortOpen(bool)));

  QAction* disconnectAction = new QAction("Disconnect", this);
  disconnectAction->setObjectName("Disconnect");
  m_pComPortActionGroup->addAction(disconnectAction);
  ui->menuPort->addAction(disconnectAction);
  connect(disconnectAction, SIGNAL(triggered(bool)), SLOT(disConnectComPort()));


  foreach(port, ports)
  {
    newAction = new QAction(port.portName(), this);
    newAction->setObjectName(port.portName());
    newAction->setCheckable(true);
    m_pComPortActionGroup ->addAction(newAction);
    ui->menuPort->addAction(newAction);
    connect(newAction, SIGNAL(triggered(bool)), SLOT(comPortOpen(bool)));
  }
}

void MainWindow::comPortOpen(bool ctrl)
{
  if (m_SerialPort.isOpen())
  {
    m_SerialPort.close();
    qDebug() << "Closing port" << m_SerialPort.portName();
  }
  qDebug() << sender()->objectName();

  if (ctrl)
  {
    //m_pComPort->setPortName(ui->comboBoxComports->currentText());
    m_SerialPort.setPortName(sender()->objectName());

    if (m_SerialPort.open(QIODevice::ReadWrite))
    {
      qDebug() << "Opening port" << m_SerialPort.portName();
      m_SerialPort.flush();
      m_SerialPort.setBaudRate(38400);
      m_SerialPort.setFlowControl(QSerialPort::NoFlowControl);
      m_SerialPort.setDataBits(QSerialPort::Data8);
      m_SerialPort.setStopBits(QSerialPort::OneStop);
      m_SerialPort.setParity(QSerialPort::NoParity);

      ui->statusBar->showMessage("Comport " + sender()->objectName() + " opened", 3000);
      setWindowTitle("Anemometer (" + sender()->objectName() + ")");
      clearPlot();
    }
    else
    {
      QMessageBox msgBox;
      msgBox.setStandardButtons(0);
      msgBox.addButton(QMessageBox::Ok);
      msgBox.setWindowTitle("ERROR");
      msgBox.setText("Failed to open COM port");
      msgBox.exec();
      setWindowTitle("Anemometer");
      m_pComPortActionGroup->checkedAction()->setChecked(false);
    }
  }
}

void MainWindow::freezeButtonSlot(bool ctrl)
{
  if (ctrl)
  {
    QDateTime timeDT(QDateTime::currentDateTimeUtc());
    m_fT0 = (float)timeDT.time().msecsSinceStartOfDay()/1000;
    m_bLockLCD = true;
    addT0Marker();
    ui->pushButtonFreeze->setDisabled(true);
    ui->pushButtonFreeze->setText("Run");
    processEventQueueSleep(5000);
    m_bUpdateValues = false;
    ui->pushButtonFreeze->setDisabled(false);

  }
  else
  {
    m_bUpdateValues = true;
    ui->pushButtonFreeze->setText("Stop");
    removeT0Marker();
    m_bLockLCD = false;
  }
}


void MainWindow::disConnectComPort()
{
  qDebug() << "Close serialport";
  // Close serial port
  if (m_SerialPort.isOpen())
  {
    m_SerialPort.close();
  }

  // Unmark Com port selection
  if (m_pComPortActionGroup->checkedAction())
  {
    m_pComPortActionGroup->checkedAction()->setChecked(false);
  }

  // Change window title
  setWindowTitle("Anemometer");
}

void MainWindow::settingsSlot()
{
  qDebug() << "Open settings dialog";

  if (m_ds.exec())
  {
    QCP::ScatterStyle ss;
    if (m_ds.isDataMarksChecked())
    {
      ss = QCP::ScatterStyle(QCP::ssCircle);
    }
    else
    {
      ss = QCP::ScatterStyle(QCP::ssNone);
    }
    ui->widgetPlotWind1->graph(0)->setScatterStyle(ss);
    ui->widgetPlotWind1->graph(1)->setScatterStyle(ss);
    ui->widgetPlotWind2->graph(0)->setScatterStyle(ss);
    ui->widgetPlotWind2->graph(1)->setScatterStyle(ss);
    m_dPlotTimeSec = m_ds.getPlotTime();
    m_dYScale = m_ds.getYScale();

    if (m_ds.isLegendChecked())
    {
      ui->widgetPlotWind1->legend->setVisible(true);
      ui->widgetPlotWind2->legend->setVisible(true);
    }
    else
    {
      ui->widgetPlotWind1->legend->setVisible(false);
      ui->widgetPlotWind2->legend->setVisible(false);
    }
  }
  else
  {
    qDebug() << "Settings dialog discarded";
  }
}

void MainWindow::clearPlot()
{

  ui->widgetPlotWind1->graph(0)->clearData();
  ui->widgetPlotWind1->graph(1)->clearData();

  ui->widgetPlotWind2->graph(0)->clearData();
  ui->widgetPlotWind2->graph(1)->clearData();

}

void MainWindow::resynchronize()
{
  qDebug() << "Try resynchronize";
  for (int i = 0; i < 100; i++)
  {
    m_SerialPort.write("*");
    processEventQueueSleep(10);
  }

  processEventQueueSleep(1000);

  for (int i = 0; i < 100; i++)
  {
    m_SerialPort.write("Q\r\n");
    processEventQueueSleep(10);
  }
}


void MainWindow::addT0Marker()
{

  // Add to to Wind1 plot
  // add the text label at the top:
  m_textLabelPlot1 = new QCPItemText(ui->widgetPlotWind1);
  m_textLabelPlot1->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
  m_textLabelPlot1->position->setType(QCPItemPosition::ptAxisRectRatio);
  m_textLabelPlot1->position->setCoords(0.5, 0); // place position at center/top of axis rect
  m_textLabelPlot1->setText("Stop time");
  m_textLabelPlot1->setFont(QFont(font().family(), 16)); // make font a bit larger
  m_textLabelPlot1->setPen(QPen(Qt::black)); // show black border around text

  // add the arrow:
  m_arrowPlot1 = new QCPItemLine(ui->widgetPlotWind1);
  m_arrowPlot1->start->setParentAnchor(m_textLabelPlot1->bottom);
  m_arrowPlot1->end->setCoords(m_fT0, 0); // point to (4, 1.6) in x-y-plot coordinates
  m_arrowPlot1->setHead(QCPLineEnding::esSpikeArrow);
  ui->widgetPlotWind1->replot();


  // Add to to Wind2 plot
  // add the text label at the top:
  m_textLabelPlot2 = new QCPItemText(ui->widgetPlotWind2);
  m_textLabelPlot2->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
  m_textLabelPlot2->position->setType(QCPItemPosition::ptAxisRectRatio);
  m_textLabelPlot2->position->setCoords(0.5, 0); // place position at center/top of axis rect
  m_textLabelPlot2->setText("Stop time");
  m_textLabelPlot2->setFont(QFont(font().family(), 16)); // make font a bit larger
  m_textLabelPlot2->setPen(QPen(Qt::black)); // show black border around text

  // add the arrow:
  m_arrowPlot2 = new QCPItemLine(ui->widgetPlotWind2);
  m_arrowPlot2->start->setParentAnchor(m_textLabelPlot2->bottom);
  m_arrowPlot2->end->setCoords(m_fT0, 0); // point to (4, 1.6) in x-y-plot coordinates
  m_arrowPlot2->setHead(QCPLineEnding::esSpikeArrow);

  ui->widgetPlotWind2->replot();

}

void MainWindow::removeT0Marker()
{
  ui->widgetPlotWind1->removeItem(m_textLabelPlot1);
  ui->widgetPlotWind1->removeItem(m_arrowPlot1);
  delete m_textLabelPlot1;
  delete m_arrowPlot1;
  ui->widgetPlotWind1->replot();

  ui->widgetPlotWind2->removeItem(m_textLabelPlot2);
  ui->widgetPlotWind2->removeItem(m_arrowPlot2);
  delete m_textLabelPlot2;
  delete m_arrowPlot2;
  ui->widgetPlotWind2->replot();
}

void MainWindow::exportToPDF()
{
  static QString pdfFileName;

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save to PDF"), pdfFileName, tr("PDF Files (*.pdf)"));

  QString plotID1 = ui->widgetPlotWind1->title();
  QString plotID2 = ui->widgetPlotWind2->title();


  if (!fileName.isEmpty())
  {

    QStringList qsl =  fileName.split(".pdf");


    QString fileName1 = qsl[0];
    fileName1.append(plotID1);
    fileName1.append(".pdf");
    ui->widgetPlotWind1->savePdf(fileName1);

    QString fileName2 = qsl[0];
    fileName2.append(plotID2);
    fileName2.append(".pdf");
    ui->widgetPlotWind2->savePdf(fileName2);

  }

}


void MainWindow::resetCommunication()
{

    disConnectComPort();
    QCoreApplication::processEvents();
    comPortOpen(true);
    QCoreApplication::processEvents();
}
