// Student Record Management System - main window (matches the hand-made Designer form)
//
// Widget object names expected in mainwindow.ui:
//   QLineEdit : nameEdit, rollEdit, fatherEdit, casteEdit, districtEdit, gpaEdit
//   QPushButton: addBtn, updateBtn, deleteBtn, nextBtn
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextStream>

// ---------------------------------------------------------------- setup

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Button clicks (event-driven programming)
    connect(ui->addBtn,    &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(ui->updateBtn, &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(ui->nextBtn,   &QPushButton::clicked, this, &MainWindow::onNext);

    loadFromFile();
    enterAddMode();   // at start: Add enabled, Update/Delete disabled
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------- file handling

QString MainWindow::filePath() const
{
    // students.txt is kept next to the program
    return QCoreApplication::applicationDirPath() + "/students.txt";
}

// File format: one record per line ->
// name,roll,father,caste,district,gpa
void MainWindow::loadFromFile()
{
    m_students.clear();
    QFile file(filePath());
    if (!file.exists())
        return;   // first run: no file yet
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File error", "Could not open " + filePath());
        return;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        const QStringList f = line.split(',');
        if (f.size() != 6)
            continue;   // skip damaged lines
        m_students.push_back({f[0].trimmed(), f[1].trimmed(), f[2].trimmed(),
                              f[3].trimmed(), f[4].trimmed(), f[5].trimmed()});
    }
}

bool MainWindow::saveToFile()
{
    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "File error", "Could not write to " + filePath());
        return false;
    }
    QTextStream out(&file);
    for (const Student &s : m_students)
        out << s.name << ',' << s.roll << ',' << s.father << ','
            << s.caste << ',' << s.district << ',' << s.gpa << '\n';
    return true;
}

// ------------------------------------------------------------- helpers

// Reads and validates the form. Returns false (after showing a message) if invalid.
bool MainWindow::readForm(Student &s)
{
    s.name     = ui->nameEdit->text().trimmed();
    s.roll     = ui->rollEdit->text().trimmed();
    s.father   = ui->fatherEdit->text().trimmed();
    s.caste    = ui->casteEdit->text().trimmed();
    s.district = ui->districtEdit->text().trimmed();
    s.gpa      = ui->gpaEdit->text().trimmed();

    if (s.name.isEmpty() || s.roll.isEmpty() || s.father.isEmpty() ||
        s.caste.isEmpty() || s.district.isEmpty() || s.gpa.isEmpty()) {
        QMessageBox::warning(this, "Missing data", "Please fill in all fields.");
        return false;
    }

    // Commas would break the comma-separated file format
    const QStringList all = {s.name, s.roll, s.father, s.caste, s.district, s.gpa};
    for (const QString &v : all) {
        if (v.contains(',')) {
            QMessageBox::warning(this, "Invalid character",
                                 "Fields must not contain commas (,).");
            return false;
        }
    }

    bool ok = false;
    const double g = s.gpa.toDouble(&ok);
    if (!ok || g < 0.0 || g > 4.0) {
        QMessageBox::warning(this, "Invalid GPA", "GPA must be a number between 0 and 4.");
        return false;
    }
    return true;
}

int MainWindow::findByRoll(const QString &roll) const
{
    for (int i = 0; i < m_students.size(); ++i)
        if (m_students[i].roll.compare(roll, Qt::CaseInsensitive) == 0)
            return i;
    return -1;
}

void MainWindow::showStudent(int index)
{
    m_current = index;
    const Student &s = m_students[index];
    ui->nameEdit->setText(s.name);
    ui->rollEdit->setText(s.roll);
    ui->fatherEdit->setText(s.father);
    ui->casteEdit->setText(s.caste);
    ui->districtEdit->setText(s.district);
    ui->gpaEdit->setText(s.gpa);
    updateButtons();
    statusBar()->showMessage(
        QString("Record %1 of %2").arg(index + 1).arg(m_students.size()));
}

void MainWindow::enterAddMode()
{
    m_current = -1;
    ui->nameEdit->clear();
    ui->rollEdit->clear();
    ui->fatherEdit->clear();
    ui->casteEdit->clear();
    ui->districtEdit->clear();
    ui->gpaEdit->clear();
    updateButtons();
    statusBar()->showMessage(
        QString("Add mode - %1 record(s) saved. Enter details and click Add.")
            .arg(m_students.size()));
    ui->nameEdit->setFocus();
}

void MainWindow::updateButtons()
{
    const bool viewing = (m_current >= 0);
    ui->addBtn->setEnabled(!viewing);       // Add only in add mode
    ui->updateBtn->setEnabled(viewing);     // Update/Delete only while viewing a record
    ui->deleteBtn->setEnabled(viewing);
    ui->nextBtn->setEnabled(!m_students.isEmpty());
    ui->rollEdit->setReadOnly(viewing);     // roll number is the unique ID, not editable
}

// --------------------------------------------------------------- slots

void MainWindow::onAdd()
{
    Student s;
    if (!readForm(s))
        return;
    if (findByRoll(s.roll) != -1) {
        QMessageBox::warning(this, "Duplicate roll number",
                             "A student with this roll number already exists.");
        return;
    }
    m_students.push_back(s);
    if (!saveToFile()) {
        m_students.pop_back();
        return;
    }
    QMessageBox::information(this, "Success", "Student record added successfully.");
    enterAddMode();
}

void MainWindow::onUpdate()
{
    if (m_current < 0)
        return;
    Student s;
    if (!readForm(s))
        return;
    // Roll number identifies the record to modify
    const int idx = findByRoll(s.roll);
    if (idx == -1) {
        QMessageBox::warning(this, "Not found", "No record with this roll number.");
        return;
    }
    m_students[idx] = s;
    if (!saveToFile())
        return;
    QMessageBox::information(this, "Success", "Student record updated successfully.");
    showStudent(idx);
}

void MainWindow::onDelete()
{
    if (m_current < 0)
        return;
    const auto answer = QMessageBox::question(
        this, "Confirm delete",
        QString("Delete record of %1 (%2)?").arg(m_students[m_current].name,
                                                 m_students[m_current].roll));
    if (answer != QMessageBox::Yes)
        return;

    m_students.remove(m_current);
    if (!saveToFile())
        return;
    QMessageBox::information(this, "Success", "Student record deleted.");

    if (m_students.isEmpty())
        enterAddMode();
    else
        showStudent(qMin(m_current, m_students.size() - 1));
}

// Next shows the following record. After the last record it returns to
// add mode (blank form) so new students can be added again.
void MainWindow::onNext()
{
    if (m_students.isEmpty()) {
        QMessageBox::information(this, "No records", "There are no saved records yet.");
        return;
    }
    const int next = m_current + 1;
    if (next >= m_students.size())
        enterAddMode();
    else
        showStudent(next);
}
