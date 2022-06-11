#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <algorithm>
#include <QFontDatabase>
#include <QShortcut>
#include <QFileDialog>
#include <fstream>

#define DONT_ADD_NULL 0

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	setFixedSize(QSize(674, 615));
	QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	font.setPointSize(12);
	ui->AddButton->setFont(font);
	ui->Input->setFont(font);
	ui->BaseView->setFont(font);
	ui->StatusLabel->setFont(font);
	connect(ui->AddButton, &QPushButton::released, this, &MainWindow::AddPolynomial);
	connect(ui->Input, &QLineEdit::returnPressed, this, &MainWindow::AddPolynomial);
	connect(ui->get_3, &QPushButton::released, this, &MainWindow::GetCoeff);
	connect(ui->add_3, &QPushButton::released, this, &MainWindow::Add);
	connect(ui->mul_3, &QPushButton::released, this, &MainWindow::Multiply);
	connect(ui->roots_2, &QPushButton::released, this, &MainWindow::Roots);
	connect(ui->der_3, &QPushButton::released, this, &MainWindow::Derivative);
	connect(ui->del_2, &QPushButton::released, this, &MainWindow::Delete);
	connect(ui->actionLoad_from_file, &QAction::triggered, this, &MainWindow::LoadFromFile);
	connect(ui->actionSave_to_file, &QAction::triggered, this, &MainWindow::SaveToFile);
	ui->get_2->setValidator(new QIntValidator(0, 1000000000));
	ui->der_2->setValidator(new QIntValidator(0, 10));
	SetValidators();
	ui->Success->setVisible(false);
	ui->Error->setVisible(false);
	//	ui->actionLoad_from_file
}

void MainWindow::SetValidators() {
	if (base.Empty()) {
		ui->get_1->setReadOnly(true);
		ui->get_3->setDisabled(true);
		ui->add_1->setReadOnly(true);
		ui->add_2->setReadOnly(true);
		ui->add_3->setDisabled(true);
		ui->mul_1->setReadOnly(true);
		ui->mul_2->setReadOnly(true);
		ui->mul_3->setDisabled(true);
		ui->roots_1->setReadOnly(true);
		ui->roots_2->setDisabled(true);
		ui->der_1->setReadOnly(true);
		ui->der_3->setDisabled(true);
		ui->del_1->setReadOnly(true);
		ui->del_2->setDisabled(true);
	} else {
		uint32_t max = base.Size();
		ui->get_1->setReadOnly(false);
		ui->get_1->setValidator(new QIntValidator(1, max));
		ui->get_3->setDisabled(false);
		ui->add_1->setReadOnly(false);
		ui->add_1->setValidator(new QIntValidator(1, max));
		ui->add_2->setReadOnly(false);
		ui->add_2->setValidator(new QIntValidator(1, max));
		ui->add_3->setDisabled(false);
		ui->mul_1->setReadOnly(false);
		ui->mul_1->setValidator(new QIntValidator(1, max));
		ui->mul_2->setReadOnly(false);
		ui->mul_2->setValidator(new QIntValidator(1, max));
		ui->mul_3->setDisabled(false);
		ui->roots_1->setReadOnly(false);
		ui->roots_1->setValidator(new QIntValidator(1, max));
		ui->roots_2->setDisabled(false);
		ui->der_1->setReadOnly(false);
		ui->der_1->setValidator(new QIntValidator(1, max));
		ui->der_3->setDisabled(false);
		ui->del_1->setReadOnly(false);
		ui->del_1->setValidator(new QIntValidator(1, max));
		ui->del_2->setDisabled(false);
	}
}

