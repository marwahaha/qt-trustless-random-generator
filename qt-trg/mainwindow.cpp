#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QListWidget>
#include <QPushButton>
#include <qcombobox.h>
#include <qdesktopwidget.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <QMessageBox>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/value.h>
#include <boost/functional/hash.hpp>
#include "web.h"
#include <QLineEdit>
#include <qdesktopservices.h>
#include <stdlib.h>

QListWidget* dataList = nullptr;
QPushButton* btnGenerate = nullptr;
QComboBox* cboBlockchains = nullptr;
QLineEdit* leBlockchain = nullptr;
QLineEdit* leHash = nullptr;
QLineEdit* leTimestamp = nullptr;
QLineEdit* leResult = nullptr;
QPushButton* btnBrowse = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(WIDTH, HEIGHT);
    this->setWindowTitle(tr("Trustless Random Generator"));
    QDesktopWidget* desktop = QApplication::desktop();
    this->move(desktop->width()/2 - WIDTH/2, desktop->height()/2 - HEIGHT/2);

    QMenu* menuFile = menuBar()->addMenu("&File");
    QAction* actionQuit = new QAction("&Quit");
    QAction* actionImportFile = new QAction("&Import .csv");

    menuFile->addAction(actionImportFile);
    menuFile->addAction(actionQuit);

    connect(actionImportFile, SIGNAL(triggered()), this, SLOT(import()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    QGridLayout* gridLayout = new QGridLayout(centralWidget());
    const QRect layoutGeometry(0, 0, WIDTH, HEIGHT);
    gridLayout->setGeometry(layoutGeometry);

    QLabel* lblBlockchains = new QLabel(this);
    lblBlockchains->setText("Blockchain : ");

    cboBlockchains = new QComboBox(this);
    QStringList qBlockchainsList = (QStringList() << "BTC" << "ETH" << "LTC");
    cboBlockchains->addItems(qBlockchainsList);

    gridLayout->addWidget(lblBlockchains, 0, 0, 1, 1);
    gridLayout->addWidget(cboBlockchains, 0, 1, 1, 1);

    dataList = new QListWidget(this);
    gridLayout->addWidget(dataList, 1, 0, 8, 8);

    QLabel* lblBlockchainUsed = new QLabel(this);
    lblBlockchainUsed->setText(tr("Blockchain used : "));
    gridLayout->addWidget(lblBlockchainUsed, 1, 9, 1, 1);

    leBlockchain = new QLineEdit(this);
    leBlockchain->setEnabled(false);
    gridLayout->addWidget(leBlockchain, 1, 10, 1, 1);

    QLabel* lblHashUsed = new QLabel(this);
    lblHashUsed->setText(tr("Hash used : "));
    gridLayout->addWidget(lblHashUsed, 2, 9, 1, 1);

    leHash = new QLineEdit(this);
    leHash->setEnabled(false);
    gridLayout->addWidget(leHash, 2, 10, 1, 1);

    QLabel* lblTimestampHash = new QLabel(this);
    lblTimestampHash->setText(tr("Hash timestamp : "));
    gridLayout->addWidget(lblTimestampHash, 3, 9, 1, 1);

    leTimestamp = new QLineEdit(this);
    leTimestamp->setEnabled(false);
    gridLayout->addWidget(leTimestamp, 3, 10, 1, 1);

    QLabel* lblResult = new QLabel(this);
    lblResult->setText(tr("Result : "));
    gridLayout->addWidget(lblResult, 4, 9, 1, 1);

    leResult = new QLineEdit(this);
    leResult->setEnabled(false);
    gridLayout->addWidget(leResult, 4, 10, 1, 1);

    btnBrowse = new QPushButton(this);
    btnBrowse->setText(tr("Browse on explorer"));
    btnBrowse->setEnabled(false);
    gridLayout->addWidget(btnBrowse, 5, 9, 1, 2);
    connect(btnBrowse, SIGNAL(clicked()), this, SLOT(browse()));

    btnGenerate = new QPushButton(this);
    btnGenerate->setText("Generate");
    btnGenerate->setEnabled(false);
    connect(btnGenerate, SIGNAL(clicked()), this, SLOT(generate()));
    gridLayout->addWidget(btnGenerate, 10, 0, 1, 8);
}

void MainWindow::import()
{
    dataList->clear();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),".",tr("CSV Files (*.csv)"));
    std::ifstream f(fileName.toStdString());
    std::string data((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
    QString qData(data.c_str());
    QRegExp rx("(\\,)");
    dataList->addItems(qData.split(rx));
    btnGenerate->setEnabled(dataList->count() > 1);    
}

void MainWindow::generate()
{
    Json::Value root = Web::getJsonFromAPI(("https://api.blockcypher.com/v1/" + cboBlockchains->currentText().toLower().toStdString() + "/main").c_str());

    std::string hash = root.get("hash", "").asString();
    std::string timestamp = root.get("time", "").asString();

    boost::hash<std::string> string_hash;

    int seed = string_hash(hash);
    srand(seed);
    int resIndex = rand() % dataList->count();

    dataList->setCurrentRow(resIndex);
    leBlockchain->setText(cboBlockchains->currentText());
    leHash->setText(QString(hash.c_str()));
    leTimestamp->setText(QString(timestamp.c_str()));
    leResult->setText(dataList->item(resIndex)->text());
    btnBrowse->setEnabled(true);
}

void MainWindow::browse()
{
    QDesktopServices::openUrl(QUrl(QString(("https://live.blockcypher.com/" + cboBlockchains->currentText().toLower().toStdString() + "/block/" + leHash->text().toStdString()).c_str())));
}

MainWindow::~MainWindow()
{
    delete btnBrowse;
    delete leBlockchain;
    delete leHash;
    delete cboBlockchains;
    delete dataList;
    delete btnGenerate;
    delete ui;
}
