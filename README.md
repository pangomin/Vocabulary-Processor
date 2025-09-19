# What it does?
This program extract all unique words from a source document while filtering out common words, slang, or any other unwanted terms specified in exclusion files.
# Usage
**This program uses C99 standard.**
Compile using
```c
make
```
Then run and use using
```bash
./vocab_processor <source_file> <exclusion_file_1> [exclusion_file_2] ... > Output
```
# Features
• Extracts Unique Words
• Exclusion Filtering from the given files
• Case-Insensitive

**Note that you can easily change the minimum word length or hash table size by editing constants in the source code.**
