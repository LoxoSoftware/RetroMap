#include "project.h"
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QImage>
#include <QFileDialog>

Project::Project()
{
    tileset_selected_tile= -1;
}

Project::~Project() {}

int Project::CreateNew(int width_tiles, int height_tiles)
{
    if (editor_canvas)
        delete editor_canvas;

    editor_canvas= new Canvas(canvas_container, 32, 32);
    return 0;
}

int Project::SaveToFile(QString fname)
{
    if (!editor_canvas)
        return 1;
    if (!tileset.image)
    {
        QMessageBox::critical(canvas_container, "Cannot save project", "No tileset image is loaded");
        return 2;
    }
    if (!QFile::exists(tileset.image_fpath))
    {
        QMessageBox::critical(canvas_container, "Cannot save project", "Tileset image file is invalid");
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

    tileset.image->setColorTable(tileset.palette);
    tileset.image->save(tileset.image_fpath, "bmp");

    QFile ofile= QFile(fname);
    ofile.open(QIODevice::WriteOnly);
    if (!ofile.isOpen())
    {
        QMessageBox::critical(canvas_container, "Cannot save project", "Cannot open output file for writing");
        return 4;
    }
    ofile.write(jdoc.toJson());

    return 0;
}

int Project::LoadFromFile(QString fname)
{
    QFile ifile= QFile(fname);
    ifile.open(QIODevice::ReadOnly);
    if (!ifile.isOpen())
    {
        QMessageBox::critical(canvas_container, "Cannot save project", "Cannot open input file for reading");
        return 1;
    }
    QByteArray ifile_data= ifile.readAll();

    QJsonDocument jdoc= QJsonDocument::fromJson(ifile_data);
    QJsonArray jtilemap= jdoc["tilemap_tiles"].toArray();

    CreateNew(jdoc["tilemap_columns"].toString().toInt(), jdoc["tilemap_rows"].toString().toInt());

    if (!QFile::exists(jdoc["tileset_source"].toString()))
        tileset.FromImage(QFileDialog::getOpenFileName(canvas_container, "Please locate missing tileset image", "", "Supported image formats (*.bmp)"));
    else
        tileset.FromImage(jdoc["tileset_source"].toString());

    tileset.is4bpp= (jdoc["tileset_bpp"].toString() == "4"? true:false);

    for (int iy=0; iy<editor_canvas->Size().height(); iy++)
    {
        for (int ix=0; ix<editor_canvas->Size().width(); ix++)
        {
            Tile ttile;
            unsigned int tdata= jtilemap[ix+iy*editor_canvas->Size().width()].toInt();
            ttile.tileset_offset=   (tdata)%0x0400;
            ttile.hflip=            (tdata/0x0800)%2;
            ttile.vflip=            (tdata/0x1000)%2;
            ttile.palette_index=    (tdata/0x2000)%16;
            editor_canvas->Plot(iy, ix, ttile);
        }
    }

    editor_canvas->Redraw();

    return 0;
}
