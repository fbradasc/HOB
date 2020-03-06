# Header Only Buffer

**Header Only Buffer** (**HOB** for short) is a single C++ header implementation
preprocessor macro to define C++ structured buffers classes which can be binary
and text/JSON serializable over a std::iostream.

## The HOBSTRUCT macro

An **HOB** structure needs to be declared by using the HOBSTRUCT() macro.

### Identification

A very basic **HOB** requires a name and an identifier:

```
HOBSTRUCT (
    hob_name,
    identifier
)
```

*hob_name*: it's the **HOB** C++ class name  
*identifier*: it's used to assign the **HOB** *UID* base value  

The *identifier* can be:  

- a 63 bit integer number (it's used as is for the *UID* base value)
- a string (whose 63 bit hash code is used for the *UID* base value)

### Core parameters

An **HOB** can have core parameters:

```
HOBSTRUCT (
    hob_name,
    identifier,
    parameters
)
```

*parameters*: a list of core parameter definitions

```
(filed_1_type, field_1_name [, field_1_default_value])
(filed_2_type, field_2_name [, field_2_default_value])
...                   
(filed_n_type, field_n_name [, field_n_default_value])
```

*field_#_type*         : [mandatory] it's the field's C++ variable type  
*field_#_name*         : [mandatory] it's the field's C++ varible name  
*field_#_default_value*: [optional] it's the field's C++ variable default value

The core parameters are used to update the **HOB** *UID*.

### Extra parameters

An **HOB** can also have extra parameters:

```
HOBSTRUCT (
    hob_name,
    identifier,
    core_parameters,
    extra_parameters
)
```

Extra parameters are defined in the same way as core parameters.

The extra parameters are **not** used to update the **HOB** *UID*.

### Parameter types

The **HOB** parameters are C++ variables by value (references and pointers are
not supported) and can be any of the following types:

#### Base types

    int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, boot,
    float, double

#### Complex types

    string, bitset<N>

Where N is the number of bits to be stored in the bitset.

#### Another **HOB**

    HOB

Example:
```
HOBSTRUCT(Payload, 42)

HOBSTRUCT (
    Envelope,
    "Encapsulated",
    ( Payload, payload )
)
```

#### Collections

    vector<T>, map<K,V>

Where T, K and V can be any of the above parameter type.

#### Optional type

It can be possible to declare a parameter as optional, when its value can be
missing.  
To do this just declare the parameter by means of:

    optional<T>

Where T can be any of the above parameter's type.

## **HOB** definition and usage

After an **HOB** has been declared, by means of the HOBSTRUCT() macro, a
variable can be defined with the **HOB** type:

```
HOBSTRUCT(MyMessageT, ...)

MyMessageT myMessage;
```

### Serialization

**HOBs** can be serialized over a C++ STL ostream or iostream derived object:

```
std::stringstream stream;

myMessage >> stream;
```

### Deserialization

**HOBSs** can be retrieved from a C++ STL istream derived object.  
However to make sure that the deserialized data is really what it should be,
some steps are needed:

1. Deserialization of the **HOB** *UID*
    
| equivalent syntaxes           |
| :---                          |
| ```std::istream >> message``` |
| ```message << std::istream``` |

2. Optionally match the deserialized *UID* with the **HOB** type to be read
    
| syntaxes                    | relation                       |
| :---                        | :---                           |
| ```message == my_message``` | message **IS A** my_message    | 
| ```message != my_message``` | message **IS NOT A** my_message|

3. Deserialization of the **HOB** data, it is successful only if the *UID*
   matches the **HOB** to be read

| equivalent syntaxes          |
| :---                         |
| ```message >> my_message```  |
| ```my_message << message```  |

Example:
```
std::ifstream stream("messages.dat", std::ios::binary);

HOB m;  // define a generic HOB object

// 1. retrieve the HOB UID from the input stream
//
if (stream >> m)
{
    // 2. [optional] ensure the HOB read is of MyMessageT type
    //
    if (m != myMessage)
    {
        printf("Unknown HOB type\n");
    }

    // 3. deserialize the HOB content
    //
    if (m >> myMessage)
    {
        // do stuffs with the HOB
    }
}
```

NOTE:  
The following syntaxes are equivalent:
    
| 1. retrieve **HOB** *UID*     | 3. retrieve **HOB** data    |
| :---                          | :---                        |
| ```std::istream >> message``` | ```message >> my_message``` |
| ```message << std::istream``` | ```my_message << message``` |

#### Detecting changes due to deserialization

The declared **HOBSs** have methods to enquire/reset changes in their data:

    operator bool () const;

Can be used to detect the change status in any of the **HOB** fileds.

    void operator ~ ();

Can be used to reset the change status in all of the **HOB** fileds.

    bool operator & (const Fields &field) const;

Can be used to detect the change status in a given field.

    void operator -= (const Fields &field);

Can be used to reset the change status in a given field.

The **Fields** type is an *enum* containing the id of all the fields:

```
enum Fields
{
    _field_1_name,
    ...
    _field_n_name
};
```

##### Changes detection example

```
// declare the specific HOB class
//
HOBSTRUCT (
    ChemicalReactionM,
    42,
    (double, temp, 0.0d)
    (double, pres, 0.0d)
)

HOB m;  // define a generic HOB object

ChemicalReactionM chemicalReaction; // define the specific HOB object

// retrieve the HOB UID from the input stream
//
while (stream >> m)
{
    // deserialize the HOB content
    //
    // NOTE:
    // the change status of all the fields is being reset by the deserialization
    //
    if (m >> chemicalReaction)
    {
        if (chemicalReaction)
        {
            // something is changed in deserialized data

            if (chemicalReaction & ChemicalReactionM::_temp)
            {
                // the temp field is changed

                // you can reset the temp field change status
                //
                // Not really needed, see NOTE above
                //
                chemicalReaction -= ChemicalReactionM::_temp;
            }

            if (chemicalReaction & ChemicalReactionM::_pres)
            {
                // the pres field is changed
            }

            // you can reset the pres field change status
            //
            // Not really needed, see NOTE above
            //
            chemicalReaction -= ChemicalReactionM::_pres;

            // you can reset the change status of all the field
            //
            // Not really needed, see NOTE above
            //
            ~chemicalReaction;
        }
    }
}
```
#### HOBs as events

**HOBSs** can be used as events in an event oriented application.

```
HOBSTRUCT(AlmightyEvent, 42)

AlmightyEvent almight_evt;
```

##### Sending events

To send an event just use **HOB** serialization:

```
std::stringstream events_queue;

almight_evt >> events_queue;
```

##### Dispatching events

Dispatch events in the following manner:

```
typedef void (*EventHandler)(const HOB &);

list<EventHandlers> event_handlers;

HOB evt;

// 1. retrieve the event UID from the events queue
//
while (events_queue >> evt)
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
void handle_message(HOB &m)
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
    if (m >> almight_evt)
    {
        // do stuffs with the event
    }
}
```

## On the wire

### Binary Serialization

The **HOB** content data types are encoded according to their data types.

#### Data type encoding

##### Base types

###### Unsigned Integer types

```
uint8_t, uint16_t, uint32_t, uint64_t
```

Unsigned integer types are encoded in the variable integer format.

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

###### Signed Integer types

```
int8_t, int16_t, int32_t, int64_t
```

Signed integer types are ZIGZAG-VARINT encoded by mapping
negative values to positive values while going back and forth:

```
(0=0, -1=1, 1=2, -2=3, 2=4, -3=5, 3=6, ...)
```

###### Boolean types

```
bool
```

*bool* types are packed in 1 byte

| bool value | byte value |
|    :---:   |    :---:   |
|    true    |      1     |
|    false   |      0     |

###### Floating point types

```
float, double
```

*float* types are packed in 4 bytes, little endian  
*double* types are packed in 8 bytes, little endian

##### Complex types

###### String types

```
string
```

*string* types are encoded by packing the string size and the string data.

| string.size() | string.data() |
|    :---:      |     :---:     |
|    VARINT     | char[size()]  |

Empty string:

| 0 == string.size() |
|      :---:         |
|    VARINT(0)       |

###### Bitset types

```
bitset<N>
```

*bitset* types with *N* bits are encoded as a **vector<uint8_t>** having
**(N+7)/8** items.

###### **HOB** types

```
HOB
```

**HOB** are encoded by packing the **HOB** *UID* optionally followed by 
the number of bytes required to serialize the **HOB** parameters, followed by
the **HOB** parameters serialization.

The *UID* of an **HOB** without parameters is an even integer numeric value.

| UID (even value) |
|     :---:        |
|     VARINT       |

The *UID* of an **HOB** with parameters is an odd integer numeric value.

| UID (odd value) | Payload Size |      Payload       |
|     :---:       |    :---:     |       :---:        |
|     VARINT      |    VARINT    | char[Payload Size] |

The *UID* and *Payload Size* fields are **uint64_t** variables, thus they are
encoded in a VARINT format.

The *Payload Size* is the count of bytes used to serialize the *Payload*.

The *Payload* is the serialization of the *Core Parameters* followed by the
*Extra Parameters*, if any.

The *Payload Size* and *Payload* sections are present only when the *UID* is an
odd integer numeric value.

##### Collections

###### Vector types

```
vector<T>
```

*vector* types are encoded by packing the number of the items contained in
the vector, VARINT encoded, followed by the serialization of each of the
contained items:

| vector\<T>.size() | T[0]  |  ...  | T[size()-1] |
|      :---:        | :---: | :---: |    :---:    |
|      VARINT       |   T   |  ...  |      T      |

Empty vector:

| 0 == vector\<T>.size() |
|         :---:          |
|       VARINT(0)        |

A *vector* can store types of any of the above data types.

###### Map types

```
map<K,V>
```

*map* types are encoded by packing the number of the key and value pairs 
contained in the map, VARINT encoded, followed by the serialization of each of 
the contained pairs:

| map\<K,V>.size() | K[0]  | V[0]  |  ...  | K[size()-1] | V[size()-1] |
|      :---:       | :---: | :---: | :---: |    :---:    |    :---:    |
|      VARINT      |   K   |   V   |  ...  |      K      |      V      |

Empty map:

| 0 == map\<K,V>.size() |
|         :---:         |
|       VARINT(0)       |

A *map* can store keys and values of any of the above data types.

###### Optional types

```
optional<T>
```

*optional* types are encoded by packing a bool type value optionally followed by 
the serialization of the given type value.

When **no** value has been given to the *optional\<T>* data:

| false == (bool)optional\<T> |
|          :---:              |
|          false              |

When a value has been given to the *optional\<T>* data:

| true == (bool)optional\<T> | (T&)*optional\<T> |
|         :---:              |       :---:       |
|         true               |         T         |
