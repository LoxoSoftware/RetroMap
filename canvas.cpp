#include "canvas.h"
#include "project.h"
#include "toolboxpanel.h"
#include <QMenu>
#include <QMouseEvent>
#include <QAction>
#include <QScrollBar>
#include <QMessageBox>
#include <QLabel>
#include <math.h>

extern Project project;

#define CANVASX_TO_COLUMN(x)    (((x-TILEPAD*((x/TILE_W)/scaling))/TILE_W)/scaling)
#define CANVASY_TO_ROW(y)       (((y-TILEPAD*((y/TILE_H)/scaling))/TILE_H)/scaling)
#define TILEPAD                 (draw_tilegrid? 1:0)  //You can change these values to manage how thick the grid is, even if it's off

#define CANVAS_HISTORY_MAX      32

Canvas::Canvas(QScrollArea* parent, int width, int height)
{
    size= QSize(width, height);
    setScene(&scene);
    setParent(parent);
    parent->setWidget(this);
    setStyleSheet("background-color: white;"
                         "border: "+QString::number(CANVAS_BORDER_W)+"px solid #666;");
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    Clear(0);

    Redraw();
    show();
}

void Canvas::Clear(int bgtile)
{
    tiles.clear();
    for (int i=0; i<size.width()*size.height(); i++)
        tiles+= Tile(bgtile);
    //Redraw();
}

void Canvas::Resize(int width, int height)
{
    QList<Tile> new_tiles;

    //Try to adapt the old tile list into a new size
    for (int iy=0; iy<height; iy++)
    {
        for (int ix=0; ix<width; ix++)
        {
            if (iy >= size.height() || ix >= size.width())
            {
                Tile ttile;
                ttile.tileset_offset= 0;
                ttile.hflip= 0;
                ttile.vflip= 0;
                ttile.palette_index= 0;
                new_tiles+= ttile;
                continue;
            }
            else
                new_tiles+= tiles[ix+iy*size.width()];
        }
    }

    size= QSize(width, height);
    tiles= new_tiles;

    tiles_history.clear();
    history_current_index= 0;
    UpdateHistory();

    //UpdateScaling();
    Redraw();
}

void Canvas::Plot(int row, int column, Tile tile)
{
    if (row<0 || row>=size.height())
        return;
    if (column<0 || column>=size.width())
        return;
    tiles[column+row*size.width()]= tile;
}

Canvas::~Canvas()
{
    close();
}

QSize Canvas::Size()
{
    return size;
}

void Canvas::ZoomIn()
{
    if (scaling >= CANVAS_MAX_SCALING)
        return;
    scaling++;
    Redraw();
}

void Canvas::ZoomOut()
{
    if (scaling <= 1)
        return;
    scaling--;
    Redraw();
}

void Canvas::Redraw()
{
    scene.clear();
    UpdateScaling();
    for (int iy=0; iy<size.height(); iy++)
    {
        for (int ix=0; ix<size.width(); ix++)
        {
            RedrawTile(iy, ix);
        }
    }
}

void Canvas::RedrawTile(int row, int column)
{
    if (row<0 || row>=size.height())
        return;
    if (column<0 || column>=size.width())
        return;

    QBrush bru;
    bru.setColor(QColor::fromRgb(255,0,0));
    bru.setStyle(Qt::DiagCrossPattern);
    QPen pen;
    pen.setColor(QColor::fromRgb(128,128,128));
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    Tile* ttile= &tiles[column+row*size.width()];

    if (ttile->tileset_offset >= project.tileset.tiles.count())
    {
        //Tile index is outside the bounds of the tileset
        scene.addRect(column*TILE_W*scaling+TILEPAD*column, row*TILE_H*scaling+TILEPAD*row,
                      TILE_W*scaling+TILEPAD, TILE_H*scaling+TILEPAD, pen, bru);
    }
    else
    {
        QPixmap pix;
        QTransform trans= QTransform();

        if (project.tileset.format == Tileset::GBA_4bpp)
        {
            //Clamp 256 color indexes to 16 colors of one of 16 palettes
            QImage pix_tf= project.tileset.tiles[ttile->tileset_offset];

            for (int iy=0; iy<pix_tf.height(); iy++)
            {
                unsigned char* slptr= pix_tf.scanLine(iy);

                for (int ix=0; ix<pix_tf.width(); ix++)
                {
                    slptr[ix]= tiles[column+row*size.width()].palette_index*PALETTE_W+(slptr[ix]%PALETTE_W);
                }
            }

            pix= QPixmap::fromImage(pix_tf);
        }
        else
            pix= QPixmap::fromImage(project.tileset.tiles[ttile->tileset_offset]);

        trans= trans.scale((ttile->hflip?-1:1), (ttile->vflip?-1:1));
        pix= pix.transformed(trans);

        QGraphicsPixmapItem* item= new QGraphicsPixmapItem(pix);
        item->setX(column*TILE_W*scaling+TILEPAD*column);
        item->setY(row*TILE_H*scaling+TILEPAD*row);
        item->setScale(scaling);
        scene.addItem(item);
    }
}

