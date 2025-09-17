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

QImage Tile::TransformImage(QImage img, bool vflip, bool hflip, int palid)
{
    QImage oimg;
    QTransform ttrans;
    ttrans.scale(hflip?-1:1, vflip?-1:1);
    oimg= img.transformed(ttrans);

    if (palid >= 0)
    {
        for (int iy=0; iy<img.height(); iy++) for (int ix=0; ix<img.width(); ix++)
            oimg.scanLine(iy)[ix]= ((palid%16)<<4)|(oimg.scanLine(iy)[ix]&0x0F);
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

QVector<QImage> Tileset::Optimized(QList<Tile>* tilemap, Tileset::optimize_flags_t optiflags)
{
    QVector<QImage> new_tileset;
    QVector<QImage> temp_tileset;
    QList<Tile> new_tilemap;
    new_tileset.clear();
    temp_tileset.clear();
    new_tilemap.clear();

    for (int im=0; im<tilemap->count(); im++)
    {
        Tile* tmtile= &(*tilemap)[im];
        QImage tmtile_img= tmtile->RenderImage(this,true);
        bool hflipped=false, vflipped=false;
        int ind_found= -2;

        //Search the new tilemap by all the possible versions of this tile
        for (int ipal=0; ipal<PALETTE_H; ipal++)
        {
            if (!(optiflags&Tileset::OptimizeWithPalette))
                if (ipal != tmtile->palette_index) continue;

            hflipped=false, vflipped=false;
            ind_found= new_tileset.indexOf(Tile::TransformImage(tmtile_img,vflipped,hflipped,ipal));
            if (ind_found >= 0) break;

            if (!(optiflags&Tileset::OptimizeWithFlip)) continue;

            hflipped=false, vflipped=true;
            ind_found= new_tileset.indexOf(Tile::TransformImage(tmtile_img,vflipped,hflipped,ipal));
            if (ind_found >= 0) break;
            hflipped=true, vflipped=false;
            ind_found= new_tileset.indexOf(Tile::TransformImage(tmtile_img,vflipped,hflipped,ipal));
            if (ind_found >= 0) break;
            hflipped=true, vflipped=true;
            ind_found= new_tileset.indexOf(Tile::TransformImage(tmtile_img,vflipped,hflipped,ipal));
            if (ind_found >= 0) break;
        }

        if (ind_found >= 0)
        {
            new_tilemap+= Tile(ind_found,vflipped,hflipped,tmtile->palette_index);
        }
        else
        {
            new_tileset+= tmtile_img;
            new_tilemap+= Tile(new_tileset.count()-1,false,false,new_tileset.last().scanLine(0)[0]>>4);
        }

        (*tilemap)[im]= new_tilemap[im];
    }

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
