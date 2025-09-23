#ifndef ABSTRACTCANVAS_H
#define ABSTRACTCANVAS_H

#include <QWidget>
#include <QGraphicsView>
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui {
class AbstractCanvas;
}
QT_END_NAMESPACE

class AbstractCanvas : public QGraphicsView
{
    Q_OBJECT
public:
    virtual QSize Size();
    virtual void ZoomIn();
    virtual void ZoomOut();
    virtual void Redraw();
    virtual void UpdateHistory();
    virtual void Undo();
    virtual void Redo();

    //Type: 'm' for MapCanvas, 't' for TileCanvas, '\0' for AbstractCanvas (invalid)
    virtual char Type() { return 0; }

protected:
    AbstractCanvas();
    ~AbstractCanvas();

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
#if QT_VERSION_MAJOR > 5
    virtual void enterEvent(QEnterEvent* event);
#else
    virtual void enterEvent(QEvent* event);
#endif
    virtual void leaveEvent(QEvent* event);

    int scaling= 2;
    Ui::AbstractCanvas* ui;
    QGraphicsScene scene;
    QBrush brush;
    QPen pen;
    QMenu* context_menu= nullptr;
    Qt::MouseButton mouse_down_button= Qt::NoButton;
    bool mouse_has_moved= false;
    QPointF mouse_last_pos;
    QPointF mouse_last_global_pos;
    int history_current_index= -1;

    const int max_scaling= 10;

    virtual void UpdateScaling();
    virtual void OpenContextMenu(QPoint screen_pos, QPoint canvas_pos);
};

#endif // ABSTRACTCANVAS_H
