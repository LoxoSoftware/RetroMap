#include "tile.h"
#include <QMessageBox>

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
            palette[ipy][ipx]= image->colorTable()[ipx+PALETTE_W*ipy];
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
    return FromImage();
}
