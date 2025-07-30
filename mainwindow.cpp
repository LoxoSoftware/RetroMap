#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "canvas.h"
#include "tile.h"
#include <QFileDialog>
#include <math.h>

Canvas* winCanvas;
Tileset tileset;

int tileset_selected_tile= -1;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    CheckCanvasPresent();
    ui->action16_color_mode->setChecked(tileset.is4bpp);

    if (tileset.is4bpp)
        ui->dckPalette->setWindowTitle("Palettes (4bpp)");
    else
        ui->dckPalette->setWindowTitle("Palette (8bpp)");
}

MainWindow::~MainWindow()
{
    if (winCanvas)
        delete winCanvas;
    delete ui;
    //exit(0);
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}


void MainWindow::on_actionNew_triggered()
{
    winCanvas= new Canvas(ui->winCanvasArea, 32, 32);

    CheckCanvasPresent();

    if (!winCanvas)
    {
        printf("Error creating canvas!\n");
        exit(1);
    }

    winCanvas->Clear(0);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //ui->winCanvasArea_content->setMinimumSize(ui->winCanvasArea->size());
    if (!winCanvas)
        return;
    ui->winCanvasArea_content->setMinimumSize(winCanvas->Size());
    ui->winCanvasArea_content->setMaximumSize(winCanvas->Size());
}

void MainWindow::CheckCanvasPresent()
{
    ui->dckPalette->setVisible((bool)winCanvas);
    ui->dckTiles->setVisible((bool)winCanvas);
    ui->dckToolbox->setVisible((bool)winCanvas);
    ui->menuTileset->setEnabled((bool)winCanvas);
    ui->menuView->setEnabled((bool)winCanvas);
}

void MainWindow::UpdateTilesetTable()
{
    ui->tblTiles->clear();
    ui->tblTiles->setRowCount(ceil(tileset.tiles.count()/ui->tblTiles->columnCount()));
    for (int iy=0; iy<tileset.tiles.count()/ui->tblTiles->columnCount(); iy++)
    {
        for (int ix=0; ix<ui->tblTiles->columnCount(); ix++)
        {
            QPixmap pix;
            pix.convertFromImage(tileset.tiles[ix+iy*ui->tblTiles->columnCount()]);
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
            bru_bg.setColor(tileset.palette[iy][ix]);
            bru_bg.setStyle(Qt::SolidPattern);
            item->setBackground(bru_bg);
            ui->tblPalette->setItem(iy, ix, item);
        }
    }
}

void MainWindow::on_actionZoom_in_triggered()
{
    if (!winCanvas)
        return;
    winCanvas->ZoomIn();
}

void MainWindow::on_actionZoom_out_triggered()
{
    if (!winCanvas)
        return;
    winCanvas->ZoomOut();
}

void MainWindow::on_btnImportTilesetImage_clicked()
{
    if (!winCanvas)
        return;
    QString ifile_name= QFileDialog::getOpenFileName(this, "Import tileset from image", "", "Supported image formats (*.bmp)");
    tileset.FromImage(ifile_name);
    ui->tblTiles->setRowCount(0);
    UpdateTilesetTable();
    UpdatePaletteTable();
    winCanvas->Redraw();
}


void MainWindow::on_tblTiles_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    tileset_selected_tile= currentColumn+currentRow*ui->tblTiles->columnCount();
}


void MainWindow::on_action16_color_mode_changed()
{
    tileset.is4bpp= ui->action16_color_mode->isChecked();

    if (tileset.is4bpp)
        ui->dckPalette->setWindowTitle("Palettes (4bpp)");
    else
        ui->dckPalette->setWindowTitle("Palette (8bpp)");
}

