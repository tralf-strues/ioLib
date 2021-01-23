This is my attempt to understand how some standard C i/o functions work. Basically I've written my own very basic versions of `printf`, `scanf` and some other functions.  

# Functionality
## File*
Basically all iolib functions work with `File`, instead of C `FILE` (though `File` actually has `FILE` in it 😄). Many input and output functions use a fixed-size (`BUFFER_SIZE`, which is set to 512 by default and when) buffer in order not to read from or write to a file each symbol individually.  

In order to open/close a file use `openFile`/`closeFile` - simple as that 🐨.  

## String termination
Ever wanted to make some symbol other than this dull `'\0'` be treated as string termination indicator? Well now you can! Just use `setStringTermination` function and all iolib functions will consider e.g. `'Q'` as string termination (<del> Quit - isn't it brilliant!?</del>🦉)!

## Some other elvish magic 🪄
In iolib you can also find several <del>useful</del> functions like `numberOfDigits`, `strConcatenate`, `strNumOfOccurrences`, `isCyrillicLetter` and others.

# Documentation
You can also look at [documentation](https://tralf-strues.github.io/iolib/files.html) for more details.