void Canvas::UpdateScaling()
{
    setMinimumSize(size.width()*TILE_W*scaling+(CANVAS_BORDER_W*2)+TILEPAD*size.width(),
                   size.height()*TILE_H*scaling+(CANVAS_BORDER_W*2)+TILEPAD*size.height());
    setMaximumSize(this->minimumSize());
    scene.setSceneRect(QRect(0,0,size.width()*(TILE_W+TILEPAD)*scaling,size.height()*(TILE_H+TILEPAD)*scaling));
}

void Canvas::OpenContextMenu(QPoint screen_pos, QPoint canvas_pos)
{
    int tilex= CANVASX_TO_COLUMN(canvas_pos.x());
    int tiley= CANVASY_TO_ROW(canvas_pos.y());
    int tilen= tilex+tiley*size.width();

    if (tilen >= tiles.count())
    {
        QMessageBox::critical(this, "Error", "Cannot open menu, index out fo range.\r\tThis is a bug!");
        return;
    }

    if (context_menu)
        delete context_menu;

    context_menu= new QMenu();
    context_menu->addAction("Clear tile with background");
    connect(context_menu->actions().last(), &QAction::triggered, this, &Canvas::onMenuClearWithBgTile_triggered);
    context_menu_palette_sel= context_menu->addMenu("Palette index: "+QString::number(tiles[tilen].palette_index)+" (change)");
    for (int i=0; i<PALETTE_H; i++)
    {
        context_menu_palette_sel->addAction(""+QString::number(i));
    }
    connect(context_menu_palette_sel, &QMenu::triggered, this, &Canvas::onMenuChangePal_triggered);
    context_menu_palette_sel->setEnabled(project.tileset.format == Tileset::GBA_4bpp);
    context_menu->addAction("Flip tile horizontally");
    context_menu->actions().last()->setCheckable(true);
    context_menu->actions().last()->setChecked(tiles[tilen].hflip);
    connect(context_menu->actions().last(), &QAction::triggered, this, &Canvas::onMenuHFlip_triggered);
    context_menu->addAction("Flip tile vertically");
    context_menu->actions().last()->setCheckable(true);
    context_menu->actions().last()->setChecked(tiles[tilen].vflip);
    connect(context_menu->actions().last(), &QAction::triggered, this, &Canvas::onMenuVFlip_triggered);

    context_menu->setWindowModality(Qt::ApplicationModal);
    context_menu->setGeometry(QRect(screen_pos,context_menu->sizeHint()));
    context_menu->show();
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    //event->accept();
    mouse_down_button= event->button();
    mouse_last_pos= event->pos();
#if QT_VERSION_MAJOR > 5
    mouse_last_global_pos= event->globalPosition();
#else
    mouse_last_global_pos= event->globalPos();
#endif

    if (mouse_down_button == Qt::MiddleButton)
        this->setCursor(Qt::ClosedHandCursor);

    if (mouse_down_button != Qt::RightButton)
        mouseMoveEvent(event);

    mouse_has_moved= false;
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();

    if (mouse_down_button == Qt::RightButton || mouse_down_button == Qt::LeftButton)
    for (int iy=-floor((float)project.pen_size/2); iy<ceil((float)project.pen_size/2); iy++)
    {
        for (int ix=-floor((float)project.pen_size/2); ix<ceil((float)project.pen_size/2); ix++)
        {
            int tilex= CANVASX_TO_COLUMN(event->pos().x())+ix;
            int tiley= CANVASY_TO_ROW(event->pos().y())+iy;

            ManagedPlot(tilex, tiley);
        }
    }

    if (mouse_down_button == Qt::MiddleButton)
    {
#if QT_VERSION_MAJOR > 5
        ((QScrollArea*)parent())->scroll(event->globalPosition().x()-mouse_last_global_pos.x(),
                                         event->globalPosition().y()-mouse_last_global_pos.y());
        mouse_last_global_pos= event->globalPosition();
#else
        ((QScrollArea*)parent())->scroll(event->globalPos().x()-mouse_last_global_pos.x(),
                                         event->globalPos().y()-mouse_last_global_pos.y());
        mouse_last_global_pos= event->globalPos();
#endif
    }

    mouse_has_moved= true;
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    if (mouse_down_button == Qt::MiddleButton)
        this->setCursor(Qt::ArrowCursor);

    if (!mouse_has_moved && mouse_down_button == Qt::RightButton)
#if QT_VERSION_MAJOR > 5
        OpenContextMenu(event->globalPosition().toPoint(), event->pos());
#else
        OpenContextMenu(event->globalPos(), event->pos());
#endif
    else if (mouse_down_button == Qt::RightButton || mouse_down_button == Qt::LeftButton)
        UpdateHistory();

    mouse_down_button= Qt::NoButton;
    mouse_has_moved= false;
}

