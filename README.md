# Build documentation

## Prerequisites

install sphinx, sphinx-rtd-theme and sphinx-design using pip

```
pip install sphinx sphinx-rtd-theme sphinx-design 
```

## Linux

simply run in the root directory

```bash
chmod +x main/scripts/build_docs.sh
main/scripts/build_docs.sh 
```

## Windows/Manual

```bash
sphinx-build -M html ./docs/source ./docs/build
```

The documentation will be available under /docs/build/html/index.html