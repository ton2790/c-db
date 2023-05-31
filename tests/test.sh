#!/bin/bash

WD=/home/nak/dev/C/database/tests/

cat "${WD}test${1}" | ./db | diff - "${WD}expected${1}"
