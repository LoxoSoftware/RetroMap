#include "project.h"

Project::Project() {}

Project::~Project()
{
    delete editor_canvas;
}

int Project::SaveTo(QString fname)
{
    return 0;
}

int Project::LoadFrom(QString fname)
{
    return 0;
}
