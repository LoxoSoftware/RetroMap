#include "optimizedialog.h"
#include "ui_optimizedialog.h"
#include <QMessageBox>

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
            ( ui->chkRmPalette->isChecked()?Tileset::OptimizeWithPalette:0 ) |
            ( ui->chkKeepUnused->isChecked()?Tileset::OptimizeKeepUnused:0 )
            + (accepted?0x80:0);
}

void OptimizeDialog::on_bnbDialog_accepted()
{
    if (!ui->chkKeepUnused->isChecked())
    {
        QMessageBox::StandardButton dialresult;
        dialresult= QMessageBox::question(this, "Removing unused tiles", "You are about to delete unused tiles,\r\n"
                                                                         "Are you sure? This cannot be undone");
        if (dialresult == QMessageBox::StandardButton::Yes)
        {
            accepted=true;
            this->close();
        }
        else
            return;
    }
    accepted= true;
    this->close();
}

void OptimizeDialog::on_bnbDialog_rejected()
{
    accepted= false;
    this->close();
}

