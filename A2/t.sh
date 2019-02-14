#!/bin/bash
buffersize=(1 256 512 1024 2048 2096)
## Start testing
for value in ${buffersize[*]}
do
## Testing Unix I/O system calls
tcommando="./unixio filename $value 1"
eval $tcommando
## Testing C calls
tcommando="./unixio filename $value 0"
eval $tcommando
done
## Testing is done! :-)
