# Native controller lifecycle regression

These dependency-free tests compile the real Windows controller with a minimal
queue and performance-clock seam. They exercise controller ownership, restart,
repeated close, and frame metadata. They do not validate Windows shared memory,
OBS, pybind11, or actual video transport.

From the repository root, with a C++11 compiler:

```sh
c++ -std=c++11 -Wall -Wextra -Werror -Wno-narrowing \
  -Itests/native/stubs -Ipyvirtualcam/native_windows \
  tests/native/controller_lifecycle.cpp \
  pyvirtualcam/native_windows/controller/controller.cpp \
  -o /tmp/pyvirtualcam-controller-test
for case in stop-before-start send-stop duplicate-start double-stop restart repeat; do
  /tmp/pyvirtualcam-controller-test "$case" || exit 1
done
```

The `-Wno-narrowing` option accommodates the controller's existing signed-width
to unsigned-stride initializer. To detect invalid memory accesses, add
`-fsanitize=address,undefined -fno-omit-frame-pointer -g` to the compiler command
and rerun all six cases. The test replaces only queue/clock dependencies, not
the controller's allocations or frees. Run each case in a fresh process.
