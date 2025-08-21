#include "tile.h"
#include "project.h"
#include <QMessageBox>

extern Project project;

Tile::Tile() {}

Tile::Tile(int tileset_offset)
{
    this->tileset_offset= tileset_offset;
    this->hflip= false;
    this->vflip= false;
    this->palette_index= 0;
}

///// Tileset operations /////

bool Tileset::FromImage()
{
    //Returns true on success

    if (!image)
    {
        printf("DEBUG: tileset_image is null!\n");
        return false;
    }
    if (image->colorTable().isEmpty())
    {
        QMessageBox* msgb= new QMessageBox(QMessageBox::Critical, "Error! - Import tileset from image", "Image does not have a color palette");
        msgb->show();
        return false;
    }
    if (image->width()%TILE_W || image->height()%TILE_H)
    {
        QMessageBox* msgb= new QMessageBox(QMessageBox::Critical, "Error! - Import tileset from image",
                                            "Image size should be a multiple of "+QString::number(TILE_W)+"x"+QString::number(TILE_H));
        msgb->show();
        return false;
    }
    //Populate the pixmap array with 8x8 tiles from the image
    tiles.clear();
    for (int ity=0; ity<image->height()/TILE_H; ity++)
    {
        for (int itx=0; itx<image->width()/TILE_W; itx++)
        {
            tiles+= image->copy(itx*TILE_W, ity*TILE_H, TILE_W, TILE_H);
        }
    }
    //Populate the palette array with the one embedded in the image
    palette.clear();
    for (int ipy=0; ipy<PALETTE_H; ipy++)
    {
        for (int ipx=0; ipx<PALETTE_W; ipx++)
        {
            palette+= image->colorTable()[ipx+PALETTE_W*ipy];
        }
    }

    return true;
}

bool Tileset::FromImage(QString fname, bool load_new)
{
    if (image)
    {
        delete image;
        image= nullptr;
    }

    image= new QImage(fname);
    if (!image || image->format() != QImage::Format_Indexed8)
    {
        QMessageBox::critical(project.canvas_container, "Error! - Import tileset from image", "Image is invalid!");
        return false;
    }

    if (load_new)
    {
        QMessageBox::StandardButton dial_result= QMessageBox::question(project.canvas_container, "Question - Import tileset from image",
                              "It is reccomended that you create a copy of the image for the project.\r\nDo you wish to create a copy now?\r\nChoosing \"no\" will overwrite the original file");
        if (dial_result == QMessageBox::Yes)
            image_fpath= "//clone//";
        else
            image_fpath= fname;
    }
    else
        image_fpath= fname;

    return FromImage();
}

void Tileset::Optimize(Tileset::optimize_flags_t optiflags)
{
    QVector<QImage> new_tileset;
    new_tileset.clear();

    for (int it=0; it<tiles.count(); it++)
    {
        QImage tstile= tiles[it];
        QTransform ttrans;

        if (tstile.format() != QImage::Format_Indexed8)
            exit(12);

        int ind_found= new_tileset.indexOf(tstile);
        bool vflipped= false, hflipped= false;

        //Flip the tiles for all possible combinations and calculate the hash for each one
        if (optiflags & Tileset::OptimizeWithFlip && ind_found < 0)
        {
            ttrans.scale(-1,1);
            tstile= tstile.transformed(ttrans);
            ind_found= new_tileset.indexOf(tstile);
            if (ind_found >= 0)
                vflipped= false, hflipped= true;
        }
        if (optiflags & Tileset::OptimizeWithFlip && ind_found < 0)
        {
            ttrans.scale(1,-1);
            tstile= tstile.transformed(ttrans);
            ind_found= new_tileset.indexOf(tstile);
            if (ind_found >= 0)
                vflipped= true, hflipped= false;
        }
        if (optiflags & Tileset::OptimizeWithFlip && ind_found < 0)
        {
            ttrans.scale(-1,-1);
            tstile= tstile.transformed(ttrans);
            ind_found= new_tileset.indexOf(tstile);
            if (ind_found >= 0)
                vflipped= true, hflipped= true;
        }

        int new_offs;

        if (ind_found >= 0)
            new_offs= ind_found;
        else
        {
            new_tileset+= tiles[it];
            new_offs= new_tileset.count()-1;
        }

        //Update all occurrences of the tile in the tilemap
        for (int iit=0; iit<project.editor_canvas->tiles.count(); iit++)
        {
            Tile* ttile= &project.editor_canvas->tiles[iit];
            if (ttile->tileset_offset != it)
                continue;
            ttile->tileset_offset= new_offs;
            if (ind_found >= 0)
            {
                ttile->hflip= hflipped;
                ttile->vflip= vflipped;
            }

        }
    }

    tiles= new_tileset;
    RebuildTilesetImage(16);
}

void Tileset::RebuildTilesetImage(int columns)
{
    if (!tiles.count())
        return;

    QImage timg= QImage(QSize(columns*TILE_W,tiles.count()/columns*TILE_H+TILE_H), QImage::Format_Indexed8);
    timg.setColorTable(palette);
    timg.fill(240);

    for (int it=0; it<tiles.count(); it++)
    {
        QImage* ttile= &tiles[it];
        int final_tx= (it*TILE_W)%(columns*TILE_W);
        int final_ty= (it*TILE_W)/(columns*TILE_W)*TILE_H;

        for (int ity=0; ity<TILE_H; ity++)
        {
            unsigned char* slptr= ttile->scanLine(ity);

            for (int itx=0; itx<TILE_W; itx++)
            {
                timg.setPixel(final_tx+itx, final_ty+ity, slptr[itx]);
            }
        }
    }

    // if (image)
    //     delete image;
    image= new QImage(timg);
}

void Tileset::UpdatePalettes()
{
    for (int it=0; it<tiles.count(); it++)
    {
        tiles[it].setColorTable(palette);
    }
}
