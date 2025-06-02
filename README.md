# CLN (Compact Literal Notation)
# WORK IN PROGRESS, IT'S NOT USABLE YET

### What is CLN
CLN is a minimal notation for storing literals (strings and numbers) in a compact format designed to increase the speed of .

### Syntax
Everything is in one line, data is separated with commas (though it can be easily changed).
Both the sender and the receiver should know the layout beforehand, so if the layout was
*string - string - float - string - signed int32 - double* then the encoded version would look like this:
```string1,str2,7.3,hello\, "world"!,2112,472189894.9104704282```
