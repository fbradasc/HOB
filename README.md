# Message

**Message** is a single header implementation C preprocessor macro to define C++
message classes binary and JSON serializable over a std::iostream.

JSON data is only serialized, binary data are deserialized as well.

## The DECLARE_MESSAGE macro

A message needs to be declared by using the DECLARE_MESSAGE() macro.

### Identification

A very basic message requires a name and an identifier:

    DECLARE_MESSAGE (
        class_name,
        identifier
    )

*class_name*: it's the message C++ class name  
*identifier*: it's used to assign the message unique ID base value
          the *identifier* can be:
          - an unit64_t number (which is used as is for the ID base value)
          - a string (whose 64bit hash code is used for the ID base value)

### Core parameters

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

### Extra parameters

A message can also have extra parameters:

    DECLARE_MESSAGE (
        class_name,
        identifier,
        core_parameters,
        extra_parameters
    )

Extra parameters are defined in the same way as core parameters but they are
not involved in the message unique ID calculation.

### Parameter types

The message parameters are C++ variables by value (references and pointers are
not supported) and can be any of the following types:

#### C++ atomic types

    int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t,
    boot, float, double

#### C++ STL types

    string, bitset<N>

#### Another message

    Message

#### C++ STL container of any of above types

    vector<T>, map<K,V>

### Optional parameters

It can be possible to declare a parameter as optional, when its value can be
missing.  
To do this just declare the parameter by means of:

    optional<T>

Where T can be any of the above parameter's type.

## Messages definition and usage

After a message has been declared, by means of the DECLARE_MESSAGE() macro, a
variable can be defined with the message type:

```
    DECLARE_MESSAGE(MyMessageT, ...)

    MyMessageT myMessage;
```

### Serialization

Messages can be serialized over a C++ STL ostream or iostream derived object:

```
    std::stringstream stream;

    myMessage >> stream;
```

### Deserialization

Messages can be retrieved from a C++ STL istream derived object.  
However to make sure that the deserialized data is really what it should be,
some steps are needed:

1. Deserialization of the message identifier
2. Optionally match the deserialized identifier with the message type to be read
3. Deserialization of the message data, it is successful only if the identifier
   matches the message to be read

```
    std::ifstream stream("messages.dat", std::ios::binary);

    Message m;  // define a generic message object

    // 1. retrieve the message identifier from the input stream
    //
    if (m << stream)
    {
        // 2. [optional] ensure the message read is of MyMessageT type
        //
        if (m != myMessage)
        {
            printf("Unknown message type\n");
        }

        // 3. deserialize the message content
        //
        if (myMessage << m)
        {
            // do stuffs with the message
        }
    }
```

#### Messages as events

Messages can be used as events in an event oriented application.

```
    DECLARE_MESSAGE(AlmightyEvent, 42)

    AlmightyEvent almight_evt;
```

##### Sending events

To send an event just use message serialization:

```
    std::stringstream events_queue;

    evt >> events_queue;
```

##### Dispatching events

Dispatch events in the following manner:

```
    typedef void (*EventHandler)(const Message &);

    list<EventHandlers> event_handlers;

    Message evt;

    // 1. retrieve the event identifier from the events queue
    //
    while (evt << events_queue)
    {
        for (EventHandlers::Iterator event_handler  = event_handlers.begin();
                                     event_handler != event_handlers.end();
                                     event_handler++)
        {
            (*event_handler)(evt);
        }
    }
```

##### Handling events

```
    void handle_message(const Message &m)
    {
        // 2. ensure the event read is of AlmightyEvent type
        //
        if (m == almight_evt)
        {
            printf("Known event\n");
        }
        else
        {
            printf("Unknown event\n");
        }

        // 3. deserialize the event content
        //
        if (almight_evt << m)
        {
            // do stuffs with the event
        }
    }
```

## On the wire

### Binary Serialization

The Message content data types are encoded according to their data types.

#### Data type encoding

##### Unsigned Integer values

```
uint8_t, uint16_t, uint32_t, uint64_t
```

Unsigned integer values are encoded in the variable integer format.

Variable integer (VARINT) are packed in 1 to 9 bytes, big endian

The MSb bits in the MSB byte identify how many bytes are required to store the
numeric value.

```
x: single bit  
X: single byte (8 bits)

1 byte  [numeric value encoded in  7 bits]: 0xxxxxxx  
2 bytes [numeric value encoded in 14 bits]: 10xxxxxx.X  
3 bytes [numeric value encoded in 21 bits]: 110xxxxx.X.X  
4 bytes [numeric value encoded in 28 bits]: 1110xxxx.X.X.X  
5 bytes [numeric value encoded in 35 bits]: 11110xxx.X.X.X.X  
6 bytes [numeric value encoded in 42 bits]: 111110xx.X.X.X.X.X  
7 bytes [numeric value encoded in 49 bits]: 1111110x.X.X.X.X.X.X  
8 bytes [numeric value encoded in 56 bits]: 11111110.X.X.X.X.X.X.X  
9 bytes [numeric value encoded in 64 bits]: 11111111.X.X.X.X.X.X.X.X
```

##### Signed Integer values

```
int8_t, int16_t, int32_t, int64_t
```

Signed integer values are ZIGZAG-VARINT encoded by mapping
negative values to positive values while going back and forth:

```
(0=0, -1=1, 1=2, -2=3, 2=4, -3=5, 3=6, ...)
```

##### Boolean values

```
bool
```

*bool* values are packed in 1 byte

| bool value | byte value |
|    :---:   |    :---:   |
|    true    |      1     |
|    false   |      0     |

##### Floating point values

```
float, double
```

*float* values are packed in 4 bytes, little endian  
*double* values are packed in 8 bytes, little endian

##### String values

```
string
```

String values are encoded by packing the string size and the string data.

| string.size() | string.data() |
|    :---:      |     :---:     |
|    VARINT     |  char[size]   |

##### Bitset values

```
bitset<N>
```

Bitset values with *N* bits are encoded as a **vector<uint8_t>** having
**(N + 7) / 8** items.

##### Message values

```
Message
```

Messages are encoded by packing the message identifier optionally followed by 
the number of bytes required to serialize the message parameters, followed by
the message parameters serialization.

The identifier of a message without parameters is an even integer numeric value.

The identifier of a message with parameters is an odd integer numeric value.

| Identifier |  Size  |     Parameters     |
|   :---:    | :---:  |       :---:        |
|   VARINT   | VARINT | char[message size] |

The *Identifier* and *Size* fields are uint64_t variables, thus they are
encoded in a VARINT format.

The *Size* is the count of bytes used to serialize the *Parameters*.

The *Parameters* is the serialization of the *Core Parameters* followed by the
*Extra Parameters*, if any.

The *Size* and *Parameters* sections are present only when the *Identifier*
field is an odd integer numeric value
