#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "src/Assembler.hpp"

#include <cstdlib>

#include <iostream>

#include <QFileDialog>
#include <QGraphicsItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	maxSpeed_(true)
{
	ui->setupUi(this);

	srand(time(NULL));

	timer = new QTimer(this);

	connect(timer, SIGNAL(timeout()), this, SLOT(doStep()));

	ui->runButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
	ui->stepButton->setEnabled(false);

	scene = new QGraphicsScene(0, 0, 299, 239, this);

	scene->setBackgroundBrush(QBrush(Qt::black));

	ui->graphicsView->setScene(scene);
}

MainWindow::~MainWindow()
{
	delete timer;
	delete scene;
	delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	ui->graphicsView->fitInView(scene->sceneRect());

	QMainWindow::resizeEvent(event);
}

void MainWindow::showEvent(QShowEvent *)
{
	ui->graphicsView->fitInView(scene->sceneRect());
}

void MainWindow::on_actionAssemble_And_Load_Player_1_triggered()
{
	std::string fname = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath()).toStdString();

	if(fname.empty())
		return;

	Assembler::getInstance().openFile(fname.c_str());

	if(!Assembler::getInstance().assembly())
	{
		QMessageBox::warning(this, tr("Compilation Error"), tr("Selected file was assembled with errors!"));

		return;
	}

	if(vm_.isLoadedP2())
	{
		p1Pos_ = p2Pos_ + rand() % (vm_.getCoreSize() - 2000) + 1000;

		ui->runButton->setEnabled(true);
		ui->stepButton->setEnabled(true);
	}

	else
		p1Pos_ = rand() % vm_.getCoreSize();

	vm_.loadProgram(Assembler::getInstance().getInstructions(), p1Pos_);

	ui->p1Log->appendPlainText(tr("Program loaded."));

	ui->p1ProcessesBar->setValue(1);

	ui->actionAssemble_And_Load_Player_1->setDisabled(true);
}

void MainWindow::on_actionAssemble_And_Load_Player_2_triggered()
{
	std::string fname = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath()).toStdString();

	if(fname.empty())
		return;

	Assembler::getInstance().openFile(fname.c_str());

	if(!Assembler::getInstance().assembly())
	{
		QMessageBox::warning(this, tr("Compilation Error"), tr("Selected file was assembled with errors!"));

		return;
	}

	if(vm_.isLoadedP1())
	{
		p2Pos_ = p1Pos_ + rand() % (vm_.getCoreSize() - 2000) + 1000;

		ui->runButton->setEnabled(true);
		ui->stepButton->setEnabled(true);
	}

	else
		p2Pos_ = rand() % vm_.getCoreSize();

	vm_.loadProgram(Assembler::getInstance().getInstructions(), p2Pos_, false);

	ui->p2Log->appendPlainText(tr("Program loaded."));

	ui->p2ProcessesBar->setValue(1);

	ui->actionAssemble_And_Load_Player_2->setDisabled(true);
}

void MainWindow::on_actionExit_triggered()
{
	QCoreApplication::quit();
}

void MainWindow::on_stepButton_clicked()
{
	doStep();
}

void MainWindow::on_actionReset_triggered()
{
	if(timer->isActive())
		timer->stop();

	scene->clear();

	ui->p1Log->clear();
	ui->p2Log->clear();

	ui->p1ProcessesBar->setValue(0);
	ui->p2ProcessesBar->setValue(0);

	ui->runButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
	ui->stepButton->setEnabled(false);

	ui->actionAssemble_And_Load_Player_1->setEnabled(true);
	ui->actionAssemble_And_Load_Player_2->setEnabled(true);

	vm_.reset();
}

void MainWindow::on_runButton_clicked()
{
	ui->runButton->setEnabled(false);
	ui->stopButton->setEnabled(true);
	ui->stepButton->setEnabled(false);

	timer->start();
}

void MainWindow::on_stopButton_clicked()
{
	ui->runButton->setEnabled(true);
	ui->stopButton->setEnabled(false);
	ui->stepButton->setEnabled(true);

	timer->stop();
}


