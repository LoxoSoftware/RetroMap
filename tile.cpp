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
    for (int ipy=0; ipy<PALETTE_H; ipy++)
    {
        for (int ipx=0; ipx<PALETTE_W; ipx++)
        {
            palette+= image->colorTable()[ipx+PALETTE_W*ipy];
        }
    }

    return true;
}

bool Tileset::FromImage(QString fname)
{
    image= new QImage(fname);
    if (!image)
    {
        printf("Cannot create QPixmap from image file!\n");
        return false;
    }
    image_fpath= fname;

    return FromImage();
}

// void Tileset::Optimize(Tileset::optimize_flags_t optiflags)
// {
//     QVector<unsigned long> hashes;
//     QVector<QImage> new_tileset;
//     hashes.clear();
//     new_tileset.clear();

//     for (int it=0; it<tiles.count(); it++)
//     {
//         QImage tstile= tiles[it];
//         QTransform ttrans;

//         if (tstile.format() != QImage::Format_Indexed8)
//             exit(12);

//         unsigned long gfx_hash= img_hash(&tstile);
//         int ind_found= ind_found= hashes.indexOf(gfx_hash);
//         bool vflipped= false, hflipped= false;

//         //Flip the tiles for all possible combinations and calculate the hash for each one
//         if (optiflags & Tileset::OptimizeWithFlip && ind_found < 0)
//         {
//             ttrans.scale(-1,1);
//             tstile= tstile.transformed(ttrans);
//             gfx_hash= img_hash(&tstile);
//             ind_found= hashes.indexOf(gfx_hash);
//             vflipped= false;
//             hflipped= true;
//         }
//         if (optiflags & Tileset::OptimizeWithFlip && ind_found < 0)
//         {
//             ttrans.scale(1,-1);
//             tstile= tstile.transformed(ttrans);
//             gfx_hash= img_hash(&tstile);
//             ind_found= hashes.indexOf(gfx_hash);
//             vflipped= true;
//             hflipped= false;
//         }
//         if (optiflags & Tileset::OptimizeWithFlip && ind_found < 0)
//         {
//             ttrans.scale(-1,-1);
//             tstile= tstile.transformed(ttrans);
//             gfx_hash= img_hash(&tstile);
//             ind_found= hashes.indexOf(gfx_hash);
//             vflipped= true;
//             hflipped= true;
//         }

//         if (ind_found >= 0)
//         {
//             //TODO: update all occurrences of the tile in the tilemap
//             continue;
//         }
//         else
//         {
//             hashes+= gfx_hash;
//             new_tileset+= tiles[it];
//             continue;
//         }
//     }

//     tiles= new_tileset;
// }

void Tileset::Optimize(Tileset::optimize_flags_t optiflags)
{
    QVector<unsigned long> hashes;
    QVector<QImage> new_tileset;
    hashes.clear();
    new_tileset.clear();

    for (int it=0; it<tiles.count(); it++)
    {
        QImage tstile= tiles[it];
        QTransform ttrans;

        if (tstile.format() != QImage::Format_Indexed8)
            exit(12);

        int ind_found= ind_found= new_tileset.indexOf(tstile);
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
}

unsigned long Tileset::img_hash(QImage* srcimg)
{
    unsigned long sum= 0;

    for (int iy=0; iy<srcimg->height(); iy++)
    {
        unsigned char* slptr= srcimg->scanLine(iy);

        for (int ix=0; ix<srcimg->width(); ix++)
            sum += slptr[ix]*((iy*(PALETTE_SIZE*2)*TILE_W)*(ix*8));
    }

    return sum;
}
