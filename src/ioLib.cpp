#include "ioLib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

char STRING_TERMINATION = '\0';

struct File
{
    unsigned char buffer[BUFFER_SIZE] = {NULL};
    FILE*         cfile               = NULL;
    unsigned int  position            = 0;
    unsigned int  correctBufferValues = 0;
    int           fileEndReached      = 0;
    char          mode                = 0;
};

int updateBuffer  (File* file);
int writeFormatted(File* file, const char* str, va_list valist);

//-----------------------------------------------------------------------------
//! Opens the file with name filename (by default in the same directory as the
//! executable file).
//!
//! @param [in] filename  name of the file to open
//! @param [in] mode      'r' for read-only;
//!                       'w' for writing (deletes previous contents of
//!                                        the file);
//!                       'a' for appending to the file
//!
//! @note If the file with this name doesn't exist then if mode is 'r' the
//!       function returns NULL and if mode is 'w' or 'a' a new file with
//!       this name will be created.
//!
//! @return a pointer to the File opened or NULL if an error occurred.
//-----------------------------------------------------------------------------
File* openFile(const char* fileName, const char mode)
{
    if (fileName == NULL)
        return NULL;

    if (mode != 'r' &&
        mode != 'w' &&
        mode != 'a')
        return NULL;

    char strMode[] = { mode , '\0' };
    FILE* cFILE = fopen((const char*) fileName, strMode);
    if (cFILE == NULL)
        return NULL;

    File* file = (File*)calloc(1, sizeof(File));
    if (file == NULL)
        return NULL;

    file->cfile = cFILE;
    file->position = BUFFER_SIZE;
    file->correctBufferValues = 0;
    file->mode = mode;

    return file;
}

//-----------------------------------------------------------------------------
//! Closes the file. Sets correctBufferValues of file to 0.
//!
//! @param [in] file  pointer to the file to be closed
//-----------------------------------------------------------------------------
void closeFile(File* file)
{
    if (file == NULL)
        return;

    file->correctBufferValues = 0;
    fclose(file->cfile);
    free(file);
}

//-----------------------------------------------------------------------------
//! Sets a symbol which will indicate the end of strings for all ioLib 
//! functions. By default it's '\0'.
//!
//! @param [in] terminationSymbol   
//-----------------------------------------------------------------------------
void setStringTermination (char terminationSymbol)
{
    STRING_TERMINATION = terminationSymbol;
}

//-----------------------------------------------------------------------------
//! Returns a symbol which will indicate the end of strings for all ioLib 
//! functions. By default it's '\0'.
//!
//! @return string termination symbol
//-----------------------------------------------------------------------------
char getStringTermination ()
{
    return STRING_TERMINATION;
}

//-----------------------------------------------------------------------------
//! Reads count objects of type that has size typeSize from file to buffer.  
//!
//! @param [in] file      pointer to the file from which to read
//! @param [in] typeSize  size of each object
//! @param [in] count     number of objects
//! @param [in] buffer    buffer to which to write
//!
//! @return number of objects read successfully or FILE_END on failure.
//-----------------------------------------------------------------------------
size_t readBufferFromFile(File* file, size_t typeSize, size_t count, void* buffer)
{
    if (file        == NULL ||
        file->cfile == NULL ||
        buffer      == NULL ||
        (file->mode != 'r'))
        return FILE_END;
    
    size_t result = fread(buffer, typeSize, count, file->cfile);
    return result != count ? FILE_END : result;
}

//-----------------------------------------------------------------------------
//! Writes count objects of type that has size typeSize from buffer to file.  
//!
//! @param [in] file      pointer to the file to which to write
//! @param [in] typeSize  size of each object
//! @param [in] count     number of objects
//! @param [in] buffer    buffer from which to read
//!
//! @return number of objects written successfully or FILE_END on failure.
//-----------------------------------------------------------------------------
size_t writeBufferToFile(File* file, size_t typeSize, size_t count, void* buffer)
{
    if (file        == NULL ||
        file->cfile == NULL ||
        buffer      == NULL ||
        (file->mode != 'w' && file->mode != 'a'))
        return FILE_END;

    size_t result = fwrite(buffer, typeSize, count, file->cfile);
    return result == EOF ? FILE_END : result;
}

