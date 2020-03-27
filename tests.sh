#!/bin/bash -x

on_file() {
    ./test_message_on_file -w out.dat
    ./test_message_on_file -r out.dat
}

on_iostream() {
    ./test_message_on_iostream
}

on_separate_stringstream() {
    ./test_message_on_separate_stringstream
}

from_cat() {
    ./test_message_on_file -w out.dat
    cat out.dat | ./test_message_on_file -r -
}

from_dd() {
    ./test_message_on_file -w out.dat
    dd if=out.dat | ./test_message_on_file -r -
}

from_pipe() {
    ./test_message_on_file -w out.dat
    ./test_message_on_file -r - < ./out.dat
}

from_text() {
    ./test_message_on_file -w /dev/null
    ./test_message_on_file -w /dev/null -d text | ./test_message_on_file -i -
}

from_json() {
    ./test_message_on_file -w /dev/null
    ./test_message_on_file -w /dev/null -d json | ./test_message_on_file -i -
}

from_text_file() {
    ./test_message_on_file -w /dev/null
    ./test_message_on_file -w /dev/null -d text > dump.text
    ./test_message_on_file -i dump.text
}

from_json_file() {
    ./test_message_on_file -w /dev/null -d json | tee dump.json
    ./test_message_on_file -i dump.json
}

on_file                  > message_on_file.txt
on_iostream              > message_on_iostream.txt
on_separate_stringstream > message_on_separate_stringstream.txt
from_cat                 > message_from_cat.txt
from_dd                  > message_from_dd.txt
from_pipe                > message_from_pipe.txt
from_text                > message_from_text.txt
from_json                > message_from_json.txt
from_text_file           > message_from_text_file.txt
from_json_file           > message_from_json_file.txt

md5sum message_*
