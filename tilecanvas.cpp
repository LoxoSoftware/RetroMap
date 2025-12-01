#include "tilecanvas.h"
#include "tile.h"
#include "project.h"
#include <QMenu>
#include <QMouseEvent>
#include <QAction>
#include <QScrollBar>
#include <QMessageBox>

#define TILECANVASX_TO_PIXEL(x)     ((x/scaling)%TILE_W)
#define TILECANVASY_TO_PIXEL(y)     ((y/scaling)%TILE_H)
#define TILECANVAS_TILEPAD          (draw_tilegrid? 1:0)

#define TILECANVAS_HISTORY_MAX      32

extern Project project;

TileCanvas::TileCanvas(QScrollArea* parent, int tile_id)
{
    image= QImage(TILE_W, TILE_H, QImage::Format_Indexed8);
    setScene(&scene);
    setParent(parent);
    parent->setWidget(this);
    setStyleSheet("background-image: url(:/ui/bgtile2);"
                  "border: "+QString::number(TILECANVAS_BORDER_W)+"px solid black;");
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (tile_id < 0 || tile_id >= project.tileset.tiles.count())
    {
        QMessageBox::critical(this, "Error", "Target tile id is out of bounds");
        return;
    }

    image= project.tileset.tiles[tile_id];
    this->tile_id= tile_id;

    Redraw();
    show();
    //parent->scroll(image.width()*scaling/2, image.height()*scaling/2);
}

TileCanvas::~TileCanvas() {};

void TileCanvas::Clear(int color)
{ image.fill(color); }

void TileCanvas::Plot(int x, int y, int color)
{
    if (x<0 || x>=image.width())
        return;
    if (y<0 || y>=image.height())
        return;
    image.setPixel(x, y, color);

    Redraw();
}

void TileCanvas::UpdateHistory() {} //Todo

void TileCanvas::Undo() {} //Todo

void TileCanvas::Redo() {} //Todo

void TileCanvas::ZoomIn()
{
    if (scaling >= TILECANVAS_MAX_SCALING)
    {
        scaling= TILECANVAS_MAX_SCALING;
        return;
    }
    scaling+=4;
    Redraw();
}

void TileCanvas::ZoomOut()
{
    if (scaling <= 4)
    {
        scaling= 4;
        return;
    }
    scaling-=4;
    Redraw();
}

void TileCanvas::Redraw()
{
    UpdateScaling();
    image.setColorTable(project.tileset.tiles[tile_id].colorTable());

    //Check if an update is necessary
    if (image == last_image)
        return;
    else
        last_image= image;

    scene.clear();
    QPixmap pix;
    pix= QPixmap::fromImage(image);
    int iterx= fill_screen? scene.width()/scaling : 1;
    int itery= fill_screen? scene.height()/scaling : 1;
    for (int iy=0; iy<itery; iy++)
    {
        for (int ix=0; ix<iterx; ix++)
        {
            QGraphicsPixmapItem* item= new QGraphicsPixmapItem(pix);
            item->setX(ix*(TILE_W*scaling+TILECANVAS_TILEPAD));
            item->setY(iy*(TILE_H*scaling+TILECANVAS_TILEPAD));
            item->setScale(scaling);
            scene.addItem(item);
        }
    }

}

void TileCanvas::mousePressEvent(QMouseEvent* event)
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

void TileCanvas::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();

    if (mouse_down_button == Qt::LeftButton)
    {
        int tilex= TILECANVASX_TO_PIXEL(event->pos().x());
        int tiley= TILECANVASY_TO_PIXEL(event->pos().y());
        Plot(tilex, tiley, project.paltable_current_column+project.paltable_current_row*PALETTE_W);
    }
}

void TileCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
    UpdateSourceTile();

    mouse_down_button= Qt::NoButton;
    mouse_has_moved= false;
}

void TileCanvas::wheelEvent(QWheelEvent* event) {} //Todo

#if QT_VERSION_MAJOR > 5
void TileCanvas::enterEvent(QEnterEvent* event)
#else
void TileCanvas::enterEvent(QEvent* event)
#endif
{
    if (!project.statusbar)
        return;
    project.statusbar->showMessage("Click and drag to draw with the selected color");
}

void TileCanvas::leaveEvent(QEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->clearMessage();
}

void TileCanvas::UpdateScaling()
{
    if (!fill_screen)
    {
        setMinimumSize(image.width()*scaling+(TILECANVAS_BORDER_W*2),
                       image.height()*scaling+(TILECANVAS_BORDER_W*2));
        setMaximumSize(this->minimumSize());
        scene.setSceneRect(QRect(0,0,image.width()*scaling,image.height()*scaling));
    }
    else
    {
        setMinimumSize(((QScrollArea*)parent())->size());
        setMaximumSize(this->minimumSize());
        scene.setSceneRect(QRect(0,0,this->maximumWidth(),this->maximumHeight()));
    }
}

void TileCanvas::UpdateTileId(int new_tile_id)
{

}

void TileCanvas::UpdateSourceTile()
{
    if (tile_id < 0 || tile_id >= project.tileset.tiles.count())
    {
        QMessageBox::critical(this, "Error", "Target tile id is out of bounds");
        return;
    }

    project.tileset.tiles[tile_id]= image;
}

void TileCanvas::UpdateMyTile()
{
    image= project.tileset.tiles[tile_id];
    Redraw();
}
