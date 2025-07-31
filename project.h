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
    int SaveTo(QString fname);
    int LoadFrom(QString fname);

    //Private members are for sigmas
    QWidget* canvas_container= nullptr;
    Canvas* editor_canvas= nullptr;
    Tileset tileset;
    int tileset_selected_tile= -1;
};

#endif // PROJECT_H
