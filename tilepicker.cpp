#include "tilepicker.h"
#include "ui_tilepicker.h"
#include "project.h"
#include "mainwindow.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <math.h>

extern Project project;

TilePicker::TilePicker(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::TilePicker)
{
    ui->setupUi(this);
    this->main_window= main_window;
}

TilePicker::~TilePicker()
{
    delete ui;
}

void TilePicker::Update()
{
    if (ui->tblTiles->columnCount() <= 0)
    {
        ui->tblTiles->clear();
        return;
    }

    ui->tblTiles->clear();
    ui->tblTiles->setRowCount(floor((float)project.tileset.tiles.count()/(float)ui->tblTiles->columnCount())+1);
    for (int iy=0; iy<ui->tblTiles->rowCount(); iy++)
    {
        for (int ix=0; ix<ui->tblTiles->columnCount(); ix++)
        {
            int tindex= ix+iy*ui->tblTiles->columnCount();

            if (tindex >= project.tileset.tiles.count())
                break;

            QPixmap pix;
            bool previewPal= main_window->isTilePicker_ViewSelPal() && project.tileset.isPalettedFormat();
            pix.convertFromImage(Tile(tindex,false,false,project.paltable_current_row).RenderImage(&project.tileset, previewPal));

            QIcon icon= QIcon(pix.scaled(ui->tblTiles->columnWidth(0),ui->tblTiles->rowHeight(0)));
            QTableWidgetItem* item= new QTableWidgetItem(icon, "");
            ui->tblTiles->setItem(iy, ix, item);
        }
    }

    RedrawSelection();
}

void TilePicker::RedrawSelection()
{
    if (project.tileset.tiles.count() <= 0)
        return;
    if (project.tileset.tiles.count() < 2)
        project.tileset_selected_tile= project.tileset_selected_bgtile= 0;
    else
    {
        if (project.tileset_selected_tile < 0 || project.tileset_selected_tile >= project.tileset.tiles.count())
            project.tileset_selected_tile= 1;
        if (project.tileset_selected_bgtile < 0 || project.tileset_selected_bgtile >= project.tileset.tiles.count())
            project.tileset_selected_bgtile= 0;
    }

    QBrush bru_fg;
    bru_fg.setColor(QColor::fromRgb(255,0,0));
    bru_fg.setStyle(Qt::SolidPattern);
    QBrush bru_bg;
    bru_bg.setColor(QColor::fromRgb(255,0,0));
    bru_bg.setStyle(Qt::Dense5Pattern);
    QBrush bru_both;
    bru_both.setColor(QColor::fromRgb(128,0,128));
    bru_both.setStyle(Qt::SolidPattern);
    QBrush bru_notsel;
    bru_notsel.setStyle(Qt::NoBrush);

    for (int iy=0; iy<ui->tblTiles->rowCount(); iy++)
    {
        for (int ix=0; ix<ui->tblTiles->columnCount(); ix++)
        {
            int i= ix+iy*ui->tblTiles->columnCount();
            QTableWidgetItem* item= ui->tblTiles->item(iy, ix);
            if (!item) continue;

            if (i == project.tileset_selected_tile && i == project.tileset_selected_bgtile)
                item->setBackground(bru_both);
            else if (i == project.tileset_selected_bgtile)
                item->setBackground(bru_bg);
            else if (i == project.tileset_selected_tile)
                item->setBackground(bru_fg);
            else
                item->setBackground(bru_notsel);
        }
    }

    ui->tblTiles->clearSelection();
    ui->tblTiles->update();
}

void TilePicker::resizeEvent(QResizeEvent* event)
{
    //ui->tblTiles->setGeometry(10,10,event->size().width()-10,event->size().height()-10);
    event->accept();
    int old_columns= (event->oldSize().width()-12)/ui->tblTiles->columnWidth(0)-1;
    int new_columns= (event->size().width()-12)/ui->tblTiles->columnWidth(0)-1;
    ui->tblTiles->setColumnCount(new_columns);
    if (old_columns != new_columns)
        Update();
}

#if QT_VERSION_MAJOR > 5
void TilePicker::enterEvent(QEnterEvent* event)
#else
void TilePicker::enterEvent(QEvent* event)
#endif
{
    if (!project.statusbar)
        return;
    project.statusbar->showMessage("Left mouse click to select a tile as primary for drawing || Right mouse click to select a tile as secondary/background for drawing");
}

void TilePicker::leaveEvent(QEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->clearMessage();
}

void TilePicker::on_tblTiles_cellClicked(int row, int column)
{
    int new_selected_tile= column+row*ui->tblTiles->columnCount();
    if (new_selected_tile <= project.tileset.tiles.count())
        project.tileset_selected_tile= column+row*ui->tblTiles->columnCount();

    RedrawSelection();
}

void TilePicker::on_tblTiles_customContextMenuRequested(const QPoint &pos)
{
    if (project.tileset.tiles.count() <= 0)
        return;

    int new_selected_tile= ui->tblTiles->currentColumn()+ui->tblTiles->currentRow()*ui->tblTiles->columnCount();

    if (new_selected_tile >= project.tileset.tiles.count())
        return;

    project.tileset_selected_bgtile= new_selected_tile;
    RedrawSelection();

    // QMenu* context_menu= new QMenu();

    // context_menu->addAction("Select as background tile");

    // context_menu->setGeometry(QRect(
    //     pos.x()+x(), pos.y()+y()+50,
    //     context_menu->sizeHint().width(), context_menu->sizeHint().height()));
    // context_menu->show();
}

void TilePicker::on_tblTiles_cellDoubleClicked(int row, int column)
{
    int tile_id= column+row*ui->tblTiles->columnCount();
    if (tile_id == project.tileset.tiles.count())
    {
        //Add new tile
        QImage new_image= QImage(TILE_W, TILE_H, QImage::Format_Indexed8);
        new_image.setColorTable(project.tileset.palette);
        new_image.fill(0);
        project.tileset.tiles+= new_image;
    }
    if (tile_id > project.tileset.tiles.count())
        return;
    main_window->NewTileEditTab(tile_id);
}

