#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "project.h"
#include "mapsizeselector.h"
#include "exportdialog.h"
#include "aboutdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QRgb>
#include <math.h>

Project project;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    project.canvas_container= ui->winCanvasArea;

    dckTilePicker= new TilePicker(ui->centralwidget, this);
    addDockWidget(Qt::RightDockWidgetArea, dckTilePicker);

    ui->tblPalette->setCurrentCell(0,0);
    UpdatePaletteTable();
    UpdateToolStatus();
    UpdateColorStatus();

    project.CreateNew(32, 32);
    if (project.editor_canvas)
        project.editor_canvas->draw_tilegrid= ui->actionShow_tile_grid->isChecked();
    CheckCanvasPresent();

    ChangeTileFormat(Tileset::GBA_8bpp);

    connect(ui->sliRedChannel, &QSlider::valueChanged, this, &MainWindow::on_colorChanged);
    connect(ui->sliBlueChannel, &QSlider::valueChanged, this, &MainWindow::on_colorChanged);
    connect(ui->sliGreenChannel, &QSlider::valueChanged, this, &MainWindow::on_colorChanged);
    connect(ui->spbRedChannel, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_colorChanged);
    connect(ui->spbBlueChannel, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_colorChanged);
    connect(ui->spbGreenChannel, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_colorChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::opt_tilePicker_view_sel_pal()
    { return ui->actionTilePicker_selected_pal->isChecked(); }

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
    ui->dckToolbox->setVisible((bool)project.editor_canvas);
    ui->menuTileset->setEnabled((bool)project.editor_canvas);
    ui->menuTilemap->setEnabled((bool)project.editor_canvas);
    ui->menuView->setEnabled((bool)project.editor_canvas);
    ui->menuEdit->setEnabled((bool)project.editor_canvas);
}

void MainWindow::ChangeTileFormat(Tileset::tile_format_t format)
{
    project.tileset.format= format;

    ui->actionGBA_8bpp->setChecked(format==Tileset::GBA_8bpp);
    ui->actionGBA_4bpp->setChecked(format==Tileset::GBA_4bpp);

    switch (format)
    {
    case Tileset::GBA_4bpp:
        ui->dckPalette->setWindowTitle("Palettes (4bpp)");
        ui->actionTilePicker_selected_pal->setEnabled(true);
        break;
    case Tileset::GBA_8bpp:
        ui->dckPalette->setWindowTitle("Palette (8bpp)");
        ui->actionTilePicker_selected_pal->setEnabled(false);
        break;
    default:
        ui->dckPalette->setWindowTitle("Palette (invalid tile format!)");
        break;
    }

    if (project.editor_canvas)
        project.editor_canvas->Redraw();
    UpdatePaletteTable();
    dckTilePicker->Update();
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
            if (project.tileset.format == Tileset::GBA_4bpp)
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
            if (project.tileset.format == Tileset::GBA_4bpp)
                item->setToolTip("Pal #"+QString::number(iy)+": "+QString::number(ix));
            else
                item->setToolTip("Index: "+QString::number(ix+iy*PALETTE_W));
            ui->tblPalette->setItem(iy, ix, item);
        }
    }
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

void MainWindow::UpdateColorStatus(bool force)
{
    if (ui->sliRedChannel->hasFocus() || force)
        ui->spbRedChannel->setValue(ui->sliRedChannel->value());
    if (ui->sliGreenChannel->hasFocus() || force)
        ui->spbGreenChannel->setValue(ui->sliGreenChannel->value());
    if (ui->sliBlueChannel->hasFocus() || force)
        ui->spbBlueChannel->setValue(ui->sliBlueChannel->value());

    if (ui->spbRedChannel->hasFocus() && !force)
        ui->sliRedChannel->setValue(ui->spbRedChannel->value());
    if (ui->spbGreenChannel->hasFocus() && !force)
        ui->sliGreenChannel->setValue(ui->spbGreenChannel->value());
    if (ui->spbBlueChannel->hasFocus() && !force)
        ui->sliBlueChannel->setValue(ui->spbBlueChannel->value());

    ui->widPrimaryColor->setStyleSheet("border: 1px solid black; background-color: rgb("
        + QString::number(ui->spbRedChannel->value()*8) + ","
        + QString::number(ui->spbGreenChannel->value()*8) + ","
        + QString::number(ui->spbBlueChannel->value()*8) + ");");

    int paltable_index= project.paltable_current_column+project.paltable_current_row*PALETTE_W;
    QRgb new_col= QColor(ui->spbRedChannel->value()*8,
                            ui->spbGreenChannel->value()*8, ui->spbBlueChannel->value()*8).rgb();

    if (paltable_index < project.tileset.palette.count() && !force)
    {
        project.tileset.palette[paltable_index]= new_col;
        if (block_pal_updates) return;
        UpdatePaletteTable();
    }
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
}

