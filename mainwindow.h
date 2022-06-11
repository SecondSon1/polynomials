#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QtWidgets/qlistwidget.h"
#include "core.h"
#include <QMainWindow>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	Core::Base base;
	void AddPolynomial();
	void Renumber();
	void LoadFromFile();
	void SaveToFile();
	void SetValidators();
	void GetCoeff();
	void Add();
	void Multiply();
	void Roots();
	void Derivative();
	void Delete();
private:
	Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
