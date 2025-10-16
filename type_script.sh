#!/bin/bash
set -e
echo "Compiling..."
make
echo
echo "=== Test bounded_buffer (correct run) ==="
./bounded_buffer
echo
echo "=== Test mh (correct input) ==="
./mh 2
echo
echo "=== Test mh (incorrect input) ==="
if ./mh 0 ; then
  echo "ERROR: should have failed"
else
  echo "Correctly failed for bad input"
fi
echo
echo "=== Test airline (small run) ==="
./airline 8 2 2 2
echo
echo "=== Test airline (incorrect input) ==="
if ./airline 0 0 0 0 ; then
  echo "ERROR: should have failed"
else
  echo "Correctly failed for bad input"
fi
echo
echo "All tests completed."