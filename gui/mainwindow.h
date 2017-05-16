#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/VirtualMachine.hpp"

#include <QMainWindow>
#include <QTimer>
#include "ui_mainwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:

	void resizeEvent(QResizeEvent*);

	void showEvent(QShowEvent*);

private slots:
	void on_actionAssemble_And_Load_Player_1_triggered();

	void on_actionAssemble_And_Load_Player_2_triggered();

	void on_actionExit_triggered();

	void on_stepButton_clicked();

	void on_actionReset_triggered();

	void on_runButton_clicked();

	void on_stopButton_clicked();

	void doStep();

	void on_speedSlider_valueChanged(int value);

private:

	Ui::MainWindow *ui;

	QGraphicsScene* scene;

	QTimer* timer;

	VirtualMachine vm_;

	bool maxSpeed_;

	unsigned int p1Pos_;
	unsigned int p2Pos_;
};

#endif // MAINWINDOW_H
