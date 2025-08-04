#include "canvas.h"
#include "project.h"
#include <QMenu>
#include <QMouseEvent>
#include <QAction>

extern Project project;

#define CANVASX_TO_COLUMN(x)    (x/TILE_W/scaling)
#define CANVASY_TO_ROW(y)       (y/TILE_H/scaling)

Canvas::Canvas(QWidget* parent, int width, int height)
{
    size= QSize(width, height);
    setParent(parent);
    setScene(&scene);
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
    Redraw();
}

void Canvas::PlotUnscaled(QPoint pos, Tile tile)
{
    Plot((pos.y()/TILE_H)/scaling, (pos.x()/TILE_W)/scaling, tile);
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
        //Tile is outside the bounds of the tileset
        scene.addRect(column*TILE_W*scaling, row*TILE_H*scaling, TILE_W*scaling, TILE_H*scaling, pen, bru);
    }
    else
    {
        QPixmap pix= QPixmap::fromImage(project.tileset.tiles[ttile->tileset_offset]);
        QTransform trans= QTransform();
        trans= trans.scale((ttile->hflip?-1:1), (ttile->vflip?-1:1));
        pix= pix.transformed(trans);
        QGraphicsPixmapItem* item= new QGraphicsPixmapItem(pix);
        item->setX(column*TILE_W*scaling);
        item->setY(row*TILE_H*scaling);
        item->setScale(scaling);
        scene.addItem(item);
    }
}

void Canvas::UpdateScaling()
{
    setMinimumSize(size.width()*TILE_W*scaling+(CANVAS_BORDER_W*2), size.height()*TILE_H*scaling+(CANVAS_BORDER_W*2));
    setMaximumSize(size.width()*TILE_W*scaling+(CANVAS_BORDER_W*2), size.height()*TILE_H*scaling+(CANVAS_BORDER_W*2));
    scene.setSceneRect(QRect(0,0,size.width()*TILE_W*scaling,size.height()*TILE_H*scaling));
    // foreach (QGraphicsItem* item, scene.items())
    // {
    //     item->setScale(scaling);
    // }
}

void Canvas::OpenContextMenu(QPoint screen_pos, QPoint canvas_pos)
{
    int tilex= (canvas_pos.x()/TILE_W)/scaling;
    int tiley= (canvas_pos.y()/TILE_H)/scaling;
    int tilen= tilex+tiley*size.width();

    QMenu* menu= new QMenu();
    menu->addAction("Clear tile with background");
    connect(menu->actions().last(), &QAction::triggered, this, &Canvas::onMenuClearWithBgTile_triggered);
    QMenu* menu_palette= menu->addMenu("Change palette index");
    for (int i=0; i<PALETTE_H; i++)
        menu_palette->addAction(""+QString::number(i));
    menu_palette->setEnabled(project.tileset.is4bpp);
    menu->addAction("Flip tile horizontally");
    menu->actions().last()->setCheckable(true);
    menu->actions().last()->setChecked(tiles[tilen].hflip);
    connect(menu->actions().last(), &QAction::triggered, this, &Canvas::onMenuHFlip_triggered);
    menu->addAction("Flip tile vertically");
    menu->actions().last()->setCheckable(true);
    menu->actions().last()->setChecked(tiles[tilen].vflip);
    connect(menu->actions().last(), &QAction::triggered, this, &Canvas::onMenuVFlip_triggered);
    menu->addSeparator();
    QString stat_lbl= "pos: ["+QString::number(tilex)+","+ QString::number(tiley)+"]"
                    " flip: ["+((tiles[tilen].hflip)?"H ":"- ")+((tiles[tilen].vflip)?"V":"-")+"]";
    menu->addAction(stat_lbl)->setEnabled(false);

    menu->setWindowModality(Qt::ApplicationModal);
    menu->setGeometry(QRect(screen_pos,QSize(180,140)));
    menu->show();
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    //event->accept();
    mouse_down_button= event->button();
    if (mouse_down_button != Qt::RightButton)
        mouseMoveEvent(event);
    mouse_has_moved= false;
    mouse_last_pos= event->pos();
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
    Tile ttile= Tile();
    ttile.tileset_offset= project.tileset_selected_tile;

    int tilex= CANVASX_TO_COLUMN(event->pos().x());
    int tiley= CANVASY_TO_ROW(event->pos().y());

    switch (mouse_down_button)
    {
    case Qt::LeftButton:
        if (project.tileset_selected_tile < 0)
            return;
        Plot(tiley, tilex, ttile);
        RedrawTile(tiley, tilex);
        break;
    case Qt::RightButton:
        ttile.tileset_offset= 0;
        ttile.hflip= false;
        ttile.vflip= false;
        ttile.palette_index= 0;
        Plot(tiley, tilex, ttile);
        RedrawTile(tiley, tilex);
        break;
    default:
        break;
    }
    mouse_has_moved= true;
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    if (!mouse_has_moved && mouse_down_button == Qt::RightButton)
        OpenContextMenu(event->globalPos(), event->pos());

    mouse_down_button= Qt::NoButton;
    mouse_has_moved= false;
}

void Canvas::onMenuClearWithBgTile_triggered()
{
    Tile ttile;
    ttile.tileset_offset= 0;
    ttile.hflip= false;
    ttile.vflip= false;
    ttile.palette_index= 0;
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Plot(yt, xt, ttile);
    RedrawTile(yt, xt);
}

void Canvas::onMenuHFlip_triggered()
{
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Tile* tile= &tiles[xt+yt*size.width()];
    tile->hflip = !tile->hflip;
    RedrawTile(yt, xt);
}

void Canvas::onMenuVFlip_triggered()
{
    int xt= CANVASX_TO_COLUMN(mouse_last_pos.x());
    int yt= CANVASY_TO_ROW(mouse_last_pos.y());
    Tile* tile= &tiles[xt+yt*size.width()];
    tile->vflip = !tile->vflip;
    RedrawTile(yt, xt);
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

                    if (!tiles[tm_ti].hflip)
                        timg_scanline[ix+ixt*TILE_W]= pixel;
                    else
                        timg_scanline[TILE_W-(ix+1)+ixt*TILE_W]= pixel;
                }
            }
        }

    return timg;
}
