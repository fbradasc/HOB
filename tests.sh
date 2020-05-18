#!/bin/bash -x

BINENC=${1}

on_file() {
    ./test_hob -wout.dat ${BINENC}
    ./test_hob -rout.dat ${BINENC}
}

on_same_iobuffer() {
    ./test_hob ${BINENC}
}

on_separate_buffers() {
    ./test_hob -w -r ${BINENC}
}

from_cat() {
    ./test_hob -wout.dat ${BINENC}
    cat out.dat | ./test_hob -r ${BINENC}
}

from_dd() {
    ./test_hob -wout.dat ${BINENC}
    dd if=out.dat | ./test_hob -r ${BINENC}
}

from_pipe() {
    ./test_hob -wout.dat ${BINENC}
    ./test_hob -r ${BINENC} < ./out.dat
}

from_text() {
    ./test_hob -wdump.text -ftext ${BINENC}
    cat dump.text | ./test_hob -r -ftext ${BINENC}
}

from_json() {
    ./test_hob -wdump.json -fjson ${BINENC}
    cat dump.json | ./test_hob -r -fjson ${BINENC}
}

from_text_file() {
    ./test_hob -wdump.text -ftext ${BINENC}
    ./test_hob -rdump.text -ftext ${BINENC}
}

from_json_file() {
    ./test_hob -wdump.json -fjson ${BINENC}
    ./test_hob -rdump.json -ftext ${BINENC}
}

on_file             > hobs_on_file.txt
on_same_iobuffer    > hobs_on_same_iobuffer.txt
on_separate_buffers > hobs_on_separate_buffers.txt
from_cat            > hobs_from_cat.txt
from_dd             > hobs_from_dd.txt
from_pipe           > hobs_from_pipe.txt
from_text           > hobs_from_text.txt
from_json           > hobs_from_json.txt
from_text_file      > hobs_from_text_file.txt
from_json_file      > hobs_from_json_file.txt

md5sum hobs_*.txt
