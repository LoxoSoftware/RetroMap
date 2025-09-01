#ifndef PALETTEEDIT_H
#define PALETTEEDIT_H

#include <QDockWidget>

class MainWindow;

namespace Ui {
class PaletteEdit;
}

class PaletteEdit : public QDockWidget
{
    Q_OBJECT

public:
    explicit PaletteEdit(QWidget *parent, MainWindow* main_window);
    ~PaletteEdit();

    void Update();
    void UpdateColorStatus(bool force=true);

    bool isBlockingPalUpdates() { return block_pal_updates; }

private slots:
    void on_tblPalette_cellClicked(int row, int column);

private:
    Ui::PaletteEdit *ui;
    bool block_pal_updates= false;
    MainWindow* main_window;
};

#endif // PALETTEEDIT_H
