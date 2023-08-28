#include <QApplication>
#include "ComAutoTest.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    ComAutoTest w;
    w.show();
    return QApplication::exec();
}
