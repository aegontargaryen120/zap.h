# Zap!

It's like [nob](https://github.com/tsoding/nob.h), but it's simpler.
I also changed the fmt a bit.
That's it

## Quick Start
### Own Project

You copy-paste the [zap.h](./zap.h) file into your repo, define `ZAP_IMPLEMENTATION`, and use it!

### From here

```console
$ cc -o zap zap.c
$ ./zap
INFO: Creating directory `bin`...
CMD: clang -Wall -Wextra -std=c99 -pedantic -ggdb -o bin/main main.c
CMD: ./bin/main
Hello, world!
CMD: rm -rf ./bin/main ./bin/main.dSYM
```
