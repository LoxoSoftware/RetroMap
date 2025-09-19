#ifndef PROJECT_H
#define PROJECT_H

#include "canvas.h"
#include "tile.h"
#include <QString>
#include <QWidget>
#include <QStatusBar>

class Project
{
public:
    Project();
    ~Project();

    int CreateNew(int width_tiles, int height_tiles);
    int SaveToFile(QString fname);
    int LoadFromFile(QString fname);
    int ExportToSourceFile(QString fname, int export_flags= ExportDefault|ExportGBA8bpp);

    QString         project_fpath= "";
    QScrollArea*    canvas_container= nullptr;
    Canvas*         editor_canvas= nullptr;
    Tileset         tileset;
    int             tileset_selected_tile= -1;
    int             tileset_selected_bgtile= 0;
    int             paltable_current_row= 0;
    int             paltable_current_column= 0;
    int             selected_tools;
    int             pen_size= 1;

    static const int ExportNone=        0;
    static const int ExportGfx=         1;
    static const int ExportMap=         1<<1;
    static const int ExportPal=         1<<2;
    static const int ExportHFile=       1<<3;
    static const int ExportOptimize=    1<<4;
    static const int ExportDefault=     ExportGfx|ExportMap|ExportPal|ExportHFile|ExportOptimize;
    static const int ExportAll=         ExportGfx|ExportMap|ExportPal;
    static const int ExportFormat=      5; //Used for bit shifting
    static const int ExportGBA4bpp=     0<<ExportFormat;
    static const int ExportGBA8bpp=     1<<ExportFormat;
    static const int ExportGBAAffine=   2<<ExportFormat;

    QStatusBar* statusbar= nullptr;

private:
    QVector<QString>    TiledataToString(int it, Tileset* out_tileset, QString format, int export_flags);
    QVector<QString>    MapdataToString(QList<Tile>* out_map, QString format, int export_flags);
    uint16_t            TruncPal(uint32_t n32);
};

#endif // PROJECT_H
