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
    void on_tblTiles_cellDoubleClicked(int row, int column);

private:
    Ui::TilePicker *ui;
    MainWindow* main_window;

#if QT_VERSION_MAJOR > 5
    void enterEvent(QEnterEvent* event);
#else
    void enterEvent(QEvent* event);
#endif
    void leaveEvent(QEvent* event);
};

#endif // TILEPICKER_H
