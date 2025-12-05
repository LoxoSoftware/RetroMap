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
    project.statusbar= statusBar();
    project.tab_widget= ui->tabWidget;
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
    if (project.current_mapcanvas)
        project.current_mapcanvas->draw_tilegrid= ui->actionShow_tile_grid->isChecked();
    CheckCanvasPresence();

    on_tabWidget_currentChanged(ui->tabWidget->currentIndex());

    project.main_window= this;

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

    CheckCanvasPresence();

    if (!project.current_mapcanvas)
    {
        printf("Error creating canvas!\n");
        exit(1);
    }
}

void MainWindow::CheckCanvasPresence()
{
    ui->menuTileset->menuAction()->setVisible((bool)project.current_mapcanvas);
    ui->menuTilemap->menuAction()->setVisible((bool)project.current_mapcanvas);
    ui->menuView->menuAction()->setVisible((bool)project.current_canvas);
    ui->menuEdit->menuAction()->setVisible((bool)project.current_canvas);
    dckPaletteEdit->setVisible((bool)project.current_canvas);
    dckTilePicker->setVisible((bool)project.current_mapcanvas);
    dckToolbox->setVisible((bool)project.current_mapcanvas);
}

void MainWindow::ChangeTileFormat(Tileset::tile_format_t format)
{
    project.tileset.format= format;

    ui->actionGBA_8bpp->setChecked(format==Tileset::GBA_8bpp);
    ui->actionGBA_4bpp->setChecked(format==Tileset::GBA_4bpp);

    ui->actionTilePicker_selected_pal->setEnabled(project.tileset.isSubPalettedFormat());

    if (project.current_mapcanvas)
        project.current_mapcanvas->Redraw();
    dckPaletteEdit->Update();
    dckTilePicker->Update();
}

QScrollArea* MainWindow::NewTilemapTab()
{
    QScrollArea* new_scrollarea = new QScrollArea();
    new_scrollarea->setFrameShape(QFrame::WinPanel);

    if (project.GetMainMapCanvas())
        delete project.main_mapcanvas;
    project.tab_widget->clear();

    project.tab_widget->addTab(new_scrollarea, "&Tilemap");
    project.GetMainMapCanvas();

    return new_scrollarea;
}

QScrollArea* MainWindow::NewTileEditTab(int tile_id)
{
    int existing= project.GetTileCanvasIndex(tile_id);

    if (existing >= 0)
    {
        project.tab_widget->setCurrentIndex(existing);
        return dynamic_cast<QScrollArea*>(project.tab_widget->currentWidget());
    }

    QScrollArea* new_scrollarea = new QScrollArea();
    new_scrollarea->setFrameShape(QFrame::WinPanel);
    new_scrollarea->setWidget(new TileCanvas(new_scrollarea, tile_id, TILE_W, TILE_H));

    int new_tab_ind= project.tab_widget->addTab(new_scrollarea,
                                                "Edit tile (&"+QString::number(project.tab_widget->count()-1)+")");
    project.tab_widget->setCurrentIndex(new_tab_ind);

    return new_scrollarea;
}

void MainWindow::on_actionZoom_in_triggered()
{
    if (!project.current_canvas)
        return;
    project.current_canvas->ZoomIn();
}

void MainWindow::on_actionZoom_out_triggered()
{
    if (!project.current_canvas)
        return;
    project.current_canvas->ZoomOut();
}

void MainWindow::on_actionSave_triggered()
{
    if (!project.current_canvas)
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
    if (!project.GetMainMapCanvas())
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

    CheckCanvasPresence();
    dckTilePicker->Update();
    dckPaletteEdit->Update();
}

void MainWindow::on_actionImport_tileset_from_image_triggered()
{
    if (!project.current_mapcanvas)
        return;
    QString ifile_name= QFileDialog::getOpenFileName(this, "Import tileset from image", "", "Supported image formats (*.bmp)");
    if (ifile_name == "")
        return;
    project.tileset.FromImage(ifile_name, true);
    dckTilePicker->Update();
    dckPaletteEdit->Update();
    project.current_mapcanvas->Redraw();
}

