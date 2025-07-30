#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include "tile.h"

#define CANVAS_MAX_SCALING      10
#define CANVAS_BORDER_W         2

QT_BEGIN_NAMESPACE
namespace Ui {
class Canvas;
}
QT_END_NAMESPACE

class Canvas : public QGraphicsView
{
    Q_OBJECT
public:
    Canvas(QWidget* parent, int width, int height);
    ~Canvas();

    void Clear(int bgtile);
    void PlotUnscaled(QPoint pos, Tile tile);
    void Plot(int row, int column, Tile tile);

    QSize Size();   //In tiles
    void ZoomIn();
    void ZoomOut();
    void Redraw();
    void RedrawTile(int row, int column);

    QList<Tile> tiles;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::Canvas* ui;
    QSize size;
    QGraphicsScene scene;
    QBrush brush;
    QPen pen;
    int scaling= 2;
    Qt::MouseButton mouse_down_button= Qt::NoButton;
    bool mouse_has_moved= false;

    void UpdateScaling();
    void OpenContextMenu(QPoint screen_pos, QPoint canvas_pos);
};

#endif // CANVAS_H
