//
// Created by yukari on 2023/7/7.
//

#ifndef COMAUTOTEST_COMAUTOTEST_H
#define COMAUTOTEST_COMAUTOTEST_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <qdatetime.h>
#include <QTime>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class ComAutoTest; }
QT_END_NAMESPACE

struct CmdDelay
{
    QString strCmd = "";
    int iDelaySecond = 1;
};

class ComAutoTest : public QWidget {
Q_OBJECT

public:
    explicit ComAutoTest(QWidget *parent = nullptr);

    ~ComAutoTest() override;

private:
    Ui::ComAutoTest *ui;

    // TimerConfig
    float iAutoSaveLogPeriod = 1; //Minute


    // TimerInit
    QTimer timerCompareTime;
    QTimer timerCMDListTest;
    QTimer timerSaveLog;
    QTimer timerGetData;


    // SerialInit
    QSerialPort workSerial;

    // Serial Communication
    QString strGetDataCmd = "";
    QByteArray strReceiveData;
    int iExpectLen = 0;
    int flagAsciiOrHex = 0; // 0:Ascii,1:Hex

    // WorkFlowControlParameter
    QMap<QString,QList<CmdDelay>> mapCommandList;
    QList<QDateTime> listTimeArrangeTest;
    QList<QString> listTestCMDSheet;
    QList<int> listTestCondition; // 0:未完成 1：进行中 2：已完成
    int indexCurrentTest = 0;

    // CMDChainControlParameter
    QList<CmdDelay> currentCMDChain;
    int idxCMDinChain = 0 ;

    void UIModeChange(int mode);
    void AddLog(QString);
    void ReadCommandList();
    void ReadTestArrangeSheet();

private slots:
    void ScanSerial();
    void OnBtnConnect();
    void OnBtnDisconnect();
    void OnBtnStartTest();
    void OnBtnClearLog();
    void SendGetDataCmd();
    void ReadCom();
    void OnBtnForceStopTest();
    void CompareTime();
    void ContinueSendCmd();
};


#endif //COMAUTOTEST_COMAUTOTEST_H
