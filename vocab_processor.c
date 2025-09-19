#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Configuration ---
#define MIN_WORD_LENGTH 4      // Words must be > 3 letters
#define MAX_WORD_LENGTH 64     // Max buffer size for a single word
#define HASH_TABLE_SIZE 16384  // Should be a power of 2 for efficiency, or a prime

// --- Data Structures ---

// A node in the hash table's linked list (for collision handling)
typedef struct Node {
    char *word;
    struct Node *next;
} Node;

// The hash table structure
typedef struct HashTable {
    Node **buckets;
    size_t size;
} HashTable;

// Enum for processing mode
typedef enum {
    MODE_ADD,
    MODE_REMOVE
} ProcessMode;


// --- Hash Function ---

/**
 * @brief Simple and effective djb2 hash function for strings.
 * @param str The string to hash.
 * @return The calculated hash value.
 */
unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        // hash * 33 + c
        hash = ((hash << 5) + hash) + tolower(c);
    }
    return hash;
}

// --- Hash Table Functions ---

/**
 * @brief Creates and initializes a new hash table.
 * @param size The number of buckets in the table.
 * @return A pointer to the newly created HashTable, or NULL on failure.
 */
HashTable* create_hash_table(size_t size) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->size = size;
    table->buckets = calloc(size, sizeof(Node*)); // calloc initializes to NULL
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    return table;
}

/**
 * @brief Inserts a word into the hash table if it doesn't already exist.
 * @param table The hash table.
 * @param word The word to insert (case-insensitive).
 */
void insert_word(HashTable *table, const char *word) {
    unsigned long hash = hash_string(word);
    size_t index = hash % table->size;

    // Check if the word already exists in the bucket's linked list
    Node *current = table->buckets[index];
    while (current) {
        if (strcasecmp(current->word, word) == 0) {
            return; // Word already exists, do nothing
        }
        current = current->next;
    }

    // Word does not exist, create a new node and add it to the front
    Node *new_node = malloc(sizeof(Node));
    if (!new_node) {
        perror("Failed to allocate memory for new node");
        return;
    }
    new_node->word = strdup(word);
    if (!new_node->word) {
        perror("Failed to duplicate word string");
        free(new_node);
        return;
    }

    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
}

/**
 * @brief Removes a word from the hash table.
 * @param table The hash table.
 * @param word The word to remove (case-insensitive).
 */
void remove_word(HashTable *table, const char *word) {
    unsigned long hash = hash_string(word);
    size_t index = hash % table->size;

    Node *current = table->buckets[index];
    Node *prev = NULL;

    while (current) {
        if (strcasecmp(current->word, word) == 0) {
            // Found the word, now remove it
            if (prev) {
                prev->next = current->next;
            } else {
                // It's the first node in the bucket
                table->buckets[index] = current->next;
            }
            free(current->word);
            free(current);
            return; // Word removed
        }
        prev = current;
        current = current->next;
    }
}

/**
 * @brief Prints all words currently in the hash table to standard output.
 * @param table The hash table to print.
 */
void print_table(HashTable *table) {
    for (size_t i = 0; i < table->size; ++i) {
        Node *current = table->buckets[i];
        while (current) {
            printf("%s\n", current->word);
            current = current->next;
        }
    }
}

/**
 * @brief Frees all memory associated with the hash table.
 * @param table The hash table to free.
 */
void free_table(HashTable *table) {
    if (!table) return;
    for (size_t i = 0; i < table->size; ++i) {
        Node *current = table->buckets[i];
        while (current) {
            Node *temp = current;
            current = current->next;
            free(temp->word);
            free(temp);
        }
    }
    free(table->buckets);
    free(table);
}

// --- File Processing ---

/**
 * @brief Reads a file, extracts words, and processes them based on the mode.
 * @param filename The path to the file.
 * @param table The hash table to operate on.
 * @param mode The operation to perform (MODE_ADD or MODE_REMOVE).
 */
void process_file(const char *filename, HashTable *table, ProcessMode mode) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", filename);
        // In a real-world scenario, you might want to exit or handle this differently
        return;
    }

    char word_buffer[MAX_WORD_LENGTH];
    int char_index = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {
        if (isalpha(c)) {
            // Append character to buffer if there's space
            if (char_index < MAX_WORD_LENGTH - 1) {
                word_buffer[char_index++] = tolower(c);
            }
        } else {
            // Non-alphabetic character acts as a delimiter
            if (char_index > 0) {
                word_buffer[char_index] = '\0'; // Null-terminate the word
                if (strlen(word_buffer) >= MIN_WORD_LENGTH) {
                    if (mode == MODE_ADD) {
                        insert_word(table, word_buffer);
                    } else { // MODE_REMOVE
                        remove_word(table, word_buffer);
                    }
                }
                char_index = 0; // Reset for the next word
            }
        }
    }

    // Process the last word if the file doesn't end with a delimiter
    if (char_index > 0) {
        word_buffer[char_index] = '\0';
        if (strlen(word_buffer) >= MIN_WORD_LENGTH) {
            if (mode == MODE_ADD) {
                insert_word(table, word_buffer);
            } else {
                remove_word(table, word_buffer);
            }
        }
    }

    fclose(file);
}

// --- Main Function ---

int main(int argc, char *argv[]) {
    // 1. Validate command-line arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <source_file> <exclusion_file1> [exclusion_file2] ...\n", argv[0]);
        return 1;
    }

    // 2. Create the hash table
    HashTable *vocabulary = create_hash_table(HASH_TABLE_SIZE);
    if (!vocabulary) {
        fprintf(stderr, "Fatal: Could not create vocabulary hash table.\n");
        return 1;
    }

    // 3. Process the source file to build the initial vocabulary
    printf("Processing source file: %s\n", argv[1]);
    process_file(argv[1], vocabulary, MODE_ADD);

    // 4. Process all exclusion files to filter the vocabulary
    for (int i = 2; i < argc; ++i) {
        printf("Processing exclusion file: %s\n", argv[i]);
        process_file(argv[i], vocabulary, MODE_REMOVE);
    }
    
    // 5. Print the final vocabulary
    printf("\n--- Final Vocabulary ---\n");
    print_table(vocabulary);

    // 6. Clean up resources
    free_table(vocabulary);

    return 0;
}
