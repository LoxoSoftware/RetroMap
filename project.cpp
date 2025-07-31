#include "project.h"

Project::Project()
{
    tileset_selected_tile= -1;
}

Project::~Project() {}

int Project::CreateNew(int width_tiles, int height_tiles)
{
    editor_canvas= new Canvas(canvas_container, 32, 32);
    return 0;
}

int Project::SaveTo(QString fname)
{
    return 0;
}

int Project::LoadFrom(QString fname)
{
    return 0;
}