void Canvas::ManagedPlot(int tilex, int tiley)
{
    if (tilex < 0 || tilex >= size.width())
        return;
    if (tiley < 0 || tiley >= size.height())
        return;

    Tile ttile= tiles[tilex+tiley*size.width()];

    switch (mouse_down_button)
    {
    case Qt::LeftButton:
        if (project.selected_tools & ToolBoxPanel::tool_OffsetPen
            && project.tileset_selected_tile >= 0)
            ttile.tileset_offset= project.tileset_selected_tile;
        if (project.selected_tools & ToolBoxPanel::tool_HFlipPen)
            ttile.hflip= false;
        if (project.selected_tools & ToolBoxPanel::tool_VFlipPen)
            ttile.vflip= false;
        if (project.selected_tools & ToolBoxPanel::tool_PalettePen)
            ttile.palette_index= project.paltable_current_row;

        Plot(tiley, tilex, ttile);
        RedrawTile(tiley, tilex);
        break;
    case Qt::RightButton:
        if (project.selected_tools & ToolBoxPanel::tool_OffsetPen
            && project.tileset_selected_tile >= 0)
            ttile.tileset_offset= project.tileset_selected_bgtile;
        if (project.selected_tools & ToolBoxPanel::tool_HFlipPen)
            ttile.hflip= true;
        if (project.selected_tools & ToolBoxPanel::tool_VFlipPen)
            ttile.vflip= true;
        if (project.selected_tools & ToolBoxPanel::tool_PalettePen
            && project.tileset.palette.count() > 0)
            ttile.palette_index= project.tileset.tiles[ttile.tileset_offset].pixelIndex(0,0)/PALETTE_W;

        Plot(tiley, tilex, ttile);
        RedrawTile(tiley, tilex);
        break;
    default:
        break;
    }
}

void Canvas::onMenuClearWithBgTile_triggered()
{
    if (project.tileset_selected_bgtile < 0 || project.tileset_selected_bgtile >= project.tileset.tiles.count())
        return;
    Tile ttile;
    ttile.tileset_offset= project.tileset_selected_bgtile;
    ttile.hflip= false;
    ttile.vflip= false;
    ttile.palette_index= project.tileset.tiles[ttile.tileset_offset].pixelIndex(0,0)/PALETTE_W;
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Plot(yt, xt, ttile);
    RedrawTile(yt, xt);
    UpdateHistory();
}

