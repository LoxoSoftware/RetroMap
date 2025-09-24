#ifndef TILE_HPP
#define TILE_HPP

#include <QPoint>
#include <QPixmap>

#define PALETTE_W       16
#define PALETTE_H       16
#define PALETTE_SIZE    (PALETTE_W*PALETTE_H)
#define TILE_W          8
#define TILE_H          8

class Tileset;

class Tile
{
public:
    Tile(int offs=0, bool vflip=false, bool hflip=false, int palid=0);
    bool    hflip=false;
    bool    vflip=false;
    int     tileset_offset;
    int     palette_index;

    QImage  RenderImage(Tileset* tileset, bool bpp4);

    static QImage TransformImage(QImage img, bool vflip, bool hflip, int palid=-1);
};

class Tileset
{
public:
    typedef int optimize_flags_t;

    const static int OptimizeNone =         0;
    const static int OptimizeWithFlip =     1;
    const static int OptimizeWithPalette =  2;
    const static int OptimizeKeepUnused =   4;

    enum tile_format_t {
        GBA_8bpp = 1,
        GBA_4bpp = 2,
    };

    QImage*         image;
    QString         image_fpath;
    QVector<QImage> tiles;
    QVector<QRgb>   palette;
    tile_format_t   format= GBA_8bpp;

    bool            FromImage(int count);
    bool            FromImage(QString fname, bool load_new=false, int count=-1);
    void            UpdatePalettes();
    QVector<QImage> Optimized(QList<Tile>* tilemap, Tileset::optimize_flags_t optiflags);
    void            RebuildTilesetImage(int columns=16);

    bool            isPalettedFormat();
};

#endif // TILE_HPP
