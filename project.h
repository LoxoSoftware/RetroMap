#ifndef PROJECT_H
#define PROJECT_H

#include "canvas.h"
#include "tile.h"
#include <QString>

class Project
{
public:
    Project();
    ~Project();

    int SaveTo(QString fname);
    int LoadFrom(QString fname);

    Canvas* editor_canvas= nullptr;
    Tileset tileset;
    int tileset_selected_tile= -1;
};

#endif // PROJECT_H