//-----------------------------------------------------------------------------
//! Copies bytesCount bytes from source to destination.  
//!
//! @param [out] destination  pointer to the place to copy to
//! @param [in]  source       pointer to the place to copy from
//! @param [in]  bytesCount   number of bytes to copy from source to 
//!                           destination
//!
//! @return copy of destination.
//-----------------------------------------------------------------------------
void* memoryCopy(void* destination, const void* source, size_t bytesCount)
{
    assert(destination != NULL);
    assert(source      != NULL);

    for (size_t k = 0; k < bytesCount; k++)
        *((char*)destination + k) = *((char*)source + k);

    return destination;
}

//-----------------------------------------------------------------------------
//! Updates the contents of the buffer in file and sets position in file to 0.
//!
//! @param [in] file  pointer to the file to be closed
//!
//! @return 0 if the buffer was loaded correctly and UPDATE_BUFFER_DENIED 
//!         otherwise (usually when the end of the buffer hasn't been reached 
//!         yet or there's been an error).
//-----------------------------------------------------------------------------
int updateBuffer(File* file)
{
    if (file == NULL || file->mode != 'r')
        return UPDATE_BUFFER_DENIED;

    if (file->position < BUFFER_SIZE)
        return UPDATE_BUFFER_DENIED;

    file->correctBufferValues = fread(file->buffer,
                                      sizeof(char),
                                      BUFFER_SIZE,
                                      file->cfile);

    file->position = 0;

    return 0;
}

//-----------------------------------------------------------------------------
//! Reads the next char from file.
//!
//! @param [in] file  pointer to the file from which char is to be read
//!
//! @return next char or FILE_END if the end of file has been reached.
//-----------------------------------------------------------------------------
int nextChar(File* file)
{
    if (file == NULL || file->mode != 'r')
        return FILE_END;

    if (file->position < file->correctBufferValues)
    {
        file->position++;

        return file->buffer[file->position - 1];
    }

    if (file->position == BUFFER_SIZE)
    {
        updateBuffer(file);

        return nextChar(file);
    }

    file->fileEndReached = 1;

    return FILE_END;
}

//-----------------------------------------------------------------------------
//! Reads the next (maxLength - 1) characters from file to line. Adds '\0' at
//! the end of line.
//!
//! @param [in] file       pointer to the file from which line is to be read
//! @param [in] line       pointer to char* to which characters are to be read
//! @param [in] maxLength  max number of characters to read 
//!                        (typically sizeof (line))
//!
//! @return line or NULL on failure.
//-----------------------------------------------------------------------------
char* nextLine(File* file, char* line, size_t maxLength)
{
    if (line == NULL || file == NULL || maxLength == 0 || file->mode != 'r')
        return NULL;

    if (file->fileEndReached)
        return NULL;

    int currentChar = -1;
    for (int i = 0; i < maxLength - 1; i++)
    {
        currentChar = nextChar(file);
        if (currentChar == '\n' || currentChar == FILE_END)
        {
            line[i] = STRING_TERMINATION;
            return line;
        }

        line[i] = currentChar;
    }

    return NULL;
}

//-----------------------------------------------------------------------------
//! Writes ch to file. ch is converted to unsigned int.
//!
//! @param [in] file  pointer to the file to which ch is to be written
//! @param [in] ch    character to be written to file
//!
//! @return ch or FILE_END on failure.
//-----------------------------------------------------------------------------
int writeChar(File* file, char ch)
{
    if (file == NULL                          ||
        file->cfile == NULL                   ||
        (file->mode != 'w' && file->mode != 'a'))
        return FILE_END;

    int result = putc((unsigned char)ch, file->cfile);
    return result == (unsigned char)ch ? result : FILE_END;
}

//-----------------------------------------------------------------------------
//! Writes str to file.
//!
//! @param [in] file  pointer to the file to which str is to be written
//! @param [in] str   characters to be written to file
//!
//! @return 0 on success and FILE_END on failure.
//-----------------------------------------------------------------------------
int writeString(File* file, const char* str)
{
    if (file == NULL                             ||
        file->cfile == NULL                      ||
        (file->mode != 'w' && file->mode != 'a') ||
        str == NULL)
        return FILE_END;

    for (unsigned char* currentChar = (unsigned char*)str; *currentChar != STRING_TERMINATION; currentChar++)
        if (writeChar(file, *currentChar) == FILE_END)
            return FILE_END;

    return 0;
}

