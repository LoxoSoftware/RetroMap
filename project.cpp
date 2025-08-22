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

    editor_canvas= new Canvas(canvas_container, width_tiles, height_tiles);
    editor_canvas->UpdateHistory();

    project_fpath= "";
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
    if (tileset.image_fpath != "//clone//")
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


    if (tileset.image_fpath == "//clone//")
    {
        srand(time(NULL));
        QString suffix= "-tiles_"+QString::number(rand()%0x10000, 16)+".bmp";

        if (fname.lastIndexOf('.') > 0)
            tileset.image_fpath= fname.chopped(fname.size()-fname.lastIndexOf('.'))+suffix;
        else
            tileset.image_fpath= fname+suffix;
    }

    tileset.image->setColorTable(tileset.palette);
    tileset.image->save(tileset.image_fpath, "BMP");

    jobj.insert("tileset_source",tileset.image_fpath);
    jobj.insert("tileset_bpp", (tileset.is4bpp?"4":"8"));
    jobj.insert("tilemap_rows", QString::number(editor_canvas->Size().height()));
    jobj.insert("tilemap_columns", QString::number(editor_canvas->Size().width()));
    jobj.insert("tilemap_tiles", QJsonValue(jtilemap));

    QJsonDocument jdoc= QJsonDocument(jobj);

    QFile ofile= QFile(fname);
    ofile.open(QIODevice::WriteOnly);
    if (!ofile.isOpen())
    {
        QMessageBox::critical(canvas_container, "Cannot save project", "Cannot open output file for writing");
        return 4;
    }
    ofile.write(jdoc.toJson());
    ofile.close();

    project_fpath= fname;

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
        tileset.FromImage(QFileDialog::getOpenFileName(canvas_container, "Please locate missing tileset image", "", "Supported image formats (*.bmp)"), false);
    else
        tileset.FromImage(jdoc["tileset_source"].toString(), false);

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
    editor_canvas->UpdateHistory();

    project_fpath= fname;
    return 0;
}

int Project::ExportToSourceFile(QString fname)
{
    //Get file type from file extension
    if (fname.lastIndexOf('.') < 0 || fname.lastIndexOf('.') < fname.lastIndexOf('/'))
    {
        QMessageBox::critical(canvas_container, "Error - Export to source file", "Output file must have an extension");
        return 1;
    }
    QString ofmt= fname.last(fname.size()-fname.lastIndexOf('.')-1);
    QString oname;
    if (fname.lastIndexOf('/') >= 0)
        oname= fname.sliced(fname.lastIndexOf('/')+1, fname.lastIndexOf('.')-fname.lastIndexOf('/')-1);
    else
        oname= fname.first(fname.size()-ofmt.size()-1);

    oname.replace(" ", "_");
    QString obuf= "";

    if (ofmt == "s")
    {
        QFile ofile= QFile(fname);
        ofile.open(QIODeviceBase::WriteOnly);

        obuf+= "@{{BLOCK("+oname+")\n";
        obuf+= "\n@================================================";
        obuf+= "\n@      Exported by RetroMap by LoxoSoftware      ";
        obuf+= "\n@    https://github.com/LoxoSoftware/RetroMap    ";
        obuf+= "\n@================================================";
        obuf+= "\n\n";
        if (tileset.image)
        {
            obuf+= "\t.section .rodata\n\t.align 2\n\t.global "+oname+"Tiles\n";
            obuf+= oname+"Tiles:\n";
            for (int it=0; it<tileset.tiles.count(); it++)
            {
                foreach (QString tstr, TiledataToString(it, ofmt))
                    obuf+= tstr;
            }
        }
        if (editor_canvas)
        {
            if (editor_canvas->width() < 8)
                return 3;

            obuf+= "\n\t.section .rodata\n\t.align 2\n\t.global "+oname+"Map\n";
            obuf+= oname+"Map:\n";
            for (int it=0; it<editor_canvas->tiles.count(); it++)
            {
                Tile ttile= editor_canvas->tiles[it];
                unsigned int tdata= (ttile.tileset_offset)%0x0400+
                                    (ttile.hflip?0x0400:0)+
                                    (ttile.vflip?0x0800:0)+
                                    ((ttile.palette_index%16)*0x1000);
                if (it%8 == 0) obuf+= "\t.hword 0x";
                else           obuf+= ",0x";
                obuf+= QString::number(tdata, 16);
                if (it%8 == 7) obuf+= "\n";
            }
        }
        if (tileset.palette.count())
        {
            obuf+= "\n\t.section .rodata\n\t.align 2\n\t.global "+oname+"Pal\n";
            obuf+= oname+"Pal:\n";
            for (int ip=0; ip<tileset.palette.count(); ip++)
            {
                QColor tcol= tileset.palette[ip];
                unsigned int tdata=  tcol.red()/8+
                                     (tcol.green()/8)*32+
                                     (tcol.blue()/8)*1024;
                if (ip%8 == 0) obuf+= "\t.hword 0x";
                else           obuf+= ",0x";
                obuf+= QString::number(tdata, 16);
                if (ip%8 == 7) obuf+= "\n";
            }
        }
        obuf+= "\n@}}BLOCK("+oname+")";

        ofile.write(obuf.toLocal8Bit());
        ofile.close();
    }
    else if (ofmt == "c")
    {
        QMessageBox::critical(canvas_container, "Error - Export to C source file", "Not implemented yet :(");
        return 1;
    }
    else
    {
        QMessageBox::critical(canvas_container, "Error - Export to source file", "Invalid output file format \"."+ofmt+"\"");
        return 1;
    }

    return 0;
}

