# message

**message** is a single header implementation C preprocessor macro to define C++
message classes binary and JSON serializable over a std::iostream.

JSON data is only serialized, binary data are deserialized as well.

# A simple example

Its usage is simple as declaring and using a C/C++ structure:

`#include "message.hpp"

DECLARE_MESSAGE
(
    MyStruct
    ,
    "MY_STRUCT"
    ,
    (uint32_t, anInteger, 42   )
    (uint8_t , aChar    , 'A'  )
    (float   , aFloat   , 3.14f)
    ,
    (bool    , optional , true )
)

MyStruct m_MyStruct;

void handle_message(const Message &m)
{
    if (m == m_MyStruct) // check if incoming message is MyStruct
    {
        printf("Known message\n");
    }

    if (m_MyStruct << m) // deserialize incoming message, only if it's MyStruct
    {
        printf("Payload size %ld: %s\n\n",
               my_MyStruct.size(),
               my_MyStruct.json().c_str());
    }
}

int main(int argc, char *argv[])
{
    std::stringstream stream;

    MyStruct m_out;  // instantiate a message

    m_out >> stream; // serialize a message

    m_out.anInteger = 17;
    m_out.aChar     = 'B';
    m_out.aFloat    = 1.2;
    m_out.optional  = false;

    m_out >> stream; // serialize another message

    Message m_in;

    while (m_in << stream)  // read all the messages from the stream
    {
        handle_message(m_in);
    }
}
`