//-----------------------------------------------------------------------------
//! Writes line to file and adds '\n' after that.
//!
//! @param [in] file  pointer to the file to which line is to be written
//! @param [in] line  characters to be written to file
//!
//! @return 0 on success and FILE_END on failure.
//-----------------------------------------------------------------------------
int writeLine(File* file, const char* line)
{
    if (file == NULL                             ||
        file->cfile == NULL                      ||
        (file->mode != 'w' && file->mode != 'a') ||
        line == NULL)
        return FILE_END;

    for (unsigned char* currentChar = (unsigned char*)line; *currentChar != STRING_TERMINATION; currentChar++)
        if (writeChar(file, *currentChar) == FILE_END)
            return FILE_END;

    if (writeChar(file, '\n') == FILE_END)
        return FILE_END;

    return 0;
}

//-----------------------------------------------------------------------------
//! Writes formatted string to file. All %c, %d and %s from str are changed to
//! char, int or char* string equivalents of args from valist respectively. 
//!
//! @param [in] file    pointer to the file to which string is to be written
//! @param [in] str     pointer to a string containing format in which specifying
//!                     how to interpret the arguments from valist
//! @param [in] valist  
//!
//! @note The order of arguments from valist has to be the same as the order of
//!       %c, %d or %s indicators in str.
//!
//! @return number of arguments successfully interpreted or -1 if an
//!         error occurred.
//-----------------------------------------------------------------------------
int writeFormatted(File* file, const char* str, va_list valist)
{
    if (file        == NULL                      ||
        file->cfile == NULL                      ||
        (file->mode != 'w' && file->mode != 'a') ||
        str         == NULL)
        return -1;

    //=== for interpreting %d
    int    successfullyInterpreted = 0;
    int    intValue                = 0;
    size_t digits                  = 0;
    char*  convertedToStr          = NULL;
    //===
    for (char* currentChar = (char*)str; *currentChar != STRING_TERMINATION; currentChar++)
    {
        if (*currentChar == '%')
        {
            currentChar++;

            switch (*currentChar)
            {
                case 'c':
                if (writeChar(file, va_arg(valist, char)) == FILE_END)
                {
                    va_end(valist);
                    return -1;
                }

                successfullyInterpreted++;
                break;

                case 'd':
                intValue  = va_arg(valist, int);
                digits = numberOfDigits(intValue);
                digits = intValue < 0 ? digits + 1 : digits;

                convertedToStr = (char*) calloc(digits + 1, sizeof(char));
                if (intToStr(intValue, convertedToStr, intValue < 0 ? digits - 1 : digits) == NULL ||
                    writeString(file, convertedToStr) == FILE_END)
                {
                    free(convertedToStr);
                    va_end(valist);
                    return -1;
                }

                successfullyInterpreted++;
                free(convertedToStr);
                break;

                case 's':
                if (writeString(file, va_arg(valist, char*)) == FILE_END)
                {
                    va_end(valist);
                    return -1;
                }

                successfullyInterpreted++;
                break;

                default:
                if (writeChar(file, '%')          == FILE_END ||
                    writeChar(file, *currentChar) == FILE_END)
                {
                    va_end(valist);
                    return -1;
                }
                break;
            }
        }
        else if (writeChar(file, *currentChar) == FILE_END)
        {
            va_end(valist);
            return -1;
        }
    }

    va_end(valist);

    return successfullyInterpreted;    
}

//-----------------------------------------------------------------------------
//! Writes formatted string to file. All %c, %d and %s from str are changed to
//! char, int or char* string equivalents of args from ... respectively. 
//!
//! @param [in] file  pointer to the file to which string is to be written
//! @param [in] str   pointer to a string containing format in which specifying
//!                   how to interpret the arguments from ... 
//! @param [in] ...   arguments
//!
//! @note The order of arguments from ... has to be the same as the order of
//!       %c, %d or %s indicators in str.
//!
//! @return number of arguments successfully interpreted or -1 if an
//!         error occurred.
//-----------------------------------------------------------------------------
int writeFormatted(File* file, const char* str, ...)
{
    va_list valist;
    va_start(valist, str);

    return writeFormatted(file, str, valist);
}