QVector<QString> Project::TiledataToString(int it, QString format)
{
    QVector<QString> ostrv;
    ostrv.clear();
    QString tstr;

    int imgposx= (it*TILE_W)%tileset.image->width();
    int imgposy= ((it*TILE_W)/tileset.image->width())*TILE_H;

    if (tileset.is4bpp)
    {
        //Assuming a tile is 32 bytes, so 2 rows of 8 16 bit words
        for (int iti=0; iti<8; iti+=4)
        {
            tstr= "";
            if (format == "s")
                tstr+= "\t.hword ";
            if (format == "c")
                tstr+= "\t";
            tstr+=
                "0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+0)))[imgposx/4+0]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+0)))[imgposx/4+1]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+1)))[imgposx/4+0]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+1)))[imgposx/4+1]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+2)))[imgposx/4+0]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+2)))[imgposx/4+1]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+3)))[imgposx/4+0]),16)+
                ",0x"+QString::number(TruncPal(((uint32_t*)(tileset.image->scanLine(iti+imgposy+3)))[imgposx/4+1]),16);
            if (format == "c")
                tstr+= ",";
            tstr+= "\n";
            ostrv+= tstr;
        }
    }
    else
    {
        //Assuming a tile is 64 bytes, so 4 rows of 8 16 bit words
        for (int iti=0; iti<8; iti+=2)
        {
            tstr= "";
            if (format == "s")
                tstr+= "\t.hword ";
            if (format == "c")
                tstr+= "\t";
            tstr+=
                "0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+0)))[imgposx/2+0],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+0)))[imgposx/2+1],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+0)))[imgposx/2+2],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+0)))[imgposx/2+3],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+1)))[imgposx/2+0],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+1)))[imgposx/2+1],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+1)))[imgposx/2+2],16)+
                ",0x"+QString::number(((uint16_t*)(tileset.image->scanLine(iti+imgposy+1)))[imgposx/2+3],16);
            if (format == "c")
                tstr+= ",";
            tstr+= "\n";
            ostrv+= tstr;
        }
    }

    return ostrv;
}

uint16_t Project::TruncPal(uint32_t n32)
{
    // 0xPCPCPCPC -> 0xCCCC (Palette, Color)
    return ((n32&0x0F000000)>>12)|((n32&0x000F0000)>>8)|((n32&0x00000F00)>>4)|(n32&0x0000000F);
}
