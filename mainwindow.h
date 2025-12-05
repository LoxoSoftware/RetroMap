#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tile.h"
#include "tilepicker.h"
#include "paletteedit.h"
#include "toolboxpanel.h"
#include "ui_mainwindow.h"
#include <QScrollArea>

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
    void CheckCanvasPresence();
    void ChangeTileFormat(Tileset::tile_format_t format);

    TilePicker* dckTilePicker= nullptr;
    PaletteEdit* dckPaletteEdit= nullptr;
    ToolBoxPanel* dckToolbox= nullptr;

    bool isTilePicker_ViewSelPal() { return ui->actionTilePicker_selected_pal->isChecked(); }

    QScrollArea* NewTilemapTab();
    QScrollArea* NewTileEditTab(int tile_id);

public slots:
    void on_colorChanged();
    void on_actionTilePicker_selected_pal_triggered();

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
    void on_actionMapChange_Size_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionRedraw_canvas_triggered();
    void on_actionShow_tile_grid_triggered(bool checked);
    void on_actionExport_as_source_file_triggered();
    void on_actionAbout_triggered();
    void on_actionGBA_8bpp_triggered();
    void on_actionGBA_4bpp_triggered();
    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tabCloseRequested(int index);

private:
    Ui::MainWindow *ui;

    void resizeEvent(QResizeEvent* event);
};
#endif // MAINWINDOW_H
