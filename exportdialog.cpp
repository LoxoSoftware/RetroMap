#include "exportdialog.h"
#include "ui_exportdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include "project.h"

extern Project project;

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{
    ui->setupUi(this);

    switch (project.tileset.format)
    {
    case Tileset::GBA_4bpp:
        ui->cmbFormat->setCurrentIndex(1);
        break;
    case Tileset::GBA_8bpp:
        ui->cmbFormat->setCurrentIndex(0);
        break;
    }
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::on_btnBrowseOFile_clicked()
{
    QString ofile_name= QFileDialog::getSaveFileName(this, "Export map as source file", "", "GAS (*.s)\nC (*.c)");
    if (ofile_name == "")
        return;
    ui->txtOFilePath->setText(ofile_name);
}


void ExportDialog::on_btnDialog_accepted()
{
    if (ui->txtOFilePath->text() == "")
    {
        on_btnBrowseOFile_clicked();
        if (ui->txtOFilePath->text() == "")
        {
            QMessageBox::critical(this, "Error - export as source file", "No valid output file path given.\r\nExport canceled.");
            return;
        }
    }
    int export_flags= 0;
    export_flags+= ui->chkExportGfx->isChecked()?Project::ExportGfx:0;
    export_flags+= ui->chkExportMap->isChecked()?Project::ExportMap:0;
    export_flags+= ui->chkExportPal->isChecked()?Project::ExportPal:0;
    export_flags+= ui->chkIncludeHFile->isChecked()?Project::ExportHFile:0;
    export_flags+= ui->chkOptimize->isChecked()?Project::ExportOptimize:0;
    switch(ui->cmbFormat->currentIndex())
    {
    case 0:
        export_flags+= Project::ExportGBA8bpp;
        break;
    case 1:
        export_flags+= Project::ExportGBA4bpp;
        break;
    case 2:
        export_flags+= Project::ExportGBAAffine;
        break;
    }
    if ((export_flags&Project::ExportAll) == Project::ExportNone)
    {
        QMessageBox::warning(this, "", "Nothing to do...");
        return;
    }
    int err= project.ExportToSourceFile(ui->txtOFilePath->text(), export_flags);
    if (!err)
        this->close();
    else
    {
        QMessageBox::critical(this, "Error - export as source file", "Export failed");
        return;
    }
}

