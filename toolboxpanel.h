#ifndef TOOLBOXPANEL_H
#define TOOLBOXPANEL_H

#include <QDockWidget>

namespace Ui {
class ToolBoxPanel;
}

class ToolBoxPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit ToolBoxPanel(QWidget *parent);
    ~ToolBoxPanel();

    void UpdateToolStatus();
    void UpdateTheme();

    static const int tool_NoTool = 0;
    static const int tool_OffsetPen = 1;
    static const int tool_VFlipPen = 2;
    static const int tool_HFlipPen = 4;
    static const int tool_PalettePen = 8;

private:
    Ui::ToolBoxPanel *ui;

    void enterEvent(QEnterEvent* event);
    void leaveEvent(QEvent* event);
    void changeEvent(QEvent* event);
};

#endif // TOOLBOXPANEL_H
