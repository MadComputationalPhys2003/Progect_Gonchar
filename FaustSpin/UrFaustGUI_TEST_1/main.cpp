#include "main_window_1.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow_1 w;
    w.show();
    return QApplication::exec();
}