//-----------------------------------------------------------------------------
//! Reads the next char from stdin.
//!
//! @return next char or FILE_END if and error occurred.
//-----------------------------------------------------------------------------
int consoleNextChar()
{
    int ch = getchar();
    return ch == EOF ? FILE_END : ch;
}

//-----------------------------------------------------------------------------
//! Reads the next (maxLength - 1) characters from stdin to line. Adds '\0' at
//! the end of line.
//!
//! @param [in] line       pointer to char* to which characters are to be read
//! @param [in] maxLength  max number of characters to read 
//!                        (typically sizeof (line))
//!
//! @return line or NULL on failure.
//-----------------------------------------------------------------------------
char* consoleNextLine(char* line, size_t maxLength)
{
    if (line == NULL)
        return NULL;

    if (scanf("%s", line) != 1)
        return NULL;

    return line;
}

//-----------------------------------------------------------------------------
//! Writes ch to stdout. ch is converted to unsigned int.
//!
//! @param [in] ch  character to be written to stdout
//!
//! @return ch or FILE_END on failure.
//-----------------------------------------------------------------------------
int consoleWriteChar(char ch)
{
    File file;
    file.cfile = stdout;
    file.mode  = 'w';

    return writeChar(&file, ch);
}

//-----------------------------------------------------------------------------
//! Writes str to stdout.
//!
//! @param [in] str  characters to be written to stdout
//!
//! @return 0 on success and FILE_END on failure.
//-----------------------------------------------------------------------------
int consoleWriteString(const char* str)
{
    File file;
    file.cfile = stdout;
    file.mode  = 'w';

    return writeString(&file, str);
}

//-----------------------------------------------------------------------------
//! Writes line to stdout and adds '\n' after that.
//!
//! @param [in] line  characters to be written to stdout
//!
//! @return 0 on success and FILE_END on failure.
//-----------------------------------------------------------------------------
int consoleWriteLine(const char* line)
{
    File file;
    file.cfile = stdout;
    file.mode  = 'w';

    return writeLine(&file, line);
}

//-----------------------------------------------------------------------------
//! Writes formatted string to stdout. All %c, %d and %s from str are changed to
//! char, int or char* string equivalents of args from ... respectively. 
//!
//! @param [in] str   pointer to a string containing format in which specifying
//!                   how to interpret the arguments from ... 
//! @param [in] ...   arguments
//!
//! @note The order of arguments from ... has to be the same as the order of
//!       %c, %d or %s indicators in str.
//!
//! @return number of arguments successfully interpreted or -1 if an
//!         error occurred.
//-----------------------------------------------------------------------------
int consoleWriteFormatted(const char* str, ...)
{
    File file;
    file.cfile = stdout;
    file.mode  = 'w';
    
    va_list valist;
    va_start(valist, str);

    return writeFormatted(&file, str, valist);
}

//-----------------------------------------------------------------------------
//! Calls consoleNextChar() until '\n' ("clears buffer of garbage")
//-----------------------------------------------------------------------------
void consoleMoveToNextLine()
{
    while (consoleNextChar() != '\n') ;
}

//-----------------------------------------------------------------------------
//! @param [in]  value
//!
//! @return the number of digits in value (not counting the sign).
//-----------------------------------------------------------------------------
size_t numberOfDigits (int value)
{
    size_t digits = 0;
    int    temp   = value;
    while (temp / 10 != 0)
    {
        temp /= 10;
        digits++;
    }

    return digits + 1;
}

//-----------------------------------------------------------------------------
//! Converts int to string. 
//!
//! @param [in]  value   value to be converted
//! @param [out] str     string to which to write result
//! @param [in]  digits  number of digits of value (not counting the sign)
//!
//! @return str or NULL on failure.
//-----------------------------------------------------------------------------
char* intToStr (int value, char* str, size_t digits)
{
    if (str == NULL)
        return NULL;

    int i = 0;
    if (value < 0)
    {
        str[0] = '-';
        i = 1;
        value = -value;
    }

    int currentDigit = 0;
    for (int tensPower = lround(pow(10, digits - 1)); 
         tensPower >= 1; 
         tensPower /= 10, i++)
    {
        currentDigit = (value / tensPower) % 10;

        str[i] = '0' + currentDigit;
    }

    str[i] = STRING_TERMINATION;

    return str;
}

