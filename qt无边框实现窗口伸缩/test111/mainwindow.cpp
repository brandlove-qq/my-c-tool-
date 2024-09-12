#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include "DragProxy.h"
#include <QDialog>

MainWindow::MainWindow(QWidget *parent) /*:QDialog(parent)*/
    :QMainWindow(parent)
  // ui(new Ui::MainWindow)
{
    // ui(new Ui::MainWindow)
    //ui->setupUi(this);
    //ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setMinimumSize(400, 400); //最小
    setMaximumSize(1000, 1000);//最大
    DragProxy* dragProxy = new DragProxy(this);
    dragProxy->SetBorderWidth(10, 10, 10, 10);
}

MainWindow::~MainWindow()
{
   // delete ui;
}
