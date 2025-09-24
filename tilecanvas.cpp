#include "tilecanvas.h"
#include "tile.h"
#include "project.h"
#include <QMenu>
#include <QMouseEvent>
#include <QAction>
#include <QScrollBar>
#include <QMessageBox>

#define TILECANVAS_HISTORY_MAX      32

extern Project project;

TileCanvas::TileCanvas(QScrollArea* parent, int tile_id)
{
    image= QImage(TILE_W, TILE_H, QImage::Format_Indexed8);
    setScene(&scene);
    setParent(parent);
    parent->setWidget(this);
    setStyleSheet("background-color: white;"
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
}

TileCanvas::~TileCanvas() {};

void TileCanvas::Clear(int color)
{ image.fill(color); }

void TileCanvas::Plot(int x, int y, int color)
{ image.setPixel(x, y, color); }

void TileCanvas::UpdateHistory() {} //Todo

void TileCanvas::Undo() {} //Todo

void TileCanvas::Redo() {} //Todo

void TileCanvas::ZoomIn()
{

}

void TileCanvas::ZoomOut()
{

}

void TileCanvas::Redraw()
{

}

void TileCanvas::mousePressEvent(QMouseEvent* event) {} //Todo

void TileCanvas::mouseMoveEvent(QMouseEvent* event) {} //Todo

void TileCanvas::mouseReleaseEvent(QMouseEvent* event) {} //Todo

void TileCanvas::wheelEvent(QWheelEvent* event) {} //Todo

#if QT_VERSION_MAJOR > 5
void TileCanvas::enterEvent(QEnterEvent* event)
#else
void TileCanvas::enterEvent(QEvent* event)
#endif
{

}

void TileCanvas::leaveEvent(QEvent* event)
{

}

void TileCanvas::UpdateScaling()
{

}
