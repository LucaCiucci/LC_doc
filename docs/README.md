# `docs/`

This directory contains the generated documentation for the project.

To generate the documentation, run the following command:

```bash
# if you have lcdoc installed globally
lcdoc .

# if you do not have lcdoc installed globally,
# you can build this project and run the generated binary
mkdir build
cd build
cmake ..
cmake --build .
./build/lcdoc/Debug/lcdoc .
```

> **Note:** This directory is not included in the published package, the static website is generated and published by the CI/CD pipeline.
