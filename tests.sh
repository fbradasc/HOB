#!/bin/bash

BINENC=${1}

run() {
    ${*} # || >&2 strace ${*}
}

on_file() {
    run ./test_hob -wout${BINENC}.dat ${BINENC}
    run ./test_hob -rout${BINENC}.dat ${BINENC}
}

on_same_iobuffer() {
    run ./test_hob ${BINENC}
}

on_separate_buffers() {
    run ./test_hob -w -r ${BINENC}
}

from_cat() {
    run ./test_hob -wout${BINENC}.dat ${BINENC}
    cat out${BINENC}.dat | run ./test_hob -r ${BINENC}
}

from_dd() {
    run ./test_hob -wout${BINENC}.dat ${BINENC}
    dd if=out${BINENC}.dat | run ./test_hob -r ${BINENC}
}

from_pipe() {
    run ./test_hob -wout${BINENC}.dat ${BINENC}
    run ./test_hob -r ${BINENC} < ./out${BINENC}.dat
}

from_text() {
    run ./test_hob -wdump${BINENC}.text -ftext ${BINENC}
    cat dump${BINENC}.text | run ./test_hob -r -ftext ${BINENC}
}

from_json() {
    run ./test_hob -wdump${BINENC}.json -fjson ${BINENC}
    cat dump${BINENC}.json | run ./test_hob -r -fjson ${BINENC}
}

from_text_file() {
    run ./test_hob -wdump${BINENC}.text -ftext ${BINENC}
    run ./test_hob -rdump${BINENC}.text -ftext ${BINENC}
}

from_json_file() {
    run ./test_hob -wdump${BINENC}.json -fjson ${BINENC}
    run ./test_hob -rdump${BINENC}.json -ftext ${BINENC}
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

echo

md5sum hobs_*.txt
