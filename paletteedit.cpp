#include "paletteedit.h"
#include "ui_paletteedit.h"
#include "tile.h"
#include "project.h"
#include "mainwindow.h"

extern Project project;

PaletteEdit::PaletteEdit(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::PaletteEdit)
{
    ui->setupUi(this);
    this->main_window= main_window;

    ui->tblPalette->setCurrentCell(0,0);

    connect(ui->sliRedChannel, &QSlider::valueChanged, main_window, &MainWindow::on_colorChanged);
    connect(ui->sliBlueChannel, &QSlider::valueChanged, main_window, &MainWindow::on_colorChanged);
    connect(ui->sliGreenChannel, &QSlider::valueChanged, main_window, &MainWindow::on_colorChanged);
    connect(ui->spbRedChannel, QOverload<int>::of(&QSpinBox::valueChanged), main_window, &MainWindow::on_colorChanged);
    connect(ui->spbBlueChannel, QOverload<int>::of(&QSpinBox::valueChanged), main_window, &MainWindow::on_colorChanged);
    connect(ui->spbGreenChannel, QOverload<int>::of(&QSpinBox::valueChanged), main_window, &MainWindow::on_colorChanged);
}

PaletteEdit::~PaletteEdit()
{
    delete ui;
}

void PaletteEdit::Update()
{
    ui->tblPalette->clear();
    for (int iy=0; iy<PALETTE_H; iy++)
    {
        for (int ix=0; ix<PALETTE_W; ix++)
        {
            QTableWidgetItem* item= new QTableWidgetItem();
            QBrush bru_bg;
            if (ix+iy*PALETTE_W < project.tileset.palette.count())
                bru_bg.setColor(project.tileset.palette[ix+iy*PALETTE_W]);
            else
            {
                int lumarand= rand()%32;
                bru_bg.setColor(QColor::fromRgb(lumarand*8, lumarand*8, lumarand*8));
            }
            if (project.tileset.format == Tileset::GBA_4bpp)
            {
                if (project.paltable_current_row == iy && project.paltable_current_column == ix)
                    bru_bg.setStyle(Qt::Dense3Pattern);
                else if (project.paltable_current_row == iy)
                    bru_bg.setStyle(Qt::Dense1Pattern);
                else
                    bru_bg.setStyle(Qt::SolidPattern);
            }
            else
            {
                if (project.paltable_current_row == iy && project.paltable_current_column == ix)
                    bru_bg.setStyle(Qt::Dense3Pattern);
                else
                    bru_bg.setStyle(Qt::SolidPattern);
            }
            item->setBackground(bru_bg);
            if (project.tileset.format == Tileset::GBA_4bpp)
                item->setToolTip("Pal #"+QString::number(iy)+": "+QString::number(ix));
            else
                item->setToolTip("Index: "+QString::number(ix+iy*PALETTE_W));
            ui->tblPalette->setItem(iy, ix, item);
        }
    }

    switch (project.tileset.format)
    {
    case Tileset::GBA_4bpp:
        setWindowTitle("Palettes (4bpp)");
        break;
    case Tileset::GBA_8bpp:
        setWindowTitle("Palette (8bpp)");
        break;
    default:
        setWindowTitle("Palette (invalid tile format!)");
        break;
    }
}

void PaletteEdit::on_tblPalette_cellClicked(int row, int column)
{
    project.paltable_current_column= column;
    project.paltable_current_row= row;
    Update();
    int palind= column+row*PALETTE_W;
    if (palind >= project.tileset.palette.count())
        return;
    block_pal_updates= true;
    ui->sliRedChannel->setValue(qRed(project.tileset.palette[palind])/8);
    ui->sliGreenChannel->setValue(qGreen(project.tileset.palette[palind])/8);
    ui->sliBlueChannel->setValue(qBlue(project.tileset.palette[palind])/8);
    UpdateColorStatus(true);
    block_pal_updates= false;
    main_window->on_actionTilePicker_selected_pal_triggered();
}

void PaletteEdit::UpdateColorStatus(bool force)
{
    if (ui->sliRedChannel->hasFocus() || force)
        ui->spbRedChannel->setValue(ui->sliRedChannel->value());
    if (ui->sliGreenChannel->hasFocus() || force)
        ui->spbGreenChannel->setValue(ui->sliGreenChannel->value());
    if (ui->sliBlueChannel->hasFocus() || force)
        ui->spbBlueChannel->setValue(ui->sliBlueChannel->value());

    if (ui->spbRedChannel->hasFocus() && !force)
        ui->sliRedChannel->setValue(ui->spbRedChannel->value());
    if (ui->spbGreenChannel->hasFocus() && !force)
        ui->sliGreenChannel->setValue(ui->spbGreenChannel->value());
    if (ui->spbBlueChannel->hasFocus() && !force)
        ui->sliBlueChannel->setValue(ui->spbBlueChannel->value());

    ui->widPrimaryColor->setStyleSheet("border: 1px solid black; background-color: rgb("
                                       + QString::number(ui->spbRedChannel->value()*8) + ","
                                       + QString::number(ui->spbGreenChannel->value()*8) + ","
                                       + QString::number(ui->spbBlueChannel->value()*8) + ");");

    int paltable_index= project.paltable_current_column+project.paltable_current_row*PALETTE_W;
    QRgb new_col= QColor(ui->spbRedChannel->value()*8,
                          ui->spbGreenChannel->value()*8, ui->spbBlueChannel->value()*8).rgb();

    if (paltable_index < project.tileset.palette.count() && !force)
    {
        project.tileset.palette[paltable_index]= new_col;
        if (block_pal_updates) return;
        Update();
    }
}

void PaletteEdit::enterEvent(QEnterEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->showMessage("Click a color cell in the table to select it. In 4bpp tile mode, the current row will be used as the draw palette");
}

void PaletteEdit::leaveEvent(QEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->clearMessage();
}