void MainWindow::doStep()
{
	VirtualMachine::StatReport rep;
	QGraphicsRectItem* p;
	QList<QGraphicsItem*> list;

	vm_.executeCycle();

	rep = vm_.getP1Report();

	ui->p1ProcessesBar->setValue(rep.getProcessCount());

	if( (!maxSpeed_ && timer->isActive()) || !timer->isActive() )
		ui->p1Log->appendPlainText(QString::fromStdString(rep.toString()));

	//p = scene->addRect(3 * (rep.getExecutedAdr() % 100), 3 * (rep.getExecutedAdr() / 100), 2, 2, QPen(Qt::NoPen), QBrush(Qt::cyan));

	//list += scene->collidingItems(reinterpret_cast<QGraphicsItem*>(p));

	p = scene->addRect(3 * (rep.getWriteAdr() % 100), 3 * (rep.getWriteAdr() / 100), 2, 2, QPen(Qt::NoPen), QBrush(Qt::magenta));

	list += scene->collidingItems(reinterpret_cast<QGraphicsItem*>(p));

	while(!list.isEmpty())
	{
		QGraphicsItem* obj = list.last();

		scene->removeItem(obj);

		delete obj;

		list.removeLast();
	}

	if(VirtualMachine::StatReport::getState() != VirtualMachine::StatReport::ONGOING)
	{
		timer->stop();

		ui->runButton->setEnabled(false);
		ui->stopButton->setEnabled(false);
		ui->stepButton->setEnabled(false);
	}

	rep = vm_.getP2Report();

	ui->p2ProcessesBar->setValue(rep.getProcessCount());

	if( (!maxSpeed_ && timer->isActive()) || !timer->isActive() )
		ui->p2Log->appendPlainText(QString::fromStdString(rep.toString()));

	//p = scene->addRect(3 * (rep.getExecutedAdr() % 100), 3 * (rep.getExecutedAdr() / 100), 2, 2, QPen(Qt::NoPen), QBrush(Qt::red));

	//list += scene->collidingItems(reinterpret_cast<QGraphicsItem*>(p));

	p = scene->addRect(3 * (rep.getWriteAdr() % 100), 3 * (rep.getWriteAdr() / 100), 2, 2, QPen(Qt::NoPen), QBrush(Qt::yellow));

	list += scene->collidingItems(reinterpret_cast<QGraphicsItem*>(p));

	while(!list.isEmpty())
	{
		QGraphicsItem* obj = list.last();

		scene->removeItem(obj);

		delete obj;

		list.removeLast();
	}

	bool isEnd = false;

	QGraphicsSimpleTextItem* tp;
	QGraphicsRectItem* rp;

	switch(VirtualMachine::StatReport::getState())
	{
		case VirtualMachine::StatReport::ONGOING:
			break;

		case VirtualMachine::StatReport::DRAW:
			tp = new QGraphicsSimpleTextItem(QString("DRAW"));
			tp->setBrush(QBrush(Qt::white));
			tp->setPos(scene->width() / 2 - tp->boundingRect().width() / 2, scene->height() / 2 - tp->boundingRect().height() / 2);
			rp = scene->addRect(tp->boundingRect(), QPen(Qt::red), QBrush(Qt::black));
			scene->addItem(tp);
			rp->setTransform(tp->sceneTransform());
			isEnd = true;
			break;

		case VirtualMachine::StatReport::P1_WON:
			tp = new QGraphicsSimpleTextItem(QString("PLAYER 1 WINS"));
			tp->setBrush(QBrush(Qt::white));
			tp->setPos(scene->width() / 2 - tp->boundingRect().width() / 2, scene->height() / 2 - tp->boundingRect().height() / 2);
			rp = scene->addRect(tp->boundingRect(), QPen(Qt::red), QBrush(Qt::black));
			scene->addItem(tp);
			rp->setTransform(tp->sceneTransform());
			isEnd = true;
			break;

		case VirtualMachine::StatReport::P2_WON:
			tp = new QGraphicsSimpleTextItem(QString("PLAYER 2 WINS"));
			tp->setBrush(QBrush(Qt::white));
			tp->setPos(scene->width() / 2 - tp->boundingRect().width() / 2, scene->height() / 2 - tp->boundingRect().height() / 2);
			rp = scene->addRect(tp->boundingRect(), QPen(Qt::red), QBrush(Qt::black));
			scene->addItem(tp);
			rp->setTransform(tp->sceneTransform());
			isEnd = true;
			break;
	}

	if(isEnd && timer->isActive())
	{
		timer->stop();

		ui->runButton->setEnabled(false);
		ui->stopButton->setEnabled(false);
		ui->stepButton->setEnabled(false);
	}
}

void MainWindow::on_speedSlider_valueChanged(int value)
{
	timer->setInterval((ui->speedSlider->maximum() - value) * 75);

	if(value == ui->speedSlider->maximum())
		maxSpeed_ = true;

	else
		maxSpeed_ = false;
}