//-----------------------------------------------------------------------------
//! Converts int to string. 
//!
//! @param [in]  value  value to be converted
//! @param [out] str    string to which to write result
//!
//! @return str or NULL on failure.
//-----------------------------------------------------------------------------
char* intToStr (int value, char* str)
{
    return intToStr(value, str, numberOfDigits(value));
}

//-----------------------------------------------------------------------------
//! Calculates the length of str (not including the string termination symbol). 
//!
//! @param [in]  str  
//!
//! @return number of characters in str or 0 on failure.
//-----------------------------------------------------------------------------
size_t strLength (const char* str)
{
    if (str == NULL)
        return 0;

    size_t length = 0;
    for (char* currentSymbol = (char*) str; *currentSymbol != STRING_TERMINATION; currentSymbol++)
        length++;

    return length;
}

//-----------------------------------------------------------------------------
//! Compares two strings.
//! @param [in]  str1
//! @param [in]  str2
//! @return positive integer if str1 > str2, negative if str1 < str2
//!         and 0 if str1 = str2
//-----------------------------------------------------------------------------
int strCompare (const unsigned char* str1, const unsigned char* str2)
{
    assert(str1);
    assert(str2);

    unsigned char* ptr1 = (unsigned char*) str1;
    unsigned char* ptr2 = (unsigned char*) str2;
    while (*ptr1 == *ptr2)
    {
        if (*ptr1 == STRING_TERMINATION) 
            return 0;

        ptr1++;
        ptr2++;
    }

    return *ptr1 - *ptr2;
}

//-----------------------------------------------------------------------------
//! Appends the content of source to destination. Undefined behavior if there's
//! not enough space in destination. 
//!
//! @param [out]  destination
//! @param [in]   source
//!
//! @return destination
//-----------------------------------------------------------------------------
char* strConcatenate (char* destination, const char* source)
{
    if (destination == NULL)
        return NULL;
    if (source == NULL)
        return destination;

    char* ptrDestination = destination;
    while (*ptrDestination != STRING_TERMINATION) ptrDestination++;

    for (char* ptrSource = (char*) source; *ptrSource != STRING_TERMINATION; ptrSource++, ptrDestination++)
        *ptrDestination = *ptrSource;

    *ptrDestination = STRING_TERMINATION;
    
    return ptrDestination;
}

//-----------------------------------------------------------------------------
//! Finds the firt occurrence of substr in str and returns the pointer to it. 
//!
//! @param [in]  str
//! @param [in]  substr  
//!
//! @return pointer to the first occurrence of substr in str or NULL on 
//!         failure.
//-----------------------------------------------------------------------------
char* strFind (const char* str, const char* substr)
{
    if (str == NULL || substr == NULL)
        return NULL;

    size_t substrLength = strLength(substr);
    
    int    start            = -1;
    size_t currentlyMatched = 0;
    for (size_t currentIndex = 0; str[currentIndex] != STRING_TERMINATION; currentIndex++)
    {
        if (currentlyMatched == substrLength)
            break;
        
        if (substr[currentlyMatched] == STRING_TERMINATION)
            return NULL;

        if (str[currentIndex] == substr[currentlyMatched])
        {
            if (start == -1)
                start = currentIndex;

            currentlyMatched++;
        } 
        else
        {
            start            = -1;
            currentlyMatched = 0;
        }
    }

    if (start == -1)
        return NULL;

    return (char*)&str[start];
}

