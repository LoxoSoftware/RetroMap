#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "project.h"
#include <QFileDialog>
#include <math.h>

Project project;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    project.canvas_container= ui->winCanvasArea;
    CheckCanvasPresent();
    ui->action16_color_mode->setChecked(project.tileset.is4bpp);

    if (project.tileset.is4bpp)
        ui->dckPalette->setWindowTitle("Palettes (4bpp)");
    else
        ui->dckPalette->setWindowTitle("Palette (8bpp)");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}


void MainWindow::on_actionNew_triggered()
{
    project.CreateNew(32, 32);

    CheckCanvasPresent();

    if (!project.editor_canvas)
    {
        printf("Error creating canvas!\n");
        exit(1);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //ui->winCanvasArea_content->setMinimumSize(ui->winCanvasArea->size());
    if (!project.editor_canvas)
        return;
    ui->winCanvasArea_content->setMinimumSize(project.editor_canvas->Size());
    ui->winCanvasArea_content->setMaximumSize(project.editor_canvas->Size());
}

void MainWindow::CheckCanvasPresent()
{
    ui->dckPalette->setVisible((bool)project.editor_canvas);
    ui->dckTiles->setVisible((bool)project.editor_canvas);
    ui->dckToolbox->setVisible((bool)project.editor_canvas);
    ui->menuTileset->setEnabled((bool)project.editor_canvas);
    ui->menuTilemap->setEnabled((bool)project.editor_canvas);
    ui->menuView->setEnabled((bool)project.editor_canvas);
}

void MainWindow::UpdateTilesetTable()
{
    ui->tblTiles->clear();
    ui->tblTiles->setRowCount(ceil(project.tileset.tiles.count()/ui->tblTiles->columnCount()));
    for (int iy=0; iy<project.tileset.tiles.count()/ui->tblTiles->columnCount(); iy++)
    {
        for (int ix=0; ix<ui->tblTiles->columnCount(); ix++)
        {
            QPixmap pix;
            pix.convertFromImage(project.tileset.tiles[ix+iy*ui->tblTiles->columnCount()]);
            QIcon icon= QIcon(pix.scaled(ui->tblTiles->columnWidth(0),ui->tblTiles->rowHeight(0)));
            QTableWidgetItem* item= new QTableWidgetItem(icon, "");
            ui->tblTiles->setItem(iy, ix, item);
        }
    }
}

void MainWindow::UpdatePaletteTable()
{
    ui->tblPalette->clear();
    for (int iy=0; iy<PALETTE_H; iy++)
    {
        for (int ix=0; ix<PALETTE_W; ix++)
        {
            QTableWidgetItem* item= new QTableWidgetItem();
            QBrush bru_bg;
            bru_bg.setColor(project.tileset.palette[ix+iy*PALETTE_W]);
            bru_bg.setStyle(Qt::SolidPattern);
            item->setBackground(bru_bg);
            ui->tblPalette->setItem(iy, ix, item);
        }
    }
    ui->action16_color_mode->setChecked(project.tileset.is4bpp);
}

void MainWindow::on_actionZoom_in_triggered()
{
    if (!project.editor_canvas)
        return;
    project.editor_canvas->ZoomIn();
}

void MainWindow::on_actionZoom_out_triggered()
{
    if (!project.editor_canvas)
        return;
    project.editor_canvas->ZoomOut();
}

void MainWindow::on_tblTiles_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    project.tileset_selected_tile= currentColumn+currentRow*ui->tblTiles->columnCount();
}

void MainWindow::on_action16_color_mode_changed()
{
    project.tileset.is4bpp= ui->action16_color_mode->isChecked();

    if (project.tileset.is4bpp)
        ui->dckPalette->setWindowTitle("Palettes (4bpp)");
    else
        ui->dckPalette->setWindowTitle("Palette (8bpp)");
}


void MainWindow::on_actionSave_triggered()
{
    if (!project.editor_canvas)
        return;

    if (project.project_fpath == "")
    {
        on_actionSave_as_triggered();
        return;
    }
    else
    {
        project.SaveToFile(project.project_fpath);
    }
}


void MainWindow::on_actionSave_as_triggered()
{
    if (!project.editor_canvas)
        return;

    QString ofname;
    ofname= QFileDialog::getSaveFileName(this, "Save project as...", "", "Mapped project file (*.mpd)");

    if (ofname == "")
        return;

    project.SaveToFile(ofname);
    project.project_fpath= ofname;
}


void MainWindow::on_actionLoad_triggered()
{
    QString ifname;
    ifname= QFileDialog::getOpenFileName(this, "Open project", "", "Mapped project file (*.mpd)");

    if (ifname == "")
        return;

    project.LoadFromFile(ifname);
    project.project_fpath= ifname;

    CheckCanvasPresent();
    UpdateTilesetTable();
    UpdatePaletteTable();
}


void MainWindow::on_actionImport_tileset_from_image_triggered()
{
    if (!project.editor_canvas)
        return;
    QString ifile_name= QFileDialog::getOpenFileName(this, "Import tileset from image", "", "Supported image formats (*.bmp)");
    if (ifile_name == "")
        return;
    project.tileset.FromImage(ifile_name);
    ui->tblTiles->setRowCount(0);
    project.tileset.Optimize(Tileset::OptimizeDefault);
    UpdateTilesetTable();
    UpdatePaletteTable();
    project.editor_canvas->Redraw();
}


void MainWindow::on_actionExport_as_indexed_bitmap_triggered()
{
    if (!project.editor_canvas)
        return;
    QString ofile_name= QFileDialog::getSaveFileName(this, "Export map as bitmap", "", "Indexed bitmap (*.bmp)");
    if (ofile_name == "")
        return;
    project.editor_canvas->GetImage().save(ofile_name, "bmp");
}


void MainWindow::on_actionOptimize_tileset_triggered()
{
    project.tileset.Optimize(Tileset::OptimizeWithFlip);
    UpdateTilesetTable();
    project.editor_canvas->Redraw();
}


void MainWindow::on_action16_color_mode_triggered()
{
    project.tileset.is4bpp= ui->action16_color_mode->isChecked();
    project.editor_canvas->Redraw();
    UpdatePaletteTable();
    UpdateTilesetTable();
}

