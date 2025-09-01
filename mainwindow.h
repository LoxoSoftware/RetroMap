#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tile.h"
#include "tilepicker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void CheckCanvasPresent();
    void UpdatePaletteTable();
    void UpdateToolStatus();
    void UpdateColorStatus(bool force=true);

    static const int tool_NoTool = 0;
    static const int tool_OffsetPen = 1;
    static const int tool_VFlipPen = 2;
    static const int tool_HFlipPen = 4;
    static const int tool_PalettePen = 8;

    void ChangeTileFormat(Tileset::tile_format_t format);

    TilePicker* dckTilePicker= nullptr;

    bool opt_tilePicker_view_sel_pal();

private slots:
    void on_actionQuit_triggered();
    void on_actionNew_triggered();
    void on_actionZoom_in_triggered();
    void on_actionZoom_out_triggered();
    void on_actionSave_triggered();
    void on_actionSave_as_triggered();
    void on_actionLoad_triggered();
    void on_actionImport_tileset_from_image_triggered();
    void on_actionExport_as_indexed_bitmap_triggered();
    void on_actionOptimize_tileset_triggered();
    void on_tblPalette_cellClicked(int row, int column);
    void on_tlbPen_clicked(bool checked);
    void on_tlbVFlipPen_clicked(bool checked);
    void on_tlbPalettePen_clicked(bool checked);
    void on_tlbHFlipPen_clicked(bool checked);
    void on_btnSize1_clicked(bool checked);
    void on_btnSize2_clicked(bool checked);
    void on_btnSize3_clicked(bool checked);
    void on_actionMapChange_Size_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_colorChanged();
    void on_actionRedraw_canvas_triggered();
    void on_actionShow_tile_grid_triggered(bool checked);
    void on_actionExport_as_source_file_triggered();
    void on_actionAbout_triggered();
    void on_actionGBA_8bpp_triggered();
    void on_actionGBA_4bpp_triggered();
    void on_actionTilePicker_selected_pal_triggered();

private:
    Ui::MainWindow *ui;
    bool block_pal_updates= false;
};
#endif // MAINWINDOW_H