void MainWindow::on_actionLoad_triggered()
{
    QString ifname;
    ifname= QFileDialog::getOpenFileName(this, "Open project", "", "Mapped project file (*.mpd)");

    if (ifname == "")
        return;

    project.LoadFromFile(ifname);
    ChangeTileFormat(project.tileset.format);

    CheckCanvasPresent();
    dckTilePicker->Update();
    UpdatePaletteTable();
}

void MainWindow::on_actionImport_tileset_from_image_triggered()
{
    if (!project.editor_canvas)
        return;
    QString ifile_name= QFileDialog::getOpenFileName(this, "Import tileset from image", "", "Supported image formats (*.bmp)");
    if (ifile_name == "")
        return;
    project.tileset.FromImage(ifile_name, true);
    project.tileset.Optimize(Tileset::OptimizeDefault);
    dckTilePicker->Update();
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
        ChangeTileFormat(Tileset::GBA_8bpp);
        QMessageBox::critical(this, "Optimize tileset", "Please import a tileset first!");
        return;
    }
    project.tileset.Optimize(Tileset::OptimizeWithFlip);
    dckTilePicker->Update();
    project.editor_canvas->Redraw();
}

void MainWindow::on_tblPalette_cellClicked(int row, int column)
{
    project.paltable_current_column= column;
    project.paltable_current_row= row;
    UpdatePaletteTable();
    int palind= column+row*PALETTE_W;
    if (palind >= project.tileset.palette.count())
        return;
    block_pal_updates= true;
    ui->sliRedChannel->setValue(qRed(project.tileset.palette[palind])/8);
    ui->sliGreenChannel->setValue(qGreen(project.tileset.palette[palind])/8);
    ui->sliBlueChannel->setValue(qBlue(project.tileset.palette[palind])/8);
    UpdateColorStatus(true);
    block_pal_updates= false;
    if (ui->actionTilePicker_selected_pal && project.tileset.format == Tileset::GBA_4bpp)
        dckTilePicker->Update();
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
    project.pen_size= 3;
}

void MainWindow::on_btnSize3_clicked(bool checked)
{
    ui->btnSize1->setChecked(false);
    ui->btnSize2->setChecked(false);
    ui->btnSize3->setChecked(true);
    project.pen_size= 5;
}

void MainWindow::on_actionMapChange_Size_triggered()
{
    MapSizeSelector* resizeui= new MapSizeSelector();
    resizeui->setWindowModality(Qt::ApplicationModal);
    resizeui->show();
}

void MainWindow::on_actionUndo_triggered()
{
    project.editor_canvas->Undo();
}

void MainWindow::on_actionRedo_triggered()
{
    project.editor_canvas->Redo();
}

void MainWindow::on_colorChanged()
{
    if (block_pal_updates) return;
    UpdateColorStatus(false);
    project.tileset.UpdatePalettes();
    if (ui->actionAuto_canvas_update->isChecked())
    {
        project.editor_canvas->Redraw();
        dckTilePicker->Update();
    }
}

void MainWindow::on_actionRedraw_canvas_triggered()
{
    project.tileset.UpdatePalettes();
    project.editor_canvas->Redraw();
    dckTilePicker->Update();
}

void MainWindow::on_actionShow_tile_grid_triggered(bool checked)
{
    project.editor_canvas->draw_tilegrid= checked;
    project.editor_canvas->Redraw();
}

void MainWindow::on_actionExport_as_source_file_triggered()
{
    ExportDialog* odial= new ExportDialog(this);
    odial->open();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog* dialog= new AboutDialog(this);
    dialog->open();
}

void MainWindow::on_actionGBA_8bpp_triggered()
{
    ChangeTileFormat(Tileset::GBA_8bpp);
}

void MainWindow::on_actionGBA_4bpp_triggered()
{
    ChangeTileFormat(Tileset::GBA_4bpp);
}

void MainWindow::on_actionTilePicker_selected_pal_triggered()
{
    dckTilePicker->Update();
}
