#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdint.h>
#include <string>
#include <climits>

using namespace std;

void parse(istream &is, char t)
{
    char c;
    bool group = ('}' == t);
    bool array = (']' == t);
    bool kword = ('"' == t);
    bool skip  = false;

    string token;

    while (is.get(c).good() && ((t != c) || skip))
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
        while (is.get(c).good())
        {
            if (':' == c)
            {
                bool   dump      = false;
                string value     = "";
                bool   has_value = true;
                size_t count_pos = 0;

                if ("U" == token)
                {
                    dump = true;
                }
                else
                if ("S" == token)
                {
                    dump = true;
                }
                else
                if ("F" == token)
                {
                    dump = true;
                }
                else
                if ("D" == token)
                {
                    dump = true;
                }
                else
                if ("Q" == token)
                {
                    dump = true;
                }
                else
                if ("B" == token)
                {
                    dump = true;
                }
                else
                if ("T" == token)
                {
                    dump = true;
                }
                else
                if (token.find("V(") == 0)
                {
                    dump      = true;
                    has_value = false;
                    count_pos = strlen("V(");
                }
                else
                if (token.find("M(") == 0)
                {
                    dump      = true;
                    has_value = false;
                    count_pos = strlen("M(");
                }
                else
                if (token.find("O(") == 0)
                {
                    dump      = true;
                    has_value = false;
                    count_pos = strlen("O(");
                }

                if (dump)
                {
                    if (has_value)
                    {
                        bool can_be_spaced = false;

                        skip = false;

                        while (is.get(c).good())
                        {
                            if (can_be_spaced || !isspace(c))
                            {
                                if (!skip && ('"' == c))
                                {
                                    can_be_spaced = !can_be_spaced;
                                }
                                else
                                if (!skip && ('\\' == c))
                                {
                                    skip = true;
                                }
                                else
                                {
                                    skip = false;

                                    if ('}' == c)
                                    {
                                        is.unget();
                                        break;
                                    }
                                    else
                                    {
                                        value += c;
                                    }
                                }
                            }
                        }
                    }

                    if (count_pos > 0)
                    {
                        cout << token.substr(0,count_pos-1);

                        string count = token.substr(count_pos,
                                                    token.size()-count_pos-1);

                        uint64_t v_count = stoull(count);

                        cout << "[" << v_count << "]";
                    }
                    else
                    {
                        cout << token;
                    }

                    if (has_value)
                    {
                        if ("U" == token)
                        {
                            uint64_t v_value = stoull(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("S" == token)
                        {
                            int64_t v_value = stoll(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("F" == token)
                        {
                            float v_value = stof(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("D" == token)
                        {
                            double v_value = stod(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("Q" == token)
                        {
                            long double v_value = stold(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("T" == token)
                        {
                            cout << " = " << value;
                        }
                        else
                        if ("B" == token)
                        {
                            if (("true" == value) || ("false" == value))
                            {
                                bool v_value = ("true" == value);

                                cout << " = " << v_value;
                            }
                            else
                            {
                                vector<bool> v_value;

                                for (size_t i=0; i<value.size(); i++)
                                {
                                    if (('1' == value[i]) || ('0' == value[i]))
                                    {
                                        v_value.push_back(value[i] == '1');
                                    }
                                }

                                if (!value.empty())
                                {
                                    uint64_t v_count = v_value.size();

                                    cout << "[" << v_count << "]";

                                    cout << " = ";

                                    for (size_t j=0; j<v_value.size(); j++)
                                    {
                                        cout << v_value[j];
                                    }
                                }
                            }
                        }
                    }

                    cout << endl;
                }

                break;
            }
            else
            if (!isspace(c))
            {
                is.unget();

                break;
            }
        }
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