void Canvas::onMenuHFlip_triggered()
{
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Tile* tile= &tiles[xt+yt*size.width()];
    tile->hflip = !tile->hflip;
    RedrawTile(yt, xt);
    UpdateHistory();
}

void Canvas::onMenuVFlip_triggered()
{
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Tile* tile= &tiles[xt+yt*size.width()];
    tile->vflip = !tile->vflip;
    RedrawTile(yt, xt);
}

void Canvas::onMenuChangePal_triggered(QAction* selected_action)
{
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Tile* tile= &tiles[xt+yt*size.width()];
    tile->palette_index= std::strtol(selected_action->text().toLocal8Bit(), NULL, 10);
    RedrawTile(yt, xt);
    UpdateHistory();
}

QImage Canvas::GetImage()
{
    QImage timg= QImage(size.width()*TILE_W, size.height()*TILE_H, QImage::Format_Indexed8);
    timg.fill(0);
    timg.setColorTable(project.tileset.palette);

    //Manual rendering of tiles onto a QImage
    for (int iyt=0; iyt<size.height(); iyt++)
        for (int ixt=0; ixt<size.width(); ixt++)
        {
            for (int iy=0; iy<TILE_H; iy++)
            {
                unsigned char* timg_scanline= timg.scanLine(iy+iyt*TILE_H);
                int tm_ti= ixt+iyt*size.width();
                unsigned char* tile_scanline;

                if (!tiles[tm_ti].vflip)
                    tile_scanline= project.tileset.tiles[tiles[tm_ti].tileset_offset].scanLine(iy);
                else
                    tile_scanline= project.tileset.tiles[tiles[tm_ti].tileset_offset].scanLine(TILE_H-iy-1);

                for (int ix=0; ix<TILE_W; ix++)
                {
                    unsigned char pixel= tile_scanline[ix];
                    if (project.tileset.format == Tileset::GBA_4bpp)
                        pixel= tiles[tm_ti].palette_index*PALETTE_W+pixel%PALETTE_W;

                    if (!tiles[tm_ti].hflip)
                        timg_scanline[ix+ixt*TILE_W]= pixel;
                    else
                        timg_scanline[TILE_W-(ix+1)+ixt*TILE_W]= pixel;
                }
            }
        }

    return timg;
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    event->accept();

    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->angleDelta().y() > 16)
            ZoomIn();
        if (event->angleDelta().y() < -16)
            ZoomOut();
    }
    else
    {
        //This usually works with touchpads
        ((QScrollArea*)parent())->scroll(event->pixelDelta().x(), event->pixelDelta().y());
    }
}

#if QT_VERSION_MAJOR > 5
void Canvas::enterEvent(QEnterEvent* event)
#else
void Canvas::enterEvent(QEvent* event)
#endif
{
    if (!project.statusbar)
        return;
    project.statusbar->showMessage("Click and drag to draw with the selected tile || Right mouse button for context menu");
}

void Canvas::leaveEvent(QEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->clearMessage();
}

void Canvas::UpdateHistory()
{
    if (history_current_index >= CANVAS_HISTORY_MAX)
    {
        history_current_index--;
        tiles_history.removeFirst();
    }

    //Remove all future snapshots
    if (history_current_index < tiles_history.count()-1 && tiles_history.count() >= 0)
    {
        for (int i=0; i<=tiles_history.count()-history_current_index; i++)
            tiles_history.removeLast();
    }

    tiles_history+= tiles;
    history_current_index++;
}

void Canvas::Undo()
{
    if (tiles_history.isEmpty())
        return;
    if (history_current_index <= 0)
    {
        history_current_index= 0;
        return;
    }
    history_current_index--;
    tiles= tiles_history[history_current_index];
    Redraw();
}

void Canvas::Redo()
{
    if (tiles_history.isEmpty() || history_current_index < 0)
        return;
    if (history_current_index >= tiles_history.count()-1)
    {
        history_current_index= tiles_history.count()-1;
        return;
    }
    history_current_index++;
    tiles= tiles_history[history_current_index];
    Redraw();
}
