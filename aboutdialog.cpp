#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QMessageBox>
#include "app_config.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->lblVersion->setText("Version: " PROJECT_VERSION);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_btnAboutQt_clicked()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void AboutDialog::on_btnClose_clicked()
{
    this->close();
}
