#ifndef TILEPICKER_H
#define TILEPICKER_H

#include <QDockWidget>
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

    void resizeEvent(QResizeEvent* event);

public slots:
    void on_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_actionTilePicker_selected_pal_triggered();

private slots:
    void on_tblTiles_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    Ui::TilePicker *ui;
    MainWindow* main_window;
};

#endif // TILEPICKER_H
