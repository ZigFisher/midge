#!/bin/sh

local red='root\|err\|DROP\|REJECT\|fail'
local yellow='warn'
sed -e 's-\(/[ubsd][^ ]*\)-<b>\1</b>-g' -e 's-\('$red'\)-<font color=red>\1</font>-g' -e 's-\('$yellow'\)-<font color=yellow>\1</font>-g'
