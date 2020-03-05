#include <fstream>
#include "message.hpp"

int main(int argc, char *argv[])
{
    ifstream txt;
    ofstream raw;

    txt.open(argv[1]);
    raw.open(argv[2], std::ios::binary);

    Message::Src src(txt);
    Message::Snk snk(raw);

    src >> snk; // same of Message::parse(src,snk);

    txt.close();
    raw.close();

    return 0;
}