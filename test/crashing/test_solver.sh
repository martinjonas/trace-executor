#!/bin/bash


# At first, everything is fine...
read LINE
echo "success"

# ... but then the solver crashes
read LINE
exit 1
