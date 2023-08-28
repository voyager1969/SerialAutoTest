//
// Created by yukari on 2023/7/7.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ComAutoTest.h" resolved

#include "ComAutoTest.h"
#include "ui_ComAutoTest.h"
#include <QMessageBox>
#include "LXsheet.h"
#include <QDir>


ComAutoTest::ComAutoTest(QWidget * parent)
:

QWidget (parent), ui(new Ui::ComAutoTest) {
    ui->setupUi(this);
    timerSaveLog.start(1000 * 60 * iAutoSaveLogPeriod);
    ui->cbxBaudRate->setCurrentIndex(3);

    ui->getDataCmd->setPlainText("iout1?\n"
                                 "vout1?");
    ReadCommandList();
    ReadTestArrangeSheet();

    ScanSerial();
    connect(ui->btnFreshSerial, SIGNAL(clicked()), this, SLOT(ScanSerial()));
    connect(ui->btnConnectSerial, SIGNAL(clicked()), this, SLOT(OnBtnConnect()));
    connect(ui->btnDisconnectSerial, SIGNAL(clicked()), this, SLOT(OnBtnDisconnect()));


    connect(ui->btnStartTest, SIGNAL(clicked()), this, SLOT(OnBtnStartTest()));
    connect(ui->btnClearLog, SIGNAL(clicked()), this, SLOT(OnBtnClearLog()));


    connect(&workSerial, SIGNAL(readyRead()), this, SLOT(ReadCom()));


    connect(&timerSaveLog, SIGNAL(timeout()), this, SLOT(OnBtnClearLog()));
    connect(ui->btnForceStopTest, SIGNAL(clicked()), this, SLOT(OnBtnForceStopTest()));
//    connect(&timerCutScreen, SIGNAL(timeout()), this, SLOT(CutScreen()));

    connect(&timerGetData, SIGNAL(timeout()), this, SLOT(SendGetDataCmd()));
    connect(&timerCompareTime, SIGNAL(timeout()), this, SLOT(CompareTime()));
    connect(&timerCMDListTest, SIGNAL(timeout()), this, SLOT(ContinueSendCmd()));

}

ComAutoTest::~ComAutoTest() {
    delete ui;
}

void ComAutoTest::ScanSerial() {
    ui->cbxSerialName->clear();
    foreach(
    const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QSerialPort serial;
        serial.setPort(info);
        ui->cbxSerialName->addItem(serial.portName());
    }
}

