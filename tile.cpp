#include "tile.h"
#include "project.h"
#include <QMessageBox>

extern Project project;

Tile::Tile(int offs, bool vflip, bool hflip, int palid)
{
    this->tileset_offset= offs;
    this->hflip= hflip;
    this->vflip= vflip;
    this->palette_index= palid;
}

QImage Tile::RenderImage(Tileset* tileset, bool bpp4)
{
    QImage timg= tileset->tiles[tileset_offset];
    QImage oimg= timg;

    for (int iy=0; iy<TILE_H; iy++)
    {
        for (int ix=0; ix<TILE_W; ix++)
        {
            unsigned char pixel;

            if (bpp4)
                pixel= (timg.scanLine(iy)[ix]&0x0F)|((palette_index%16)<<4);
            else
                pixel= timg.scanLine(iy)[ix];

            oimg.scanLine(vflip?TILE_H-iy-1:iy)[hflip?TILE_W-ix-1:ix]= pixel;
        }
    }

    return oimg;
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
    for (int ic=0; ic<PALETTE_W*PALETTE_H; ic++)
    {
        if (ic < image->colorCount())
            palette+= image->colorTable()[ic];
        else
        {
            int randval= rand()%256;
            palette+= QColor::fromRgb(randval, randval, randval).rgb();
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
    if (optiflags == Tileset::OptimizeNone)
        return;

    QVector<QImage> new_tileset;
    new_tileset.clear();

    for (int it=0; it<tiles.count(); it++)
    {
        QImage tstile= tiles[it];
        QTransform ttrans;

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
    RebuildTilesetImage();
}

QVector<QImage> Tileset::Unoptimized(QList<Tile>* tilemap)
{
    QVector<QImage> new_tileset;
    QList<Tile> new_tilemap;
    new_tileset.clear();
    new_tilemap.clear();

    QList<bool> tilemap_matchstatus;
    for (int i=0; i<project.editor_canvas->tiles.count(); i++)
        tilemap_matchstatus+= false,
        new_tilemap+= Tile(-1);

    for (int it= 0; it< tiles.count(); it++)
    {
        QImage tstile= tiles[it];

        //Precompute all the possible versions of this tile
        QImage tile_combos[PALETTE_H*4];
        for (int itc= 0; itc< PALETTE_H*4; itc+=4)
        {
            tile_combos[itc+0]= Tile(it,false,false,itc>>2).RenderImage(this,true);
            tile_combos[itc+1]= Tile(it,true,false,itc>>2).RenderImage(this,true);
            tile_combos[itc+2]= Tile(it,false,true,itc>>2).RenderImage(this,true);
            tile_combos[itc+3]= Tile(it,true,true,itc>>2).RenderImage(this,true);
        }

        //Check for every tile in the map if it matches with at least one of the precomputed versions of this tile
        for (int im=0; im<tilemap->count(); im++)
        {
            if (tilemap_matchstatus[im]) //There is another tile that has already matched, no need to check again
                continue;

            QImage tmtile= (*tilemap)[im].RenderImage(this,format==Tileset::GBA_4bpp);

            for (int itc= 0; itc< sizeof(tile_combos)/sizeof(*tile_combos); itc++)
            {
                if (tmtile != tile_combos[itc])
                    continue;

                for (int ints= 0; ints<new_tileset.count(); ints++)
                {
                    if (new_tileset[ints] == tile_combos[itc])
                    {
                        new_tilemap[im]= Tile(ints,false,false,itc>>2);
                        tilemap_matchstatus[im]= true;
                        break;
                    }
                }
                if (tilemap_matchstatus[im])
                    break;

                new_tileset+= tile_combos[itc];
                new_tilemap[im]= Tile(new_tileset.count()-1,false,false,itc>>2);
                tilemap_matchstatus[im]= true;
                break;
            }
        }
    }

    for (int im=0; im<new_tilemap.count(); im++)
        (*tilemap)[im]= new_tilemap[im];
    return new_tileset;
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
