#ifndef OPTIMIZEDIALOG_H
#define OPTIMIZEDIALOG_H

#include <QDialog>
#include "tile.h"

namespace Ui {
class OptimizeDialog;
}

class OptimizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptimizeDialog(QWidget *parent = nullptr, bool isPalettedFmt=true);
    ~OptimizeDialog();

    Tileset::optimize_flags_t GetFlags(bool isPalettedFmt=true);

private slots:
    void on_bnbDialog_accepted();
    void on_bnbDialog_rejected();

private:
    Ui::OptimizeDialog *ui;

    bool accepted= false;
};

#endif // OPTIMIZEDIALOG_H
