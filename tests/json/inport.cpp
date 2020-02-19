#include <fstream>
#include <iostream>

using namespace std;

void parse(istream &is, char t)
{
    char c;
    bool group = ('}' == t);
    bool array = (']' == t);
    bool kword = ('"' == t);
    bool skip  = false;

    string token;

    while (is.get(c).good() && !skip && (t != c))
    {
        if (kword)
        {
            if (!skip && ('\\' == c))
            {
                skip = true;
            }
            else
            {
                skip = false;
                token += c;
            }
        }
        else
        if (!isspace(c))
        {
            skip = false;

            switch (c)
            {
                case '{' : parse(is, '}'); break;
                case '[' : parse(is, ']'); break;
                case '"' : parse(is, '"'); break;
                default  :                 break;
            }
        }
    }

    if (!token.empty())
    {
        cout << token << endl;
    }
}

int main(int argc, char *argv[])
{
    ifstream ifile;

    ifile.open(argv[1]);

    parse(ifile, '\0');

    ifile.close();

    return 0;
}