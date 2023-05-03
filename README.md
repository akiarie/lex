# A Naive Implementation of Lex

The implementation was built for educational purposes, so it doesn't provide
support for literal tokens.

After running `make`, you should be able to test the example file, which is
based on the Pig Latin question in the Dragon Book:

```bash
./lex examples/lex.l
cc lex.yy.c
./a.out 'the quick brown fox jumps over the lazy dog'
he tay uick qay rown bay ox fay umps jay overay he tay azy lay ogday
```