void MainWindow::AddPolynomial() {
	std::string text = ui->Input->text().toStdString();
	std::string status_text;
	std::pair<Core::Polynomial::ErrorType, uint32_t> error = base.AddPolynomial(text);
#if DONT_ADD_NULL
	if (error.first != Core::Polynomial::ErrorType::OK || !base.GetLastPolynomial().Empty()) {
#endif
		std::string error_substring;
		std::string error_offset;
		auto MakeOffset = [](uint32_t offset) -> std::string {
			std::string result;
			for (uint32_t i = 0; i < offset; ++i)
				result.push_back('-');
			return result;
		};
		if (error.first != Core::Polynomial::OK) {
			uint32_t offset_first = std::min(error.second, (uint32_t) 7);
			std::cout << text << ' ' << text.size() << ' ' << offset_first;
			error_substring = text.substr(error.second - offset_first, offset_first + 1);
			if (offset_first + 1 < text.size()) {
				uint32_t offset_last = std::min((uint32_t) text.size() - error.second, (uint32_t) 7);
				error_substring += text.substr(error.second + 1, offset_last);
			}
			error_offset += MakeOffset(offset_first);
			error_offset.push_back('^');
		}
		switch (error.first) {
			case Core::Polynomial::ErrorType::UNKNOWN_CHARACTERS:
				status_text = "Unknown character: ";
				error_offset = MakeOffset(status_text.size()) + error_offset;
				status_text += error_substring + "\n" + error_offset;
				ui->Success->setVisible(false);
				ui->Error->setVisible(true);
				break;
			case Core::Polynomial::ErrorType::MULTIPLE_VARIABLES:
				status_text = "Multiple variables aren't allowed: ";
				error_offset = MakeOffset(status_text.size()) + error_offset;
				status_text += error_substring + "\n" + error_offset;
				ui->Success->setVisible(false);
				ui->Error->setVisible(true);
				break;
			case Core::Polynomial::ErrorType::EXPECTED_COEFFICIENT:
				status_text = "Expected coefficient: ";
				error_offset = MakeOffset(status_text.size()) + error_offset;
				status_text += error_substring + "\n" + error_offset;
				ui->Success->setVisible(false);
				ui->Error->setVisible(true);
				break;
			case Core::Polynomial::ErrorType::EXPECTED_DEGREE:
				status_text = "Expected degree: ";
				error_offset = MakeOffset(status_text.size()) + error_offset;
				status_text += error_substring + "\n" + error_offset;
				ui->Success->setVisible(false);
				ui->Error->setVisible(true);
				break;
			case Core::Polynomial::ErrorType::EXPECTED_POWER_SYMBOL:
				status_text = "Expected the '^' symbol after variable: ";
				error_offset = MakeOffset(status_text.size()) + error_offset;
				status_text += error_substring + "\n" + error_offset;
				ui->Success->setVisible(false);
				ui->Error->setVisible(true);
				break;
			case Core::Polynomial::ErrorType::EXPECTED_VARIABLE:
				status_text = "Expected variable: ";
				error_offset = MakeOffset(status_text.size()) + error_offset;
				status_text += error_substring + "\n" + error_offset;
				ui->Success->setVisible(false);
				ui->Error->setVisible(true);
				break;
			case Core::Polynomial::ErrorType::OK:
				Renumber();
				ui->Input->setText(QString());
				status_text = "Polynomial was successfully added!";
				ui->Success->setVisible(true);
				ui->Error->setVisible(false);
				break;
		}
#if DONT_ADD_NULL
	} else {
		base.DeletePolynomial(base.Size() - 1);
		ui->Success->setVisible(false);
		ui->Error->setVisible(false);
		status_text = "Nothing to add";
	}
#endif
	ui->StatusLabel->setText(QString::fromStdString(status_text));
}

void MainWindow::Renumber() {
	SetValidators();
	ui->BaseView->clear();
	auto current = base.Head();
	for (uint32_t i = 1; current; current = current->next, ++i) {
		std::string to_print = std::to_string(i) + ". " + current->data.ExportAsString();
		ui->BaseView->addItem(QString::fromStdString(to_print));
	}
}

bool IsEmptyIgnoringSpaces(const std::string & str) {
	for (char c : str)
		if (c != ' ')
			return false;
	return true;
}

void MainWindow::GetCoeff() {
	if (IsEmptyIgnoringSpaces(ui->get_1->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index not specified")));
		return;
	}
	if (IsEmptyIgnoringSpaces(ui->get_2->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Degree not specified")));
		return;
	}
	uint32_t ind = atoi(ui->get_1->text().toStdString().data());
	if (ind > base.Size() || !ind) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index out of bounds")));
		return;
	}
	uint32_t n = atoi(ui->get_2->text().toStdString().data());
	int32_t result = base.GetPolynomial(ind - 1).GetCoefficient(n);
	ui->ActionStatus->setText(QString::fromStdString(std::string("Coefficient: ") + std::to_string(result)));
	ui->get_1->setText(QString());
	ui->get_2->setText(QString());
}
void MainWindow::Add() {
	if (IsEmptyIgnoringSpaces(ui->add_1->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("First index not specified")));
		return;
	}
	if (IsEmptyIgnoringSpaces(ui->add_2->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Second index not specified")));
		return;
	}
	uint32_t ind1 = atoi(ui->add_1->text().toStdString().data());
	if (ind1 > base.Size() || !ind1) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("First index out of bounds")));
		return;
	}
	uint32_t ind2 = atoi(ui->add_2->text().toStdString().data());
	if (ind2 > base.Size() || !ind2) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Second index out of bounds")));
		return;
	}
	Core::Polynomial p = base.AddPolynomials(ind1-1, ind2-1);
