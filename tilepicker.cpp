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

            if (tindex > project.tileset.tiles.count())
                break;

            if (tindex == project.tileset.tiles.count())
            {
                QPixmap pix= QPixmap(":/icons/tilepicker-new_tile");
                QIcon icon= QIcon(pix.scaled(ui->tblTiles->columnWidth(0),ui->tblTiles->rowHeight(0)));
                QTableWidgetItem* item= new QTableWidgetItem(icon, "");
                ui->tblTiles->setItem(iy, ix, item);
                break;
            }

            QPixmap pix;
            bool previewPal= main_window->isTilePicker_ViewSelPal() && project.tileset.isSubPalettedFormat();
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

void TilePicker::OpenContextMenu(QPoint screen_pos, int tile_id)
{
    if (tile_id >= project.tileset.tiles.count() || tile_id < 0)
    {
        //QMessageBox::critical(this, "Error", "Cannot open menu, index out fo range.\r\tThis is a bug!");
        return;
    }

    if (context_menu)
        delete context_menu;

    context_menu= new QMenu();
    context_menu->addAction("Set as background selection");
    connect(context_menu->actions().last(), &QAction::triggered, this, &TilePicker::on_changeBgSelection);
    context_menu->addAction("Edit...");
    connect(context_menu->actions().last(), &QAction::triggered, this, &TilePicker::on_editTileTriggered);
    context_menu->addAction("Duplicate");
    connect(context_menu->actions().last(), &QAction::triggered, this, &TilePicker::on_duplicateTileTriggered);
    context_menu->addAction("Delete");
    connect(context_menu->actions().last(), &QAction::triggered, this, &TilePicker::on_deleteTileTriggered);

    context_menu->setWindowModality(Qt::ApplicationModal);
    context_menu->setGeometry(QRect(screen_pos,context_menu->sizeHint()));
    context_menu->show();
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
    project.statusbar->showMessage("Left mouse click to select a tile as primary for drawing || Right mouse click for more options");
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

    tile_hovered= ui->tblTiles->currentColumn()+ui->tblTiles->currentRow()*ui->tblTiles->columnCount();

    if (tile_hovered >= project.tileset.tiles.count())
        return;

    // project.tileset_selected_bgtile= tile_hovered;
    // RedrawSelection();

    OpenContextMenu(QPoint(pos.x()+x(),pos.y()+y()+16), tile_hovered);
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

void TilePicker::on_changeBgSelection()
{
    project.tileset_selected_bgtile= tile_hovered;
    RedrawSelection();
}

void TilePicker::on_editTileTriggered()
{
    if (tile_hovered > project.tileset.tiles.count())
        return;
    main_window->NewTileEditTab(tile_hovered);
}

void TilePicker::on_duplicateTileTriggered()
{
    if (tile_hovered > project.tileset.tiles.count())
        return;
    project.tileset.tiles+= project.tileset.tiles[tile_hovered];
    Update();
}

void TilePicker::on_deleteTileTriggered()
{
    if (tile_hovered > project.tileset.tiles.count())
        return;
    if (project.tileset.tiles.count() <= 1)
        return;
    if (project.main_mapcanvas)
    {
        project.tileset.RemoveTile(tile_hovered, &project.main_mapcanvas->tiles);
        project.main_mapcanvas->Redraw();
    }
    else
        project.tileset.RemoveTile(tile_hovered);
    Update();
}

TilePickerTable::TilePickerTable(QWidget* parent)
    : QTableWidget(parent)
{
    connect(this, &QTableWidget::itemPressed, this, &TilePickerTable::on_itemPressed);
    setAcceptDrops(false);
}

void TilePickerTable::on_itemPressed(QTableWidgetItem* item)
{
    if (!item) return;

    dnd_start_cell= item;
}

void TilePickerTable::dragMoveEvent(QDragMoveEvent* event)
{
    dnd_answer_rect= event->answerRect();
}

void TilePickerTable::dropEvent(QDropEvent* event)
{
    if (!project.main_window)
    {
        QMessageBox::critical(this, "Move tile in tileset", "An unexpected error occoured, cannot move tile. (1)");
        return;
    }

    //QMessageBox::information(this, "st", QString::number(tx)+" "+QString::number(ty));

    QTableWidgetItem* src_cell= dnd_start_cell;
    QTableWidgetItem* dest_cell= itemAt(dnd_answer_rect.topLeft());

    if (!src_cell || !dest_cell)
    {
        //QMessageBox::critical(this, "Move tile in tileset", "An unexpected error occoured, cannot move tile.");
        return;
    }

    int src_tile_id= indexFromItem(src_cell).column()+columnCount()*indexFromItem(src_cell).row();
    int dest_tile_id= indexFromItem(dest_cell).column()+columnCount()*indexFromItem(dest_cell).row();

    //QMessageBox::information(this, "deb", "Drop direction: "+QString::number(dropIndicatorPosition()));

    if (src_tile_id >= project.tileset.tiles.count() || dest_tile_id >= project.tileset.tiles.count())
        return;

    // switch (dropIndicatorPosition())
    // {
    // case OnItem:
    //     break;
    // case AboveItem:
    //     break;
    // case BelowItem:
    //     break;
    // default:
    //     return;
    // }

    QImage ttile= project.tileset.tiles[src_tile_id];
    project.tileset.tiles[src_tile_id]= project.tileset.tiles[dest_tile_id];
    project.tileset.tiles[dest_tile_id]= ttile;

    if (project.main_window)
    {
        project.main_window->dckTilePicker->Update();
        if (project.main_mapcanvas)
            project.main_mapcanvas->Redraw();
    }
}

