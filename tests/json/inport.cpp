#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

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
                bool   dump = false;
                string value = "";
                bool   has_value = true;
                size_t count_pos = 0;

                if (("int8_t"  == token) ||
                    ("int16_t" == token) ||
                    ("int32_t" == token) ||
                    ("int64_t" == token))
                {
                    dump = true;
                }
                else
                if (("uint8_t"  == token) ||
                    ("uint16_t" == token) ||
                    ("uint32_t" == token) ||
                    ("uint64_t" == token))
                {
                    dump = true;
                }
                else
                if ("bool" == token)
                {
                    dump = true;
                }
                else
                if ("float" == token)
                {
                    dump = true;
                }
                else
                if ("double" == token)
                {
                    dump = true;
                }
                else
                if ("string" == token)
                {
                    dump = true;
                }
                else
                if (token.find("vector(") == 0)
                {
                    dump      = true;
                    has_value = false;
                    count_pos = strlen("vector(");
                }
                else
                if (token.find("map(") == 0)
                {
                    dump      = true;
                    has_value = false;
                    count_pos = strlen("map(");
                }
                else
                if (token.find("bitset(") == 0)
                {
                    dump      = true;
                    count_pos = strlen("bitset(");
                }
                else
                if (token.find("optional(") == 0)
                {
                    dump      = true;
                    has_value = false;
                    count_pos = strlen("optional(");
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

                    cout << token;

                    if (count_pos > 0)
                    {
                        string count = token.substr(count_pos,
                                                    token.size()-count_pos-1);

                        uint64_t v_count = stoull(count);

                        cout << "[" << v_count << "]";
                    }

                    if (has_value)
                    {
                        if (("int8_t"  == token) ||
                            ("int16_t" == token) ||
                            ("int32_t" == token) ||
                            ("int64_t" == token))
                        {
                            int64_t v_value = stoll(value);

                            cout << " = " << v_value;
                        }
                        else
                        if (("uint8_t"  == token) ||
                            ("uint16_t" == token) ||
                            ("uint32_t" == token) ||
                            ("uint64_t" == token))
                        {
                            uint64_t v_value = stoull(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("bool" == token)
                        {
                            bool v_value = ("true" == value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("float" == token)
                        {
                            float v_value = stof(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("double" == token)
                        {
                            double v_value = stod(value);

                            cout << " = " << v_value;
                        }
                        else
                        if ("string" == token)
                        {
                            cout << " = " << value;
                        }
                        else
                        if (token.find("bitset(") == 0)
                        {
                            vector<bool> v_value;

                            for (size_t i=0; i<value.size(); i++)
                            {
                                v_value.push_back(value[i] == '1');
                            }

                            cout << " = " << value;
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