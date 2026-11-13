// Student Record Management System - main window (matches the hand-made Designer form)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// One student record (stored as one comma-separated line in the text file)
struct Student {
    QString name;
    QString roll;      // unique identifier
    QString father;
    QString caste;
    QString district;
    QString gpa;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAdd();       // Add button
    void onUpdate();    // Update button
    void onDelete();    // Delete button
    void onNext();      // Next button

private:
    // file handling
    QString filePath() const;
    void loadFromFile();
    bool saveToFile();
    // helpers
    bool readForm(Student &s);
    int findByRoll(const QString &roll) const;
    void showStudent(int index);
    void enterAddMode();
    void updateButtons();

    Ui::MainWindow *ui;
    QVector<Student> m_students;
    int m_current = -1;   // -1 = add mode, >= 0 = viewing a record
};

#endif // MAINWINDOW_H
