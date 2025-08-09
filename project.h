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
    int ExportToSourceFile(QString fname);

    //Private members are for sigmas
    QString     project_fpath= "";
    QWidget*    canvas_container= nullptr;
    Canvas*     editor_canvas= nullptr;
    Tileset     tileset;
    int         tileset_selected_tile= -1;
    int         paltable_current_row= 0;
    int         paltable_current_column= 0;
};

#endif // PROJECT_H
