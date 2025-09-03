#ifndef PROJECT_H
#define PROJECT_H

#include "canvas.h"
#include "tile.h"
#include <QString>
#include <QWidget>

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
    static const int ExportDefault=     ExportGfx|ExportMap|ExportPal|ExportHFile;
    static const int ExportAll=         ExportGfx|ExportMap|ExportPal;
    static const int ExportFormat=      4; //Used for bit shifting
    static const int ExportGBA4bpp=     0<<4;
    static const int ExportGBA8bpp=     1<<4;

private:
    QVector<QString>    TiledataToString(int it, QString format, int export_flags);
    uint16_t            TruncPal(uint32_t n32);
};

#endif // PROJECT_H
