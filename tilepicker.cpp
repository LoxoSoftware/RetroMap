#include "tilepicker.h"
#include "ui_tilepicker.h"
#include "project.h"
#include "mainwindow.h"
#include <QResizeEvent>
#include <QMessageBox>

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

void TilePicker::on_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    project.tileset_selected_tile= currentColumn+currentRow*ui->tblTiles->columnCount();
}

void TilePicker::on_actionTilePicker_selected_pal_triggered()
{
    Update();
}

void TilePicker::Update()
{
    if (ui->tblTiles->columnCount() <= 0)
    {
        ui->tblTiles->clear();
        return;
    }

    int selx= project.tileset_selected_tile%ui->tblTiles->columnCount();
    int sely= project.tileset_selected_tile/ui->tblTiles->columnCount();

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
            if (main_window->opt_tilePicker_view_sel_pal() && project.tileset.format == Tileset::GBA_4bpp)
            {
                QImage timg= project.tileset.tiles[tindex];
                //Alter image pixels to clamp it to a 16 bit palette
                for (int iiy=0; iiy<timg.height(); iiy++)
                {
                    unsigned char* slptr= timg.scanLine(iiy);

                    for (int iix=0; iix<timg.width(); iix++)
                    {
                        slptr[iix]= slptr[iix]%PALETTE_W+project.paltable_current_row*PALETTE_W;
                    }
                }
                pix.convertFromImage(timg);
            }
            else
                pix.convertFromImage(project.tileset.tiles[tindex]);
            QIcon icon= QIcon(pix.scaled(ui->tblTiles->columnWidth(0),ui->tblTiles->rowHeight(0)));
            QTableWidgetItem* item= new QTableWidgetItem(icon, "");
            ui->tblTiles->setItem(iy, ix, item);
        }
    }

    if (project.tileset_selected_tile < project.tileset.tiles.count())
    {
        //Restore selection
        //ui->tblTiles->setRangeSelected(QTableWidgetSelectionRange(sely,selx,sely,selx),true);
        ui->tblTiles->setCurrentCell(sely, selx);
    }
}

void TilePicker::resizeEvent(QResizeEvent* event)
{
    ui->tblTiles->setGeometry(0,0,event->size().width(),event->size().height());
}

void TilePicker::on_tblTiles_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    project.tileset_selected_tile= currentColumn+currentRow*ui->tblTiles->columnCount();
}