void ComAutoTest::OnBtnConnect() {
    workSerial.setPortName(ui->cbxSerialName->currentText());
    workSerial.setBaudRate(ui->cbxBaudRate->currentText().toInt());

    QSerialPort::DataBits databits = QSerialPort::Data8;
    switch (ui->cbxDataBits->currentIndex()) {
        case 0: {
            databits = QSerialPort::Data8;
            break;
        }
        case 1: {
            databits = QSerialPort::Data7;
            break;
        }
        case 2: {
            databits = QSerialPort::Data6;
            break;
        }
        case 3: {
            databits = QSerialPort::Data5;
            break;
        }
        default: {
            break;
        }
    }
    workSerial.setDataBits(databits);

    QSerialPort::Parity parity = QSerialPort::OddParity;
    switch (ui->cbxParity->currentIndex()) {
        case 0: {
            parity = QSerialPort::NoParity;
            break;
        }
        case 1: {
            parity = QSerialPort::EvenParity;
            break;
        }
        case 2: {
            parity = QSerialPort::OddParity;
            break;
        }
        default: {
            break;
        }
    }
    workSerial.setParity(parity);


    QSerialPort::StopBits stopbits = QSerialPort::OneStop;
    switch (ui->cbxStopBits->currentIndex()) {
        case 0: {
            stopbits = QSerialPort::OneStop;
            break;
        }
        case 1: {
            stopbits = QSerialPort::TwoStop;
            break;
        }
        case 2: {
            stopbits = QSerialPort::OneAndHalfStop;
            break;
        }
        default: {
            break;
        }
    }
    workSerial.setStopBits(stopbits);

    QSerialPort::FlowControl flowcontrol = QSerialPort::NoFlowControl;
    switch (ui->cbxFlowControl->currentIndex()) {
        case 0: {
            flowcontrol = QSerialPort::NoFlowControl;
            break;
        }
        case 1: {
            flowcontrol = QSerialPort::SoftwareControl;
            break;
        }
        case 2: {
            flowcontrol = QSerialPort::HardwareControl;
            break;
        }
        default: {
            break;
        }
    }
    workSerial.setFlowControl(flowcontrol);


    flagAsciiOrHex = ui->cbxEncode->currentIndex();

    int ret = workSerial.open(QIODevice::ReadWrite);    //Open Serial
    if (!ret) { // Open Failed
        QMessageBox msgNewDatPic(QMessageBox::Information, "错误", "该串口被占用，打开失败");// 标题与内容
        msgNewDatPic.setStandardButtons(QMessageBox::Ok);// 按钮
        msgNewDatPic.setDefaultButton(QMessageBox::Ok);// 默认选中按钮
        msgNewDatPic.setButtonText(QMessageBox::Ok, QString("确 定"));// 按钮显示文字修改
        msgNewDatPic.exec();// 返回值
        return;
    } else {// Open Succeed
        UIModeChange(1);
        AddLog("串口开启");
        if (ui->groupManGetData->isChecked() && ui->getDataCmd->toPlainText() != "") {
            strGetDataCmd = ui->getDataCmd->toPlainText();
            iExpectLen = ui->spbExpectLen->value();
            timerGetData.start(ui->spbGetDataInter->value() * 1000);
        }
    }


}

void ComAutoTest::OnBtnDisconnect() {
    workSerial.close();
    timerGetData.stop();
    UIModeChange(0);
    AddLog("串口关闭");
}

void ComAutoTest::OnBtnStartTest() {

    AddLog("启动测试。");
    UIModeChange(2);
    timerCompareTime.start(2000);

}

void ComAutoTest::OnBtnClearLog() {
    QDir dir("./");
    if (!dir.exists("Log")) {
        dir.mkdir("Log");
    }

    QString qstrFilePath = "./Log/RSLog" + QDateTime::currentDateTime().toString("yyyyMMddhh") + ".csv";

    QFile file(qstrFilePath);

    if (!QFile::exists(qstrFilePath)) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            file.close();
        }
        QTextStream out(&file);
        QStringList logTitle;
        // 写入文件表头
        logTitle
                << "Time"
                << "Action";
        out << logTitle[0] << ",";
        out << logTitle[1] << "\n";
    }
    // 再写内容
    if (!file.isOpen()) {
        file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    }
    QTextStream out(&file);
    out << ui->txtLog->toPlainText() << "\n";
    file.close();
    ui->txtLog->clear();
}

void ComAutoTest::ReadCom() {

    strReceiveData = strReceiveData + workSerial.readAll();


    if(strReceiveData.length()<iExpectLen)
    {
        return;
    }
    if (flagAsciiOrHex == 1) {
        AddLog(strReceiveData.toHex().toUpper());
        strReceiveData = "";
    } else if (flagAsciiOrHex == 0) {
        QString tmpstr = strReceiveData;
        tmpstr.replace("\n", "");
        tmpstr.replace("\r", "");
        AddLog(tmpstr.toUpper());
        strReceiveData = "";
    }



}


void ComAutoTest::OnBtnForceStopTest() {

    AddLog("强制停止");
    timerCompareTime.stop();
    timerCMDListTest.stop();
    ui->prgTestProgress->setValue(0);
    UIModeChange(1);
}


