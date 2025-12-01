#ifndef TILECANVAS_H
#define TILECANVAS_H

#include "abstractcanvas.h"
#include "macro.h"
#include <QWidget>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMenu>

#define TILECANVAS_MAX_SCALING      64
#define TILECANVAS_BORDER_W         2

QT_BEGIN_NAMESPACE
namespace Ui {
class Canvas;
}
QT_END_NAMESPACE

class TileCanvas  : public AbstractCanvas
{
    Q_OBJECT
public:
    TileCanvas(QScrollArea* parent, int tile_id);
    ~TileCanvas();

    void Clear(int color);
    void Plot(int x, int y, int color);
    void UpdateHistory();
    void Undo();
    void Redo();

    void ZoomIn();
    void ZoomOut();
    void Redraw();
    //void RedrawPixel(int x, int y);

    QImage image;

    char Type() { return TYPE_TILECANVAS; }
    int  TileId() { return tile_id; }

    void UpdateTileId(int new_tile_id);
    void UpdateSourceTile();
    void UpdateMyTile();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
#if QT_VERSION_MAJOR > 5
    void enterEvent(QEnterEvent* event);
#else
    void enterEvent(QEvent* event);
#endif
    void leaveEvent(QEvent* event);

private:
    Ui::Canvas* ui;
    QGraphicsScene scene;
    QImage last_image;
    QBrush brush;
    QPen pen;
    int scaling= 20;
    Qt::MouseButton mouse_down_button= Qt::NoButton;
    bool mouse_has_moved= false;
    QPointF mouse_last_pos;
    QPointF mouse_last_global_pos;
    int history_current_index= -1;
    bool fill_screen= true;
    bool draw_tilegrid= true;

    int tile_id= -1;

    void UpdateScaling();
    //void OpenContextMenu(QPoint screen_pos, QPoint canvas_pos);
};

#endif // TILECANVAS_H
