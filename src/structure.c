/** @file structure.c
 *  @brief Data structures
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <string.h>
#include "structure.h"
#include "debug.h"

/**
 @brief Initializer for MOBIArray structure
 
 It allocates memory for structure and for data: array of uint32_t variables.
 Memory should be freed with array_free().
 
 @param[in] len Initial size of the array
 @return MOBIArray on success, NULL otherwise
 */
MOBIArray * array_init(const size_t len) {
    MOBIArray *arr = NULL;
    arr = malloc(sizeof(MOBIArray));
    if (arr == NULL) {
        debug_print("%s", "Array allocation failed\n");
        return NULL;
    }
    arr->data = malloc(len * sizeof(*arr->data));
    if (arr->data == NULL) {
        free(arr);
        debug_print("%s", "Array data allocation failed\n");
        return NULL;
    }
    arr->maxsize = arr->step = len;
    arr->size = 0;
    return arr;
}

/**
 @brief Inserts value into MOBIArray array
 
 @param[in,out] arr MOBIArray array
 @param[in] value Value to be inserted
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET array_insert(MOBIArray *arr, const uint32_t value) {
    if (!arr || arr->maxsize == 0) {
        return MOBI_INIT_FAILED;
    }
    if (arr->maxsize == arr->size) {
        arr->maxsize += arr->step;
        uint32_t *tmp = realloc(arr->data, arr->maxsize * sizeof(*arr->data));
        if (!tmp) {
            free(arr->data);
            arr->data = NULL;
            debug_print("%s\n", "Memory allocation failed");
            return MOBI_MALLOC_FAILED;
        }
        arr->data = tmp;
    }
    arr->data[arr->size] = value;
    arr->size++;
    return MOBI_SUCCESS;
}

/**
 @brief Helper for qsort in array_sort() function.
 
 @param[in] a First element to compare
 @param[in] b Second element to compare
 @return -1 if a < b; 1 if a > b; 0 if a = b
 */
static int array_compare(const void *a, const void *b) {
    if (*(uint32_t *) a < *(uint32_t *) b) {
        return -1;
    };
    if (*(uint32_t *) a > *(uint32_t *) b) {
        return 1;
    };
    return 0;
}

/**
 @brief Sort MOBIArray in ascending order.
 
 When unique is set to true, duplicate values are discarded.
 
 @param[in,out] arr MOBIArray array
 @param[in] unique Discard duplicate values if true
 */
void array_sort(MOBIArray *arr, const bool unique) {
    if (!arr || !arr->data || arr->size == 0) {
        return;
    }
    qsort(arr->data, arr->size, sizeof(*arr->data), array_compare);
    if (unique) {
        size_t i = 1, j = 1;
        while (i < arr->size) {
            if (arr->data[j - 1] == arr->data[i]) {
                i++;
                continue;
            }
            arr->data[j++] = arr->data[i++];
        }
        arr->size = j;
    }
}

/**
 @brief Get size of the array
 
 @param[in] arr MOBIArray structure
 */
size_t array_size(MOBIArray *arr) {
    return arr->size;
}

/**
 @brief Free MOBIArray structure and contained data
 
 Free data initialized with array_init();
 
 @param[in] arr MOBIArray structure
 */
void array_free(MOBIArray *arr) {
    if (!arr) { return; }
    if (arr->data) {
        free(arr->data);
    }
    free(arr);
}


/**
 @brief Create and return MOBITrie structure
 
 @return MOBITrie stucture initialized with zeroes
 */
static MOBITrie * mobi_trie_mknode(void) {
    MOBITrie *node = calloc(1, sizeof(MOBITrie));
    if (node == NULL) {
        debug_print("Memory allocation failed%s", "\n");
    }
    return node;
}

/**
 @brief Recursively free MOBITrie trie starting from node
 
 @param[in] node Starting node
 */
void mobi_trie_free(MOBITrie *node) {
    if (node) {
        mobi_trie_free(node->next);
        mobi_trie_free(node->children);
        free(node->values);
        free(node);
    }
}

/**
 @brief Insert value into array at given MOBITrie node
 
 @param[in,out] node Starting node
 @param[in] value Value to be inserted
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
static MOBI_RET mobi_trie_addvalue(MOBITrie *node, char *value) {
    if (node->values) {
        size_t cnt = ++node->values_count;
        void *new_values = realloc(node->values, cnt * sizeof(*node->values));
        if (new_values == NULL) {
            debug_print("Memory allocation failed%s", "\n");
            return MOBI_MALLOC_FAILED;
        }
        node->values = new_values;
        node->values[cnt - 1] = value;
    } else {
        node->values = malloc(sizeof(*node->values));
        if (node->values == NULL) {
            debug_print("Memory allocation failed%s", "\n");
            return MOBI_MALLOC_FAILED;
        }
        node->values[0] = value;
        node->values_count = 1;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Insert key character and value (if given) at MOBITrie node
 
 @param[in,out] node Starting node
 @param[in] c Key character
 @param[in] value Value to be inserted at terminal node, or NULL if not terminal
 @return MOBITrie node: current node if value inserted (terminal), 
         children node (if transitional) or NULL on error
 */
