#include "widget.h"
#include "waveswidget.h"                                                                    // 包含 WavesWidget 类的头文件

#include <QApplication>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}