#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "project.h"
#include "mapsizeselector.h"
#include <QFileDialog>
#include <QMessageBox>
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

    ui->tblPalette->setCurrentCell(0,0);
    UpdatePaletteTable();
    UpdateToolStatus();

    project.CreateNew(32, 32);
    CheckCanvasPresent();
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
    ui->tblTiles->setRowCount(ceil((float)project.tileset.tiles.count()/(float)ui->tblTiles->columnCount()));
    for (int iy=0; iy<ui->tblTiles->rowCount(); iy++)
    {
        for (int ix=0; ix<ui->tblTiles->columnCount(); ix++)
        {
            int tindex= ix+iy*ui->tblTiles->columnCount();

            if (tindex >= project.tileset.tiles.count())
                break;

            QPixmap pix;
            pix.convertFromImage(project.tileset.tiles[tindex]);
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
            if (ix+iy*PALETTE_W < project.tileset.palette.count())
                bru_bg.setColor(project.tileset.palette[ix+iy*PALETTE_W]);
            else
            {
                int lumarand= rand()%32;
                bru_bg.setColor(QColor::fromRgb(lumarand*8, lumarand*8, lumarand*8));
            }
            if (project.tileset.is4bpp)
            {
                if (project.paltable_current_row == iy && project.paltable_current_column == ix)
                    bru_bg.setStyle(Qt::Dense3Pattern);
                else if (project.paltable_current_row == iy)
                    bru_bg.setStyle(Qt::Dense1Pattern);
                else
                    bru_bg.setStyle(Qt::SolidPattern);
            }
            else
            {
                if (project.paltable_current_row == iy && project.paltable_current_column == ix)
                    bru_bg.setStyle(Qt::Dense3Pattern);
                else
                    bru_bg.setStyle(Qt::SolidPattern);
            }
            item->setBackground(bru_bg);
            if (project.tileset.is4bpp)
                item->setToolTip("Pal #"+QString::number(iy)+": "+QString::number(ix));
            else
                item->setToolTip("Index: "+QString::number(ix+iy*PALETTE_W));
            ui->tblPalette->setItem(iy, ix, item);
        }
    }

    ui->action16_color_mode->setChecked(project.tileset.is4bpp);
}

void MainWindow::UpdateToolStatus()
{
    int new_status= 0;

    if (ui->tlbPen->isChecked())
        new_status += tool_OffsetPen;
    if (ui->tlbVFlipPen->isChecked())
        new_status += tool_VFlipPen;
    if (ui->tlbHFlipPen->isChecked())
        new_status += tool_HFlipPen;
    if (ui->tlbPalettePen->isChecked())
        new_status += tool_PalettePen;

    // if (new_status == tool_NoTool)
    // {
    //     ui->tlbPen->setChecked(true);
    //     ui->tlbPalettePen->setChecked(true);
    //     return UpdateToolStatus();
    // }

    project.selected_tools= new_status;
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
    if (!project.tileset.image || !project.tileset.tiles.count())
    {
        ui->action16_color_mode->setChecked(false);
        project.tileset.is4bpp= false;
        QMessageBox::critical(this, "Optimize tileset", "Please import a tileset first!");
        return;
    }
    project.tileset.Optimize(Tileset::OptimizeWithFlip);
    UpdateTilesetTable();
    project.editor_canvas->Redraw();
}


void MainWindow::on_action16_color_mode_triggered()
{
    if (!project.tileset.image || !project.tileset.tiles.count())
    {
        ui->action16_color_mode->setChecked(false);
        project.tileset.is4bpp= false;
        QMessageBox::critical(this, "Change color mode", "Please import a tileset first!");
        return;
    }
    project.tileset.is4bpp= ui->action16_color_mode->isChecked();
    project.editor_canvas->Redraw();
    UpdatePaletteTable();
    UpdateTilesetTable();
}

void MainWindow::on_tblPalette_cellClicked(int row, int column)
{
    project.paltable_current_column= column;
    project.paltable_current_row= row;
    UpdatePaletteTable();
}

void MainWindow::on_tlbPen_clicked(bool checked)
{
    UpdateToolStatus();
}

void MainWindow::on_tlbVFlipPen_clicked(bool checked)
{
    UpdateToolStatus();
}

void MainWindow::on_tlbHFlipPen_clicked(bool checked)
{
    UpdateToolStatus();
}

void MainWindow::on_tlbPalettePen_clicked(bool checked)
{
    UpdateToolStatus();
}

void MainWindow::on_btnSize1_clicked(bool checked)
{
    ui->btnSize1->setChecked(true);
    ui->btnSize2->setChecked(false);
    ui->btnSize3->setChecked(false);
    project.pen_size= 1;
}

void MainWindow::on_btnSize2_clicked(bool checked)
{
    ui->btnSize1->setChecked(false);
    ui->btnSize2->setChecked(true);
    ui->btnSize3->setChecked(false);
    project.pen_size= 2;
}

void MainWindow::on_btnSize3_clicked(bool checked)
{
    ui->btnSize1->setChecked(false);
    ui->btnSize2->setChecked(false);
    ui->btnSize3->setChecked(true);
    project.pen_size= 3;
}

void MainWindow::on_actionMapChange_Size_triggered()
{
    MapSizeSelector* resizeui= new MapSizeSelector();
    resizeui->setWindowModality(Qt::ApplicationModal);
    resizeui->show();
}

