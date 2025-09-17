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
    return  ( ui->chkRmFlip->isChecked()?Tileset::OptimizeWithFlip:0 ) |
            ( ui->chkRmPalette->isChecked()?Tileset::OptimizeWithPalette:0 )
            + (accepted?0x80:0);
}

void OptimizeDialog::on_bnbDialog_accepted()
{
    accepted= true;
    this->close();
}

void OptimizeDialog::on_bnbDialog_rejected()
{
    ui->chkRmFlip->setChecked(false);
    ui->chkRmPalette->setChecked(false);
    accepted= false;
    this->close();
}