static MOBITrie * mobi_trie_insert_char(MOBITrie *node, char c, char *value) {
    if (!node) { return NULL; }
    while (true) {
        if (node->c == c) {
            break;
        }
        if (node->next == NULL) {
            node->next = mobi_trie_mknode();
            node = node->next;
            break;
        }
        node = node->next;
    }
    if (node->c == 0) {
        node->c = c;
    }
    if (value) {
        /* terminal node */
        if (mobi_trie_addvalue(node, value) == MOBI_SUCCESS) {
            return node;
        } else {
            return NULL;
        }
    }
    if (node->children == NULL) {
        node->children = mobi_trie_mknode();
    }
    return node->children;
}

/**
 @brief Insert reversed string into MOBITrie trie
 
 @param[in,out] root Root node
 @param[in] string String to be inserted
 @param[in] value Value associated with the string
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_trie_insert_reversed(MOBITrie **root, char *string, char *value) {
    if (*root == NULL) {
        *root = mobi_trie_mknode();
        if (*root == NULL) {
            return MOBI_MALLOC_FAILED;
        }
    }
    size_t length = strlen(string);
    MOBITrie *node = *root;
    while (length > 1) {
        node = mobi_trie_insert_char(node, string[length - 1], NULL);
        if (node == NULL) {
            return MOBI_MALLOC_FAILED;
        }
        length--;
    }
    node = mobi_trie_insert_char(node, string[length - 1], value);
    if (node == NULL) {
        return MOBI_MALLOC_FAILED;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Fetch values for key c from MOBITrie trie's current level starting at node
 
 @param[in,out] values Array of values to be fetched
 @param[in,out] values_count Array size
 @param[in] node MOBITrie node to start search
 @param[in] c Key character
 @return MOBITrie children node of the node with c key or NULL if not found
 */
MOBITrie * mobi_trie_get_next(char ***values, size_t *values_count, const MOBITrie *node, const char c) {
    if (!node) { return NULL; }
    while (node) {
        if (node->c == c) {
            *values = (char**) node->values;
            *values_count = node->values_count;
            return node->children;
        }
        node = node->next;
    }
    return NULL;
}

#if 0
/* Simple imprementation of binary tree, storing key strings 
 and associated arrays of values
 currently not used, save for later */

typedef struct MOBIBtree {
    char *key; /**< key */
    char **array; /**< array of strings */
    size_t value_count; /**< strings count */
    struct MOBIBtree *left; /**< left child */
    struct MOBIBtree *right; /**< right child */
} MOBIBtree;

/**
 @brief Search MOBIBtree tree for string key
 
 @param[in] node MOBIBtree node to start search
 @param[in] key Key string
 @return MOBIBtree node or NULL if not found
 */
MOBIBtree *mobi_btree_search(MOBIBtree *node, const char *key) {
    MOBIBtree *found = NULL;
    int compare = strcmp(key, node->key);
    if (compare < 0) {
        found = mobi_btree_search(node->left, key);
    } else if (compare > 0) {
        found = mobi_btree_search(node->right, key);
    } else {
        found = node;
    }
    return found;
}

/**
 @brief Insert key and value (into array) into MOBIBtree tree
 
 @param[in] node MOBIBtree root node
 @param[in] key Key string
 @param[in] value Value string will be inserted into array
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_btree_insert(MOBIBtree **node, char *key, char *value) {
    MOBI_RET ret = MOBI_SUCCESS;
    if (*node == NULL) {
        *node = malloc(sizeof(MOBIBtree));
        if (*node == NULL) {
            return MOBI_MALLOC_FAILED;
        }
        (*node)->key = key;
        (*node)->value_count = 1;
        (*node)->array = malloc(sizeof(*(*node)->array));
        (*node)->array[0] = value;
        (*node)->left = NULL;
        (*node)->right = NULL;
        return MOBI_SUCCESS;
    }
    int compare = strcmp(key, (*node)->key);
    if (compare < 0) {
        ret = mobi_btree_insert(&(*node)->left, key, value);
    } else if (compare > 0) {
        ret = mobi_btree_insert(&(*node)->right, key, value);
    } else {
        size_t cnt = ++(*node)->value_count;
        char **new_array = realloc((*node)->array, cnt * sizeof(*(*node)->array));
        if (new_array) {
            (*node)->array = new_array;
            (*node)->array[cnt - 1] = value;
        } else {
            return MOBI_MALLOC_FAILED;
        }
    }
    return ret;
}
#endif
