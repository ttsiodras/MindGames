#!/bin/bash
if [ -e /usr/bin/python2 ] ; then
    PYTHON=/usr/bin/python2
else
    if [ -e /usr/bin/python ] ; then
        PYTHON=/usr/bin/python
    else
        echo No Python interpreter found...
        exit 1
    fi
fi
bash -c '
for i in {1..100} ; do
    time bash -c "yes | ./bin.release/swapBishops > /dev/null"
done
' 2>&1 | grep ^real | sed 's,^real.*m,,;s,s$,,;' | \
    python2 ./contrib/stats.py
