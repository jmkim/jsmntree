#ifndef JSMNTREE_H_
#define JSMNTREE_H_ 1

#include <stddef.h>
#include "jsmn/jsmn.h" /* jsmntok_t (http://zserge.com/jsmn.html) */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * JSON type identifier for jsmntree, similar with jsmn.h. Basic types
 * are:
 *      o Object
 *      o Array
 *      o String
 *      o Number
 *      o Boolean (true/false)
 *      o null
 */
typedef enum
{
    JSMNTREE_UNDEFINED  = 0,
    JSMNTREE_OBJECT     = 1,
    JSMNTREE_ARRAY      = 2,
    JSMNTREE_STRING     = 3,
    /* To avoid conflict with JSMN_PRIMITIVE, do not set 4 */
    JSMNTREE_NUMBER     = 5,
    JSMNTREE_BOOLEAN    = 6,
    JSMNTREE_NULL       = 7,
    JSMNTREE_MEMBER     = 8,
    JSMNTREE_ELEMENT    = 9,
    JSMNTREE_MEMBER_ARRAY   = 10,
    JSMNTREE_ELEMENT_ARRAY  = 11,
}
jsmntreetype_t;

enum jsmntree_error
{
    /* Invalid token */
    JSMNTREE_ERROR_INVTOK   = -4,
};

/**
 * A name/value pair.
 * @param       name        Name (string)
 * @param       value       Value
 * @param       value_type  Type of `value' (object, array, string etc.)
 */
typedef struct
{
    char *              name;
    void *              value;
    jsmntreetype_t      value_type;
}
jsmntree_member;

/**
 * A value, which can be a string, or a number, or boolean, or null, or
 * an object or an array.
 * @param       value       Value
 * @param       value_type  Type of `value' (object, array, string etc.)
 */
typedef struct
{
    void *              value;
    jsmntreetype_t      value_type;
}
jsmntree_element;

/**
 * An object, which is an unordered set of name/value pairs.
 * @param       size        Size of array `members'
 * @param       max_size    Max size of array `members'
 * @param       members     Array of name/value pair
 */
typedef struct
{
    size_t              size;
    size_t              max_size;
    jsmntree_member **  members;
}
jsmntree_object;

/**
 * An array, which is an ordered collection of values.
 * @param       size        Size of array `elements'
 * @param       max_size    Max size of array `elements'
 * @param       elements    Array of value
 */
typedef struct
{
    size_t              size;
    size_t              max_size;
    jsmntree_element ** elements;
}
jsmntree_array;

/**
 * Make a JSON tree.
 */
jsmntree_object *
jsmntree_make_tree(const char * js, const size_t len,
                    const jsmntok_t * tokens, const unsigned int num_tokens);

/**
 * Free the memory space of JSON tree.
 */
void jsmntree_free_tree(jsmntree_object * jsmntree);

void jsmntree_fprint_tree(FILE * stream, jsmntree_object * object);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! JSMNTREE_H_ */
