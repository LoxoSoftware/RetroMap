#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void UpdateTilesetTable();
    void UpdatePaletteTable();

private slots:
    void on_actionQuit_triggered();
    void on_actionNew_triggered();
    void on_actionZoom_in_triggered();
    void on_actionZoom_out_triggered();
    void on_btnImportTilesetImage_clicked();

    void on_tblTiles_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_action16_color_mode_changed();

    void on_actionSave_triggered();

    void on_actionSave_as_triggered();

private:
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent *event);
};
#endif // MAINWINDOW_H
