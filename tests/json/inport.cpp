#include <fstream>
#include "message.hpp"

int main(int argc, char *argv[])
{
    ifstream txt;
    ofstream raw;

    txt.open(argv[1]);

    Message::Inporter txt2raw(txt);

    raw.open(argv[2], std::ios::binary);

    char c;

    while (txt2raw.get(c).good())
    {
        raw << c;
    }

    txt.close();
    raw.close();

    return 0;
}