void ComAutoTest::UIModeChange(int mode) {
    switch (mode) {
        case 0:
            // not Connected
            ui->cbxSerialName->setEnabled(1);
            ui->btnFreshSerial->setEnabled(1);
            ui->btnConnectSerial->setEnabled(1);
            ui->btnDisconnectSerial->setEnabled(0);
            ui->btnStartTest->setEnabled(0);
            ui->btnForceStopTest->setEnabled(0);
            ui->groupManGetData->setEnabled(1);
            ui->spbGetDataInter->setEnabled(1);
            ui->spbExpectLen->setEnabled(1);
            ui->getDataCmd->setEnabled(1);
            ui->btnStartTest->setEnabled(0);

            ui->cbxBaudRate->setEnabled(1);
            ui->cbxDataBits->setEnabled(1);
            ui->cbxFlowControl->setEnabled(1);
            ui->cbxParity->setEnabled(1);
            ui->cbxDataBits->setEnabled(1);
            ui->cbxStopBits->setEnabled(1);
            ui->cbxEncode->setEnabled(1);
            break;
        case 1:
            // Connected ,not in Test
            ui->cbxSerialName->setEnabled(0);
            ui->btnFreshSerial->setEnabled(0);
            ui->btnConnectSerial->setEnabled(0);
            ui->btnDisconnectSerial->setEnabled(1);
            ui->btnStartTest->setEnabled(1);
            ui->btnForceStopTest->setEnabled(0);
            ui->groupManGetData->setEnabled(0);
            ui->spbGetDataInter->setEnabled(0);
            ui->spbExpectLen->setEnabled(0);
            ui->getDataCmd->setEnabled(0);

            ui->cbxBaudRate->setEnabled(0);
            ui->cbxDataBits->setEnabled(0);
            ui->cbxFlowControl->setEnabled(0);
            ui->cbxParity->setEnabled(0);
            ui->cbxDataBits->setEnabled(0);
            ui->cbxStopBits->setEnabled(0);
            ui->cbxEncode->setEnabled(0);

            break;
        case 2:
            // Connected , in Test
            ui->cbxSerialName->setEnabled(0);
            ui->btnFreshSerial->setEnabled(0);
            ui->btnConnectSerial->setEnabled(0);
            ui->btnDisconnectSerial->setEnabled(0);
            ui->btnStartTest->setEnabled(0);
            ui->btnForceStopTest->setEnabled(1);
            break;
        default:
            break;
    }

}

void ComAutoTest::AddLog(QString log) {
    ui->txtLog->append(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz") + "," + log);
}

void ComAutoTest::ReadCommandList() {

    QString cmdPath = "./CommandList";
    QDir objDir(cmdPath);
    QStringList listFilenames = objDir.entryList(QDir
                                                 ::AllEntries | QDir::NoDotAndDotDot);
    for (int i = 0; i < listFilenames.size(); ++i) {
        QString filepath = cmdPath + "/" + listFilenames[i];
        LX::Sheet objSheetCommandList;
        objSheetCommandList.ReadCsv(filepath);
        QList < CmdDelay > objListCmdDelay;
        for (int iData = 0; iData < objSheetCommandList.data.size(); ++iData) {
            CmdDelay objCmdDelay;
            objCmdDelay.strCmd = objSheetCommandList.data[iData][0];
            objCmdDelay.iDelaySecond = objSheetCommandList.data[iData][1].toInt();
            objListCmdDelay.append(objCmdDelay);
        }
        mapCommandList[listFilenames[i]] = objListCmdDelay;
    }
}

void ComAutoTest::ReadTestArrangeSheet() {
    LX::Sheet objTestArrangeSheet;
    QString filepathTestArrangeSheet = "./TestArrange.csv";
    objTestArrangeSheet.ReadCsv(filepathTestArrangeSheet);
    for (int iData = 0; iData < objTestArrangeSheet.data.size(); ++iData) {
        listTimeArrangeTest.append(QDateTime::fromString(objTestArrangeSheet.data[iData][0], "yyyy-MM-dd HH:mm:ss"));
        listTestCMDSheet.append(objTestArrangeSheet.data[iData][1]);
        listTestCondition.append(0);
    }

    // 更新ui

    //表格模型
    ui->tableTestArrange->setColumnCount(2);//设置列数
    ui->tableTestArrange->setRowCount(listTimeArrangeTest.size());
    QStringList HorizonHead;
    HorizonHead << ("时间") << ("指令链");
    ui->tableTestArrange->setHorizontalHeaderLabels(HorizonHead);
    ui->tableTestArrange->setEditTriggers(QAbstractItemView::NoEditTriggers); // 使其无法修改
    //输出IO表属性
    ui->tableTestArrange->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);//字体居中
    ui->tableTestArrange->verticalHeader()->setDefaultSectionSize(16);//固定行高度为6
    ui->tableTestArrange->setColumnWidth(0, 130);
    ui->tableTestArrange->horizontalHeader()->setStretchLastSection(true); // 最后一列拉至填满表格
    ui->tableTestArrange->horizontalHeader()->setStyleSheet("QHeaderView::section{background:whitesmoke;}"); // 设置颜色

    // 增加条目
    for (int iData = 0; iData < listTimeArrangeTest.size(); ++iData) {
        ui->tableTestArrange->setItem(iData, 0, new QTableWidgetItem(listTimeArrangeTest[iData].toString(Qt::ISODate)));
        ui->tableTestArrange->setItem(iData, 1, new QTableWidgetItem(listTestCMDSheet[iData]));
    }

    // 更新时间状态
    for (int i = 0; i < listTimeArrangeTest.size(); ++i) {
        if (QDateTime::currentDateTime() > listTimeArrangeTest[i]) {
            listTestCondition[i] = 2;
            ui->tableTestArrange->item(i, 0)->setTextColor(Qt::blue);
            ui->tableTestArrange->item(i, 1)->setTextColor(Qt::blue);
        } else {
            indexCurrentTest = i;
            break;
        }
    }
}

