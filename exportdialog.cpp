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

    ui->rdbGBA4bpp->setChecked(project.tileset.is4bpp);
    ui->rdbGBA8bpp->setChecked(!project.tileset.is4bpp);
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
    export_flags+= ui->rdbGBA4bpp->isChecked()?Project::ExportGBA4bpp:0;
    export_flags+= ui->rdbGBA8bpp->isChecked()?Project::ExportGBA8bpp:0;
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

