#include "optimizedialog.h"
#include "ui_optimizedialog.h"

OptimizeDialog::OptimizeDialog(QWidget *parent, bool isPalettedFmt)
    : QDialog(parent)
    , ui(new Ui::OptimizeDialog)
{
    ui->setupUi(this);

    ui->chkRmPalette->setChecked(isPalettedFmt);
}

OptimizeDialog::~OptimizeDialog()
{
    delete ui;
}

Tileset::optimize_flags_t OptimizeDialog::GetFlags(bool isPalettedFmt)
{
    this->setModal(true);
    ui->chkRmPalette->setChecked(isPalettedFmt);
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

