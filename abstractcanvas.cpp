#include "abstractcanvas.h"
#include "project.h"

extern Project project;

AbstractCanvas::AbstractCanvas() {}
AbstractCanvas::~AbstractCanvas() {}

QSize AbstractCanvas::Size()
{ return QSize(-1,-1); }

void AbstractCanvas::ZoomIn()
{
    if (scaling >= max_scaling)
        return;
    scaling++;
    Redraw();
}

void AbstractCanvas::ZoomOut()
{
    if (scaling <= 1)
        return;
    scaling--;
    Redraw();
}

void AbstractCanvas::Redraw() {}

void AbstractCanvas::UpdateHistory() {}
void AbstractCanvas::Undo() {}
void AbstractCanvas::Redo() {}

void AbstractCanvas::mousePressEvent(QMouseEvent* event) {}
void AbstractCanvas::mouseMoveEvent(QMouseEvent* event) {}
void AbstractCanvas::mouseReleaseEvent(QMouseEvent* event) {}
void AbstractCanvas::wheelEvent(QWheelEvent* event) {}

#if QT_VERSION_MAJOR > 5
void AbstractCanvas::enterEvent(QEnterEvent* event)
#else
void AbstractCanvas::enterEvent(QEvent* event)
#endif
{
    if (!project.statusbar)
        return;
    project.statusbar->showMessage("AbstractCanvas");
}

void AbstractCanvas::leaveEvent(QEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->clearMessage();
}

void AbstractCanvas::UpdateScaling() {}
void AbstractCanvas::OpenContextMenu(QPoint screen_pos, QPoint canvas_pos) {}
