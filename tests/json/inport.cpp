#include <fstream>
#include "hob.hpp"

int main(int argc, char *argv[])
{
    HOBIO::FileReader txt(argv[1]);
    HOBIO::FileWriter raw(argv[2]);

    txt >> raw; // same of HOB::parse(txt,raw);

    txt.close();
    raw.close();

    return 0;
}
