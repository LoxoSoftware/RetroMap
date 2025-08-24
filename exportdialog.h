#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

private slots:
    void on_btnBrowseOFile_clicked();

    void on_btnDialog_accepted();

private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
