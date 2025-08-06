#ifndef TILE_HPP
#define TILE_HPP

#include <QPoint>
#include <QPixmap>

#define PALETTE_W       16
#define PALETTE_H       16
#define PALETTE_SIZE    (PALETTE_W*PALETTE_H)
#define TILE_W          8
#define TILE_H          8

class Tile
{
public:
    Tile();
    Tile(int tileset_offset);
    // Tile(int tileset_offset, int x, int y);
    // Tile(int tileset_offset, QPoint pos);
    // int     x, y;           //In tiles, not pixels, of course
    bool    hflip=false;
    bool    vflip=false;
    int     tileset_offset;
    int     palette_index;
};

class Tileset
{
public:
    QImage*         image;
    QString         image_fpath;
    QVector<QImage> tiles;
    QVector<QRgb>   palette;
    bool            is4bpp= false;

    enum optimize_flags_t {
        OptimizeDefault = 0,
        OptimizeWithFlip = 1,
        OptimizeWithPalette = 2,
    };

    bool            FromImage();
    bool            FromImage(QString fname);
    void            Optimize(Tileset::optimize_flags_t optiflags=Tileset::OptimizeDefault);
};

#endif // TILE_HPP
