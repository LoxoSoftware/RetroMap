#include "mapsizeselector.h"
#include "ui_mapsizeselector.h"
#include "project.h"

extern Project project;

MapSizeSelector::MapSizeSelector(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MapSizeSelector)
{
    ui->setupUi(this);

    if (project.current_mapcanvas)
    {
        ui->spbWidth->setValue(project.current_mapcanvas->Size().width()*TILE_W);
        ui->spbHeight->setValue(project.current_mapcanvas->Size().height()*TILE_H);
    }
    else
        close();
}

MapSizeSelector::~MapSizeSelector()
{
    delete ui;
}

void MapSizeSelector::on_cmbPresetSize_currentIndexChanged(int index)
{
    if (index)
    {
        ui->spbWidth->setEnabled(false);
        ui->spbHeight->setEnabled(false);
    }
    else
    {
        ui->spbWidth->setEnabled(true);
        ui->spbHeight->setEnabled(true);
    }

    switch (index)
    {
    case 1:
        ui->spbWidth->setValue(128);
        ui->spbHeight->setValue(128);
        break;
    case 2:
        ui->spbWidth->setValue(256);
        ui->spbHeight->setValue(256);
        break;
    case 3:
        ui->spbWidth->setValue(512);
        ui->spbHeight->setValue(256);
        break;
    case 4:
        ui->spbWidth->setValue(256);
        ui->spbHeight->setValue(512);
        break;
    case 5:
        ui->spbWidth->setValue(512);
        ui->spbHeight->setValue(512);
        break;
    case 6:
        ui->spbWidth->setValue(1024);
        ui->spbHeight->setValue(1024);
        break;
    }
}

void MapSizeSelector::on_bnbDialogButtons_rejected()
{
    this->close();
}

void MapSizeSelector::on_bnbDialogButtons_accepted()
{
    if (project.current_mapcanvas)
    {
        project.current_mapcanvas->Resize(ui->spbWidth->value()/TILE_W, ui->spbHeight->value()/TILE_H);
    }
    else
        return; //TODO: add this case for when creating a new project

    this->close();
}
