



```bash
cmake -S . -B build -DUSE_CLANG_TIDY=ON
```

```bash
cmake --build build --target clang-tidy-check
```