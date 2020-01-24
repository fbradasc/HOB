# Message

**Message** is a single header implementation C preprocessor macro to define C++
message classes binary and JSON serializable over a std::iostream.

JSON data is only serialized, binary data are deserialized as well.

# The DECLARE_MESSAGE macro

A message needs to be declared by using the DECLARE_MESSAGE() macro.

A very basic message requires a name and an identifier value:

    DECLARE_MESSAGE(
        class_name,
        identifier
    )

*class_name*: it's the message C++ class name

*identifier*: it's used to assign the message unique ID base value
          the *identifier* can be:
          - an unit64_t number (which is used as is for the ID base value)
          - a string (whose 64bit hash code is used for the ID base value)

A message can have core parameters:

    DECLARE_MESSAGE (
        class_name,
        identifier,
        parameters
    )

*parameters*: a list of core parameter definitions

    (filed_1_type, field_1_name [, field_1_default_value])
    (filed_2_type, field_2_name [, field_2_default_value])
    ...                   
    (filed_n_type, field_n_name [, field_n_default_value])

*field_#_type*         : [mandatory] it's the field's C++ variable type

*field_#_name*         : [mandatory] it's the field's C++ varible name

*field_#_default_value*: [optional] it's the field's C++ variable default value

The core parameters are used to calculate the message unique ID:

A message can also have extra parameters:

    DECLARE_MESSAGE (
        class_name,
        identifier,
        core_parameters,
        extra_parameters
    )


# A simple example

Its usage is simple as declaring and using a C/C++ structure:

```
#include "message.hpp"

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
```
