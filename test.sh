#!/bin/bash

numbers=8
processes=20

#preklad cpp zdrojaku
mpicc  -o oems oems.c #--prefix /usr/local/share/OpenMPI

#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers

#spusteni
mpirun -oversubscribe -np ${processes} ./oems #--prefix /usr/local/share/OpenMPI

#uklid
rm -f oems numbers