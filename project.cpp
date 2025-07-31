#include "project.h"
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QImage>

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
    if (!editor_canvas)
        return 1;
    if (!tileset.image)
    {
        QMessageBox::critical(nullptr, "Cannot save project", "No tileset image is loaded");
        return 2;
    }
    if (!QFile::exists(tileset.image_fpath))
    {
        QMessageBox::critical(nullptr, "Cannot save project", "Tileset image file is invalid");
        return 3;
    }

    QJsonObject jobj= QJsonObject();
    QJsonArray jtilemap= QJsonArray();

    foreach (Tile ttile, editor_canvas->tiles)
    {
        //Save in GBA format
        jtilemap+=  (ttile.tileset_offset)%0x0400+
                    (ttile.hflip?0x0800:0)+
                    (ttile.vflip?0x1000:0)+
                    ((ttile.palette_index%16)*0x2000);
    }

    jobj.insert("tileset_source",tileset.image_fpath);
    jobj.insert("tileset_bpp", (tileset.is4bpp?"4":"8"));
    jobj.insert("tilemap_rows", QString::number(editor_canvas->Size().height()));
    jobj.insert("tilemap_columns", QString::number(editor_canvas->Size().width()));
    jobj.insert("tilemap_tiles", QJsonValue(jtilemap));

    QJsonDocument jdoc= QJsonDocument(jobj);

    QImage oimage= QImage(*tileset.image);
    oimage.setColorTable(tileset.palette);
    oimage.save(tileset.image_fpath);

    QFile ofile= QFile(fname);
    ofile.open(QIODevice::WriteOnly);
    ofile.write(jdoc.toJson());

    return 0;
}

int Project::LoadFrom(QString fname)
{
    return 0;
}
