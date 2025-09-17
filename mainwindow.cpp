#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "project.h"
#include "mapsizeselector.h"
#include "exportdialog.h"
#include "aboutdialog.h"
#include "optimizedialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QRgb>
#include <QLabel>
#include <math.h>

Project project;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    project.canvas_container= NewTilemapTab();
    project.statusbar= statusBar();

    dckTilePicker= new TilePicker(ui->centralwidget, this);
    addDockWidget(Qt::RightDockWidgetArea, dckTilePicker);
    dckPaletteEdit= new PaletteEdit(ui->centralwidget, this);
    addDockWidget(Qt::RightDockWidgetArea, dckPaletteEdit);
    dckToolbox= new ToolBoxPanel(ui->centralwidget);
    addDockWidget(Qt::LeftDockWidgetArea, dckToolbox);

    dckPaletteEdit->Update();
    dckToolbox->UpdateToolStatus();
    dckPaletteEdit->UpdateColorStatus();

    project.CreateNew(32, 32);
    if (project.editor_canvas)
        project.editor_canvas->draw_tilegrid= ui->actionShow_tile_grid->isChecked();
    CheckCanvasPresent();

    ChangeTileFormat(Tileset::GBA_8bpp);
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

    ui->actionTilePicker_selected_pal->setEnabled(project.tileset.isPalettedFormat());

    if (project.editor_canvas)
        project.editor_canvas->Redraw();
    dckPaletteEdit->Update();
    dckTilePicker->Update();
}

QScrollArea* MainWindow::NewTilemapTab()
{
    QScrollArea* new_scrollarea = new QScrollArea();
    new_scrollarea->setFrameShape(QFrame::WinPanel);

    ui->tabWidget->addTab(new_scrollarea, "Tilemap "+QString::number(ui->tabWidget->count()));

    return new_scrollarea;
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
    dckPaletteEdit->Update();
}

void MainWindow::on_actionImport_tileset_from_image_triggered()
{
    if (!project.editor_canvas)
        return;
    QString ifile_name= QFileDialog::getOpenFileName(this, "Import tileset from image", "", "Supported image formats (*.bmp)");
    if (ifile_name == "")
        return;
    project.tileset.FromImage(ifile_name, true);
    dckTilePicker->Update();
    dckPaletteEdit->Update();
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
    if (!project.editor_canvas)
    {
        QMessageBox::critical(this, "Error - Optimize tileset", "Canvas is null!");
        return;
    }
    if (!project.tileset.image || !project.tileset.tiles.count())
    {
        ChangeTileFormat(Tileset::GBA_8bpp);
        QMessageBox::critical(this, "Optimize tileset", "Please import a tileset first!");
        return;
    }
    unsigned int optiflags= OptimizeDialog(this).GetFlags(project.tileset.isPalettedFormat());
    if (!(optiflags&0x80))
        return; //User rejected on the dialog
    project.tileset.tiles= project.tileset.Optimized(&project.editor_canvas->tiles, optiflags&0x7F);
    project.tileset.RebuildTilesetImage();
    dckTilePicker->Update();
    project.editor_canvas->Redraw();
    project.editor_canvas->UpdateHistory();
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
    if (dckPaletteEdit->isBlockingPalUpdates()) return;
    dckPaletteEdit->UpdateColorStatus(false);
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
    if (isTilePicker_ViewSelPal() && project.tileset.isPalettedFormat())
        dckTilePicker->Update();
}