void ComAutoTest::SendGetDataCmd() {

    if(flagAsciiOrHex == 0)
    {
        workSerial.write((strGetDataCmd + "\r\n").toStdString().c_str());
    }
    else if(flagAsciiOrHex == 1)
    {
        QByteArray sendBuf = QByteArray::fromHex(strGetDataCmd.toLatin1());
        workSerial.write(sendBuf);
    }

}

void ComAutoTest::CompareTime() {
    // 当前时间大于
    if (QDateTime::currentDateTime() >= listTimeArrangeTest[indexCurrentTest]) {
        if (listTestCondition[indexCurrentTest] == 0) {
            timerCMDListTest.start(1);
            ui->tableTestArrange->item(indexCurrentTest, 0)->setTextColor(Qt::red);
            ui->tableTestArrange->item(indexCurrentTest, 1)->setTextColor(Qt::red);
            listTestCondition[indexCurrentTest] = 1;
            currentCMDChain = mapCommandList[listTestCMDSheet[indexCurrentTest]];
        }

    }

}

void ComAutoTest::ContinueSendCmd() {
    timerCMDListTest.stop();
    QString cmd = currentCMDChain[idxCMDinChain].strCmd;
    if (flagAsciiOrHex == 1) {
        QByteArray sendBuf = QByteArray::fromHex(cmd.toLatin1());
        workSerial.write(sendBuf);
    } else if(flagAsciiOrHex == 0){
        workSerial.write((cmd + "\r\n").toStdString().c_str());
    }

    AddLog("已发送指令序号：" + QString::number(idxCMDinChain));
    ui->prgTestProgress->setValue((idxCMDinChain + 1) * 100 / currentCMDChain.size());
    int itimeDelay = currentCMDChain[idxCMDinChain].iDelaySecond;
    idxCMDinChain++;
    if (idxCMDinChain >= currentCMDChain.size()) {
        // 完成指令链更新状态
        idxCMDinChain = 0;
        listTestCondition[indexCurrentTest] = 2;
        ui->tableTestArrange->item(indexCurrentTest, 0)->setTextColor(Qt::blue);
        ui->tableTestArrange->item(indexCurrentTest, 1)->setTextColor(Qt::blue);
        indexCurrentTest++;
        if (indexCurrentTest >= listTimeArrangeTest.length()) {
            timerCompareTime.stop();
            AddLog("本次指令链完成。");
        }
        return;
    } else {
        timerCMDListTest.start(itimeDelay * 1000);
    }
}