#if DONT_ADD_NULL
	if (p.Empty()) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Result was empty so nothing was added")));
	} else {
#endif
		base.AddPolynomial(p);
		ui->ActionStatus->setText(QString::fromStdString(std::string("Success")));
		Renumber();
#if DONT_ADD_NULL
	}
#endif
	ui->add_1->setText(QString());
	ui->add_2->setText(QString());
}
void MainWindow::Multiply() {
	if (IsEmptyIgnoringSpaces(ui->mul_1->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("First index not specified")));
		return;
	}
	if (IsEmptyIgnoringSpaces(ui->mul_2->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Second index not specified")));
		return;
	}
	uint32_t ind1 = atoi(ui->mul_1->text().toStdString().data());
	if (ind1 > base.Size() || !ind1) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("First index out of bounds")));
		return;
	}
	uint32_t ind2 = atoi(ui-> mul_2->text().toStdString().data());
	if (ind2 > base.Size() || !ind2) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Second index out of bounds")));
		return;
	}
	Core::Polynomial p = base.MultiplyPolynomials(ind1-1, ind2-1);
	base.AddPolynomial(p);
	Renumber();
	ui->ActionStatus->setText(QString::fromStdString(std::string("Success")));
	ui->mul_1->setText(QString());
	ui->mul_2->setText(QString());
}
void MainWindow::Roots() {
	if (IsEmptyIgnoringSpaces(ui->roots_1->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index not specified")));
		return;
	}
	uint32_t ind = atoi(ui->roots_1->text().toStdString().data());
	if (ind > base.Size() || !ind) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index out of bounds")));
		return;
	}
	std::vector<int> roots = base.GetIntegerRoots(ind - 1);
	std::string result = "Roots: ";
	for (int32_t root : roots) {
		result += std::to_string(root);
		if (root != roots.back())
			result += ", ";
	}
	ui->ActionStatus->setText(QString::fromStdString(result));
	ui->roots_1->setText(QString());
}
void MainWindow::Derivative() {
	if (IsEmptyIgnoringSpaces(ui->der_1->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index not specified")));
		return;
	}
	if (IsEmptyIgnoringSpaces(ui->der_2->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("N not specified")));
		return;
	}
	uint32_t ind = atoi(ui->der_1->text().toStdString().data());
	if (ind > base.Size() || !ind) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index out of bounds")));
		return;
	}
	uint32_t n = atoi(ui->der_2->text().toStdString().data());
	Core::Polynomial p = base.GetDerivative(ind - 1, n);
#if DONT_ADD_NULL
	if (p.Empty()) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Result was empty so nothing was added")));
	} else {
#endif
		base.AddPolynomial(p);
		Renumber();
		ui->ActionStatus->setText(QString::fromStdString(std::string("Success")));
#if DONT_ADD_NULL
	}
#endif
	ui->der_1->setText(QString());
	ui->der_2->setText(QString());
}
void MainWindow::Delete() {
	if (IsEmptyIgnoringSpaces(ui->del_1->text().toStdString())) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index not specified")));
		return;
	}
	uint32_t ind = atoi(ui->del_1->text().toStdString().data());
	if (ind > base.Size() || !ind) {
		ui->ActionStatus->setText(QString::fromStdString(std::string("Index out of bounds")));
		return;
	}
	base.DeletePolynomial(ind - 1);
	Renumber();
	ui->del_1->setText(QString());
}

void MainWindow::LoadFromFile() {
	std::string path = QFileDialog::getOpenFileName(this, tr("Load Polynomials"), "/home/secondson/Desktop", tr("Polynomial File (*.pln)"))
			.toStdString();
	std::ifstream input(path);
	for (std::string polynomial; getline(input, polynomial);) {
		std::pair<Core::Polynomial::ErrorType, uint32_t> error = base.AddPolynomial(polynomial);
		if (error.first != Core::Polynomial::ErrorType::OK) {
			std::cerr << "ээээ обещали файл нормальный а это че" << std::endl;
		}
	}
	input.close();
	Renumber();
}

void MainWindow::SaveToFile() {
	std::string path = QFileDialog::getSaveFileName(this, tr("Save Polynomials"), "/home/secondson/Desktop", tr("Polynomial File (*.pln)"))
			.toStdString();
	std::ofstream output(path);
	for (auto current = base.Head(); current; current = current->next) {
		std::string polynomial = current->data.ExportAsString();
		output << polynomial << std::endl;
	}
	output.close();
	Renumber();
}

MainWindow::~MainWindow() {
	delete ui;
}

