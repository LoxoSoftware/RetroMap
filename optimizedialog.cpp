#include "optimizedialog.h"
#include "ui_optimizedialog.h"

OptimizeDialog::OptimizeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptimizeDialog)
{
    ui->setupUi(this);
}

OptimizeDialog::~OptimizeDialog()
{
    delete ui;
}

Tileset::optimize_flags_t OptimizeDialog::GetFlags()
{
    this->setModal(true);
    this->exec();
    return  ( ui->chkRmDuplicate->isChecked()?Tileset::OptimizeDuplicate:0 ) |
            ( ui->chkRmFlip->isChecked()?Tileset::OptimizeWithFlip:0 ) |
            ( ui->chkRmPalette->isChecked()?Tileset::OptimizeWithPalette:0 );
}

void OptimizeDialog::on_bnbDialog_accepted()
{
    this->close();
}

void OptimizeDialog::on_bnbDialog_rejected()
{
    ui->chkRmDuplicate->setChecked(false);
    ui->chkRmFlip->setChecked(false);
    ui->chkRmPalette->setChecked(false);
    this->close();
}

