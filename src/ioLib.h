#pragma once

#include <stdlib.h>

constexpr size_t BUFFER_SIZE          = 512;
constexpr int    FILE_END             = -1;
constexpr int    UPDATE_BUFFER_DENIED = -1;

struct File;

File*    openFile              (const char* fileName, const char mode);
void     closeFile             (File* file);
void     setStringTermination  (char terminationSymbol);
char     getStringTermination  ();
size_t   readBufferFromFile    (File* file, size_t typeSize, size_t count, void* buffer);
size_t   writeBufferToFile     (File* file, size_t typeSize, size_t count, void* buffer);
void*    memoryCopy            (void* destination, const void* source, size_t bytesCount);
int      nextChar              (File* file);
char*    nextLine              (File* file, char* line, size_t maxLength);
int      writeChar             (File* file, char ch);
int      writeString           (File* file, const char* str);
int      writeLine             (File* file, const char* line);
int      writeFormatted        (File* file, const char* str, ...);
int      consoleNextChar       ();
char*    consoleNextLine       (char* line, size_t maxLength);
int      consoleWriteChar      (char ch);
int      consoleWriteString    (const char* str);
int      consoleWriteLine      (const char* line);
int      consoleWriteFormatted (const char* str, ...);
void     consoleMoveToNextLine ();
         
size_t   numberOfDigits        (int value);
char*    intToStr              (int value, char* str, size_t digits);
char*    intToStr              (int value, char* str);
size_t   strLength             (const char* str);
int      strCompare            (const unsigned char* str1, const unsigned char* str2);
char*    strConcatenate        (char* destination, const char* source);
char*    strFind               (const char* str, const char* substr);
char*    strFind               (const char* str, const char* substr, size_t maxSymbolsToCheck);
size_t   strNumOfOccurrences   (const char* str, char symbol);
size_t   strNumOfOccurrences   (const char* str, char symbol, size_t maxSymbolsToCheck);

int      isPunctuationMark     (unsigned char ch);
int      isLatinLetter         (unsigned char ch);
int      isCyrillicLetter      (unsigned char ch);

unsigned 
char     toLowerCase           (unsigned char ch);
