#ifndef TILECANVAS_H
#define TILECANVAS_H

#include "macro.h"
#include <QWidget>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMenu>

#define TILECANVAS_MAX_SCALING      32
#define TILECANVAS_BORDER_W         2

QT_BEGIN_NAMESPACE
namespace Ui {
class Canvas;
}
QT_END_NAMESPACE

class TileCanvas  : public QGraphicsView
{
    Q_OBJECT
public:
    TileCanvas(QScrollArea* parent, int width, int height); //In pixels
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
    QBrush brush;
    QPen pen;
    int scaling= 8;
    Qt::MouseButton mouse_down_button= Qt::NoButton;
    bool mouse_has_moved= false;
    QPointF mouse_last_pos;
    QPointF mouse_last_global_pos;
    int history_current_index= -1;

    void UpdateScaling();
    //void OpenContextMenu(QPoint screen_pos, QPoint canvas_pos);
};

#endif // TILECANVAS_H
