#include <fstream>
#include "message.hpp"

int main(int argc, char *argv[])
{
    ifstream txt;
    ofstream raw;

    txt.open(argv[1]);
    raw.open(argv[2], std::ios::binary);

    txt >> raw; // same of Message::parse(txt,raw);

    txt.close();
    raw.close();

    return 0;
}