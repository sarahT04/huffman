#include "huffman.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HuffmanApp w;
    w.show();
    return a.exec();
}
