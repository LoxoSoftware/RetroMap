#include "toolboxpanel.h"
#include "macro.h"
#include "ui_toolboxpanel.h"
#include "project.h"

extern Project project;

ToolBoxPanel::ToolBoxPanel(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::ToolBoxPanel)
{
    ui->setupUi(this);
    UpdateTheme();

    connect(ui->btnSize1, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
    connect(ui->btnSize2, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
    connect(ui->btnSize3, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
    connect(ui->tlbPen, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
    connect(ui->tlbPalettePen, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
    connect(ui->tlbVFlipPen, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
    connect(ui->tlbHFlipPen, &QPushButton::clicked, this, &ToolBoxPanel::UpdateToolStatus);
}

ToolBoxPanel::~ToolBoxPanel()
{
    delete ui;
}

void ToolBoxPanel::UpdateToolStatus()
{
    int new_status= 0;

    if (ui->tlbPen->isChecked())
        new_status += tool_OffsetPen;
    if (ui->tlbVFlipPen->isChecked())
        new_status += tool_VFlipPen;
    if (ui->tlbHFlipPen->isChecked())
        new_status += tool_HFlipPen;
    if (ui->tlbPalettePen->isChecked())
        new_status += tool_PalettePen;

    if (ui->btnSize1->hasFocus())
    {
        ui->btnSize1->setChecked(true);
        ui->btnSize2->setChecked(false);
        ui->btnSize3->setChecked(false);
        project.pen_size= 1;
    } else
    if (ui->btnSize2->hasFocus())
    {
        ui->btnSize1->setChecked(false);
        ui->btnSize2->setChecked(true);
        ui->btnSize3->setChecked(false);
        project.pen_size= 3;
    } else
    if (ui->btnSize3->hasFocus())
    {
        ui->btnSize1->setChecked(false);
        ui->btnSize2->setChecked(false);
        ui->btnSize3->setChecked(true);
        project.pen_size= 5;
    }

    // if (new_status == tool_NoTool)
    // {
    //     ui->tlbPen->setChecked(true);
    //     ui->tlbPalettePen->setChecked(true);
    //     return UpdateToolStatus();
    // }

    project.selected_tools= new_status;
}

void ToolBoxPanel::UpdateTheme()
{
    if (IS_DARK_THEME)
    {
        ui->btnSize1->setIcon(QIcon(":/tool_icons/dark/toolsize_1"));
        ui->btnSize2->setIcon(QIcon(":/tool_icons/dark/toolsize_2"));
        ui->btnSize3->setIcon(QIcon(":/tool_icons/dark/toolsize_3"));
    } else {
        ui->btnSize1->setIcon(QIcon(":/tool_icons/toolsize_1"));
        ui->btnSize2->setIcon(QIcon(":/tool_icons/toolsize_2"));
        ui->btnSize3->setIcon(QIcon(":/tool_icons/toolsize_3"));
    }
}

void ToolBoxPanel::changeEvent(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::PaletteChange:
        UpdateTheme();
        break;
    default:
        return;
    }
}

#if QT_VERSION_MAJOR > 5
void ToolBoxPanel::enterEvent(QEnterEvent* event)
#else
void ToolBoxPanel::enterEvent(QEvent* event)
#endif
{
    if (!project.statusbar)
        return;
    project.statusbar->showMessage("Hover over a pen mode toggle button for a brief guide on what it does");
}

void ToolBoxPanel::leaveEvent(QEvent* event)
{
    if (!project.statusbar)
        return;
    project.statusbar->clearMessage();
}
