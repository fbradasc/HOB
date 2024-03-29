#include <fstream>
#include "hob.hpp"

int main(int argc, char *argv[])
{
    ifstream txt;
    ofstream raw;

    txt.open(argv[1]);
    raw.open(argv[2], std::ios::binary);

    HOB::Src src(txt);
    HOB::Snk snk(raw);

    src >> snk; // same of HOB::parse(src,snk);

    txt.close();
    raw.close();

    return 0;
}