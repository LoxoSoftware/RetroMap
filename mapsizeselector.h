#ifndef MAPSIZESELECTOR_H
#define MAPSIZESELECTOR_H

#include <QWidget>
#include <qmessagebox.h>

namespace Ui {
class MapSizeSelector;
}

class MapSizeSelector : public QWidget
{
    Q_OBJECT

public:
    explicit MapSizeSelector(QWidget *parent = nullptr);
    ~MapSizeSelector();

private slots:
    void on_cmbPresetSize_currentIndexChanged(int index);
    void on_bnbDialogButtons_rejected();
    void on_bnbDialogButtons_accepted();

private:
    Ui::MapSizeSelector *ui;
};

#endif // MAPSIZESELECTOR_H
