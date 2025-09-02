#!/usr/bin/env bash
set -euo pipefail

# Build SimulAVR + pysimulavr in /workspace/simulavr/build
cd /workspace
if [ ! -d simulavr ]; then
  git clone https://git.savannah.nongnu.org/git/simulavr.git
fi
cd simulavr
# Avoid debian packaging subdir which can fail on aarch64
sed -i "s/^add_subdirectory(\s*debian\s*)/# add_subdirectory(debian)/" CMakeLists.txt || true
rm -rf build
# Prefer upstream Makefile targets to build Python module
make python -j"$(nproc)"
make build -j"$(nproc)"

# Probe the built module
python3 - <<'PY'
import sys, glob, os
cands = glob.glob('build/pysimulavr/*pysimulavr*.so') or glob.glob('build/*pysimulavr*.so')
assert cands, 'pysimulavr .so not found'
sys.path.insert(0, os.path.dirname(cands[0]))
import pysimulavr
print('pysimulavr OK at', cands[0])
PY

echo "Built pysimulavr under /workspace/simulavr/build/pysimulavr"