//-----------------------------------------------------------------------------
//! Finds the firt occurrence of substr in str and returns the pointer to it. 
//! Checks only first maxSymbolsToCheck symbols of str.
//!
//! @param [in]  str
//! @param [in]  substr 
//! @param [in]  maxSymbolsToCheck
//!
//! @return pointer to the first occurrence of substr in str or NULL on 
//!         failure.
//-----------------------------------------------------------------------------
char* strFind (const char* str, const char* substr, size_t maxSymbolsToCheck)
{
    if (str == NULL || substr == NULL)
        return NULL;

    size_t substrLength     = strLength(substr);

    int    start            = -1;
    size_t currentlyMatched = 0;
    for (size_t currentIndex = 0; str[currentIndex] != STRING_TERMINATION && currentIndex < maxSymbolsToCheck; currentIndex++)
    {
        if (currentlyMatched == substrLength)
            break;

        if (substr[currentlyMatched] == STRING_TERMINATION)
            return NULL;

        if (str[currentIndex] == substr[currentlyMatched])
        {
            if (start == -1)
                start = currentIndex;

            currentlyMatched++;
        } 
        else
        {
            start            = -1;
            currentlyMatched = 0;
        }
    }

    if (currentlyMatched != substrLength)
        return NULL;

    return (char*)&str[start];
}

//-----------------------------------------------------------------------------
//! Counts how many instances of symbol are in str.
//!
//! @param [in]  str
//! @param [in]  symbol 
//!
//! @return number of occurrences of symbol in string. On error returns 0.
//-----------------------------------------------------------------------------
size_t strNumOfOccurrences(const char* str, char symbol)
{
    if (str == NULL)
        return 0;
    
    size_t numOfOccurrences = 0;
    for(; *str != STRING_TERMINATION; str++)
        if (*str == symbol)
            numOfOccurrences++;

    return numOfOccurrences;
}

//-----------------------------------------------------------------------------
//! Counts how many instances of symbol are in str, but checks only
//! maxSymbolsToCheck symbols.
//!
//! @param [in]  str
//! @param [in]  symbol 
//! @param [in]  maxSymbolsToCheck 
//!
//! @return number of occurrences of symbol in first maxSymbolsToCheck symbols
//!         of string. On error returns 0.
//-----------------------------------------------------------------------------
size_t strNumOfOccurrences(const char* str, char symbol, size_t maxSymbolsToCheck)
{
    if (str == NULL)
        return 0;

    size_t numOfOccurrences = 0;
    size_t symbolsChecked   = 0;
    for(; *str != STRING_TERMINATION && symbolsChecked < maxSymbolsToCheck; str++, symbolsChecked++)
        if (*str == symbol)
            numOfOccurrences++;

    return numOfOccurrences;
}

//-----------------------------------------------------------------------------
//! Tells whether or not ch is a punctuation mark or a digit.
//!
//! @param [in]  ch
//!
//! @return 1 if ch is a punctuation mark and 0 otherwise.
//-----------------------------------------------------------------------------
int isPunctuationMark(unsigned char ch)
{
    return (ch >= ' ' && ch <= '@') ||
           (ch >= '[' && ch <= '`') ||
           (ch >= '{' && ch <= 191);
}

//-----------------------------------------------------------------------------
//! Tells whether or not ch is a latin letter.
//!
//! @param [in]  ch
//!
//! @return 1 if ch is a latin letter and 0 otherwise.
//-----------------------------------------------------------------------------
int isLatinLetter(unsigned char ch)
{
    return (ch >= (unsigned char)'a' && ch <= (unsigned char)'z') || 
           (ch >= (unsigned char)'A' && ch <= (unsigned char)'Z');
}

//-----------------------------------------------------------------------------
//! Tells whether or not ch is a cyrilic letter.
//!
//! @param [in]  ch
//!
//! @return 1 if ch is a cyrilic letter and 0 otherwise.
//-----------------------------------------------------------------------------
int isCyrilicLetter(unsigned char ch)
{
    return (ch >= (unsigned char)'à' && ch <= (unsigned char)'ÿ') || 
           (ch >= (unsigned char)'À' && ch <= (unsigned char)'ß');
}

//-----------------------------------------------------------------------------
//! Converts upper-case letters to lower-case. If ch is not an upper-case
//! letter then return ch without any changes.
//!
//! @param [in]  ch
//!
//! @return lower-case version of ch.
//-----------------------------------------------------------------------------
unsigned char toLowerCase(unsigned char ch)
{
    if (ch >= 'A' && ch <= 'Z')
        return 'a' + ch - 'A';

    if (ch >= (unsigned char)'À' && ch <= (unsigned char)'ß')
        return (unsigned char)'à' + ch - (unsigned char)'À';

    return ch;
}