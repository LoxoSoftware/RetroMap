#ifndef TILEPICKER_H
#define TILEPICKER_H

#include <QDockWidget>
#include <QTableWidgetItem>
class MainWindow;

namespace Ui {
class TilePicker;
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

private:
    Ui::TilePicker *ui;
    MainWindow* main_window;
};

#endif // TILEPICKER_H