void MainWindow::on_actionExport_as_indexed_bitmap_triggered()
{
    if (!project.current_mapcanvas)
        return;
    QString ofile_name= QFileDialog::getSaveFileName(this, "Export map as bitmap", "", "Indexed bitmap (*.bmp)");
    if (ofile_name == "")
        return;
    project.current_mapcanvas->GetImage().save(ofile_name, "bmp");
}

void MainWindow::on_actionOptimize_tileset_triggered()
{
    if (!project.current_mapcanvas)
    {
        QMessageBox::critical(this, "Error", "Canvas is null!");
        return;
    }
    if (!project.tileset.image || !project.tileset.tiles.count())
    {
        ChangeTileFormat(Tileset::GBA_8bpp);
        QMessageBox::critical(this, "Error", "Please import a tileset first!");
        return;
    }
    unsigned int optiflags= OptimizeDialog(this).GetFlags(project.tileset.isSubPalettedFormat());
    if (!(optiflags&0x80))
        return; //User rejected on the dialog
    project.tileset.tiles= project.tileset.Optimized(&project.current_mapcanvas->tiles, optiflags&0x7F);
    project.tileset.RebuildTilesetImage();
    dckTilePicker->Update();
    project.current_mapcanvas->Redraw();
    project.current_mapcanvas->UpdateHistory();
}

void MainWindow::on_actionMapChange_Size_triggered()
{
    MapSizeSelector* resizeui= new MapSizeSelector();
    resizeui->setWindowModality(Qt::ApplicationModal);
    resizeui->show();
}

void MainWindow::on_actionUndo_triggered()
{
    if (!project.current_canvas)
        return;
    project.current_canvas->Undo();
}

void MainWindow::on_actionRedo_triggered()
{
    if (!project.current_canvas)
        return;
    project.current_canvas->Redo();
}

void MainWindow::on_colorChanged()
{
    if (dckPaletteEdit->isBlockingPalUpdates()) return;
    dckPaletteEdit->UpdateColorStatus(false);
    project.tileset.UpdatePalettes();
    if (ui->actionAuto_canvas_update->isChecked())
    {
        if (!project.current_canvas)
            return;
        project.current_canvas->Redraw();
        dckTilePicker->Update();
    }
}

void MainWindow::on_actionRedraw_canvas_triggered()
{
    if (!project.current_canvas)
        return;
    project.tileset.UpdatePalettes();
    project.current_canvas->Redraw();
    dckTilePicker->Update();
}

void MainWindow::on_actionShow_tile_grid_triggered(bool checked)
{
    if (!project.current_mapcanvas)
        return;
    project.current_mapcanvas->draw_tilegrid= checked;
    project.current_mapcanvas->Redraw();
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
    if (isTilePicker_ViewSelPal() && project.tileset.isSubPalettedFormat())
        dckTilePicker->Update();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (ui->tabWidget->count() <= 0)
    {
        project.current_container= nullptr;
        project.current_canvas= nullptr;
        project.current_mapcanvas= nullptr;
        project.current_tilecanvas= nullptr;

        CheckCanvasPresence();
        return;
    }
    if (!static_cast<QScrollArea*>(project.tab_widget->currentWidget()))
    {
        project.current_container= nullptr;
        project.current_canvas= nullptr;
        project.current_mapcanvas= nullptr;
        project.current_tilecanvas= nullptr;

        CheckCanvasPresence();
        return;
    }

    project.current_container= dynamic_cast<QScrollArea*>(project.tab_widget->currentWidget());
    project.current_canvas= dynamic_cast<AbstractCanvas*>(project.current_container->widget());
    project.current_mapcanvas= dynamic_cast<MapCanvas*>(project.current_canvas);
    project.current_tilecanvas= dynamic_cast<TileCanvas*>(project.current_canvas);

    if (project.current_mapcanvas)
        on_actionRedraw_canvas_triggered();
    if (project.current_tilecanvas)
        project.current_tilecanvas->UpdateMyTile();

    CheckCanvasPresence();
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    project.tab_widget->setCurrentIndex(index);
    on_tabWidget_currentChanged(index);

    if (project.current_tilecanvas)
    {
        project.tab_widget->removeTab(project.tab_widget->currentIndex());
        project.tab_widget->setCurrentIndex(0);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    if (project.current_tilecanvas)
        project.current_tilecanvas->Redraw();
}
