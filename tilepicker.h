#ifndef TILEPICKER_H
#define TILEPICKER_H

#include <QDockWidget>
#include <QTableWidgetItem>
class MainWindow;

namespace Ui {
class TilePicker;
class TilePickerTable;
}

class TilePicker : public QDockWidget
{
    Q_OBJECT

public:
    explicit TilePicker(QWidget *parent, MainWindow* main_window);
    ~TilePicker();

    void Update();
    void RedrawSelection();

    void resizeEvent(QResizeEvent* event);

private slots:
    void on_tblTiles_cellClicked(int row, int column);
    void on_tblTiles_customContextMenuRequested(const QPoint &pos);
    void on_tblTiles_cellDoubleClicked(int row, int column);
    void on_changeBgSelection();
    void on_editTileTriggered();
    void on_duplicateTileTriggered();
    void on_deleteTileTriggered();

private:
    Ui::TilePicker *ui;
    MainWindow* main_window;
    QMenu* context_menu= nullptr;
    int tile_hovered= 0;

    void OpenContextMenu(QPoint screen_pos, int tile_id);
#if QT_VERSION_MAJOR > 5
    void enterEvent(QEnterEvent* event);
#else
    void enterEvent(QEvent* event);
#endif
    void leaveEvent(QEvent* event);
};

class TilePickerTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit TilePickerTable(QWidget* parent);

private slots:
    void on_itemPressed(QTableWidgetItem* item);

private:
    Ui::TilePickerTable *ui;
    QTableWidgetItem* dnd_start_cell= nullptr;
    QRect dnd_answer_rect;

    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
};

#endif // TILEPICKER_H
