#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "jsmntree.h"
#include "jsmn/jsmn.h"
#include "algorithm/adt/stack.h"

static void *
jsmntree_alloc(const jsmntreetype_t type, const size_t max_size)
{
    void * ret = NULL;

    switch(type)
    {
    case JSMNTREE_OBJECT:
        ret = malloc(sizeof(jsmntree_object) * max_size);
        break;

    case JSMNTREE_ARRAY:
        ret = malloc(sizeof(jsmntree_array) * max_size);
        break;

    case JSMNTREE_MEMBER:
        ret = malloc(sizeof(jsmntree_member) * max_size);
        break;

    case JSMNTREE_ELEMENT:
        ret = malloc(sizeof(jsmntree_element) * max_size);
        break;

    case JSMNTREE_MEMBER_ARRAY:
        ret = malloc(sizeof(jsmntree_member *) * max_size);
        break;

    case JSMNTREE_ELEMENT_ARRAY:
        ret = malloc(sizeof(jsmntree_element *) * max_size);
        break;

    case JSMNTREE_STRING:
        ret = malloc(sizeof(char) * max_size);
        break;

    case JSMNTREE_NUMBER:
    case JSMNTREE_BOOLEAN:
        ret = malloc(sizeof(int) * max_size);
        break;

    defualt:
        ret = NULL;
        break;
    }

    return ret;
}

static void
jsmntree_dealloc(void * container)
{
    if(container != NULL)
        free(container);
}

static void *
jsmntree_init(void * ptr, const jsmntreetype_t type, const size_t max_size)
{
    switch(type)
    {
    case JSMNTREE_OBJECT:
        memset(ptr, 0, sizeof(jsmntree_object) * max_size);
        ((jsmntree_object *)ptr)->size      = 0;
        ((jsmntree_object *)ptr)->max_size  = max_size;
        break;

    case JSMNTREE_ARRAY:
        memset(ptr, 0, sizeof(jsmntree_array) * max_size);
        ((jsmntree_array *)ptr)->size       = 0;
        ((jsmntree_array *)ptr)->max_size   = max_size;
        break;

    case JSMNTREE_MEMBER:
        memset(ptr, 0, sizeof(jsmntree_member) * max_size);
        break;

    case JSMNTREE_ELEMENT:
        memset(ptr, 0, sizeof(jsmntree_element) * max_size);
        break;

    case JSMNTREE_MEMBER_ARRAY:
        memset(ptr, 0, sizeof(jsmntree_member *) * max_size);
        break;

    case JSMNTREE_ELEMENT_ARRAY:
        memset(ptr, 0, sizeof(jsmntree_element *) * max_size);
        break;

    case JSMNTREE_STRING:
        memset(ptr, 0, sizeof(char) * max_size);
        break;

    case JSMNTREE_NUMBER:
    case JSMNTREE_BOOLEAN:
        memset(ptr, 0, sizeof(int) * max_size);
        break;
    }

    return ptr;
}

jsmntree_object *
jsmntree_make_tree(const char * js, const size_t len,
                    const jsmntok_t * tokens, const unsigned int num_tokens)
{
    if(tokens[0].type == JSMN_UNDEFINED)
        return NULL;

    typedef struct
    {
        int             end;
        void *          c;
        jsmntreetype_t  c_type;
    }
    stack_node;

    jsmntree_object *   root    = jsmntree_alloc(JSMNTREE_OBJECT, 1);
    jsmntree_init(root, JSMNTREE_OBJECT, 1);

    root->members               = jsmntree_alloc(JSMNTREE_MEMBER_ARRAY, tokens[0].size);
    jsmntree_init(root->members, JSMNTREE_MEMBER_ARRAY, tokens[0].size);

    adt_stack *         s       = adt_stack_create(sizeof(stack_node));
    {
        stack_node      snode   = { tokens[0].end, root, JSMNTREE_OBJECT };
        adt_stack_push(s, &snode);
    }

    int i;
    for(i = 1; i < num_tokens && tokens[i].type != JSMNTREE_UNDEFINED; ++i)
    {
        while(adt_stack_size(s) > 0 &&
                tokens[i].start > ((stack_node *)adt_stack_top(s))->end)
            adt_stack_pop(s);

        stack_node *    tsc     = (stack_node *)adt_stack_top(s);

        if(tsc->c_type == JSMNTREE_OBJECT)
        {
#if 0
            /* Destroy all nodes */
            if(tokens[i].type != JSMNTREE_STRING)
                return NULL;
#endif

            jsmntree_object *   base_object         = (jsmntree_object *)tsc->c;
            jsmntree_member **  new_member_array    = base_object->members;
            new_member_array[base_object->size]     = jsmntree_alloc(JSMNTREE_MEMBER, 1);
            jsmntree_init(new_member_array[base_object->size], JSMNTREE_MEMBER, 1);
            
            jsmntree_member *   new_member          = new_member_array[base_object->size];
            new_member->name                        = jsmntree_alloc(JSMNTREE_STRING, tokens[i].end - tokens[i].start + 1);
            jsmntree_init(new_member->name, JSMNTREE_STRING, tokens[i].end - tokens[i].start + 1);

            char *              new_string          = new_member->name;
            strncpy(new_string, &js[tokens[i].start], tokens[i].end - tokens[i].start);
            new_string[tokens[i].end - tokens[i].start] = '\0';

            ++i;
        }

        switch(tokens[i].type)
        {
        case JSMN_OBJECT:
            {
                switch(tsc->c_type)
                {
                case JSMNTREE_OBJECT:
                    {
                        jsmntree_object *   base_object         = (jsmntree_object *)tsc->c;
                        jsmntree_member **  new_member_array    = base_object->members;
                        jsmntree_member *   new_member  = new_member_array[base_object->size];

                        new_member->value_type          = JSMNTREE_OBJECT;
                        new_member->value               = jsmntree_alloc(JSMNTREE_OBJECT, 1);
                        jsmntree_init(new_member->value, JSMNTREE_OBJECT, 1);

                        jsmntree_object *   new_object  = new_member->value;
                        new_object->members             = jsmntree_alloc(JSMNTREE_MEMBER_ARRAY, tokens[i].size);
                        jsmntree_init(new_object->members, JSMNTREE_MEMBER_ARRAY, tokens[i].size);

                        ++base_object->size;

                        stack_node snode = { tokens[i].end, new_object, JSMNTREE_OBJECT };
                        adt_stack_push(s, &snode);
                    }
                    break;

                case JSMNTREE_ARRAY:
                    {
                        jsmntree_array *    base_array          = (jsmntree_array *)tsc->c;
                        jsmntree_element ** new_element_array   = base_array->elements;
                        new_element_array[base_array->size]     = jsmntree_alloc(JSMNTREE_ELEMENT, 1);
                        jsmntree_init(new_element_array[base_array->size], JSMNTREE_ELEMENT, 1);

                        jsmntree_element *  new_element = new_element_array[base_array->size];
                        new_element->value_type         = JSMNTREE_OBJECT;
                        new_element->value              = jsmntree_alloc(JSMNTREE_OBJECT, 1);
                        jsmntree_init(new_element->value, JSMNTREE_OBJECT, 1);

                        jsmntree_object *   new_object  = new_element->value;
                        new_object->members             = jsmntree_alloc(JSMNTREE_MEMBER_ARRAY, tokens[i].size);
                        jsmntree_init(new_object->members, JSMNTREE_MEMBER_ARRAY, tokens[i].size);

                        ++base_array->size;

                        stack_node snode = { tokens[i].end, new_object, JSMNTREE_OBJECT };
                        adt_stack_push(s, &snode);
                    }
                    break;
                }
            }
            break;

        case JSMN_ARRAY:
            {
                switch(tsc->c_type)
                {
                case JSMNTREE_OBJECT:
                    {
                        jsmntree_object *   base_object         = (jsmntree_object *)tsc->c;
                        jsmntree_member **  new_member_array    = base_object->members;
                        jsmntree_member *   new_member  = new_member_array[base_object->size];

                        new_member->value_type          = JSMNTREE_ARRAY;
                        new_member->value               = jsmntree_alloc(JSMNTREE_ARRAY, 1);
                        jsmntree_init(new_member->value, JSMNTREE_ARRAY, 1);

                        jsmntree_array *    new_array   = new_member->value;
                        new_array->elements             = jsmntree_alloc(JSMNTREE_ELEMENT_ARRAY, tokens[i].size);
                        jsmntree_init(new_array->elements, JSMNTREE_ELEMENT_ARRAY, tokens[i].size);

                        ++base_object->size;

                        stack_node snode = { tokens[i].end, new_array, JSMNTREE_ARRAY };
                        adt_stack_push(s, &snode);
                    }
                    break;

                case JSMNTREE_ARRAY:
                    {
                        jsmntree_array *    base_array          = (jsmntree_array *)tsc->c;
                        jsmntree_element ** new_element_array   = base_array->elements;
                        new_element_array[base_array->size]     = jsmntree_alloc(JSMNTREE_ELEMENT, 1);
                        jsmntree_init(new_element_array[base_array->size], JSMNTREE_ELEMENT, 1);

                        jsmntree_element *  new_element = new_element_array[base_array->size];
                        new_element->value_type         = JSMNTREE_ARRAY;
                        new_element->value              = jsmntree_alloc(JSMNTREE_ARRAY, 1);
                        jsmntree_init(new_element->value, JSMNTREE_ARRAY, 1);

                        jsmntree_array *    new_array   = new_element->value;
                        new_array->elements             = jsmntree_alloc(JSMNTREE_ELEMENT_ARRAY, tokens[i].size);
                        jsmntree_init(new_array->elements, JSMNTREE_ELEMENT_ARRAY, tokens[i].size);

                        ++base_array->size;

                        stack_node snode = { tokens[i].end, new_array, JSMNTREE_ARRAY };
                        adt_stack_push(s, &snode);
                    }
                    break;
                }
            }
            break;

        case JSMN_STRING:
            {
                switch(tsc->c_type)
                {
                case JSMNTREE_OBJECT:
                    {
                        jsmntree_object *   base_object         = (jsmntree_object *)tsc->c;
                        jsmntree_member **  new_member_array    = base_object->members;
                        jsmntree_member *   new_member  = new_member_array[base_object->size];

                        new_member->value_type          = JSMNTREE_STRING;
                        new_member->value               = jsmntree_alloc(JSMNTREE_STRING, tokens[i].end - tokens[i].start + 1);
                        jsmntree_init(new_member->value, JSMNTREE_STRING, tokens[i].end - tokens[i].start + 1);

                        char *              new_string  = new_member->value;
                        strncpy(new_string, &js[tokens[i].start], tokens[i].end - tokens[i].start);
                        new_string[tokens[i].end - tokens[i].start] = '\0';

                        ++base_object->size;
                    }
                    break;

                case JSMNTREE_ARRAY:
                    {
                        jsmntree_array *    base_array          = (jsmntree_array *)tsc->c;
                        jsmntree_element ** new_element_array   = base_array->elements;
                        new_element_array[base_array->size]     = jsmntree_alloc(JSMNTREE_ELEMENT, 1);
                        jsmntree_init(new_element_array[base_array->size], JSMNTREE_ELEMENT, 1);

                        jsmntree_element *  new_element = new_element_array[base_array->size];
                        new_element->value_type         = JSMNTREE_STRING;
                        new_element->value              = jsmntree_alloc(JSMNTREE_STRING, tokens[i].end - tokens[i].start + 1);
                        jsmntree_init(new_element->value, JSMNTREE_STRING, tokens[i].end - tokens[i].start + 1);

                        char *              new_string  = new_element->value;
                        strncpy(new_string, &js[tokens[i].start], tokens[i].end - tokens[i].start);
                        new_string[tokens[i].end - tokens[i].start] = '\0';

                        ++base_array->size;
                    }
                    break;
                }
            }
            break;

        case JSMN_PRIMITIVE:
            {
                char temp_string[10] = { 0, };

                switch(tsc->c_type)
                {
                case JSMNTREE_OBJECT:
                    {
                        jsmntree_object *   base_object         = (jsmntree_object *)tsc->c;
                        jsmntree_member **  new_member_array    = base_object->members;
                        jsmntree_member *   new_member  = new_member_array[base_object->size];

                        strncpy(temp_string, &js[tokens[i].start], tokens[i].end - tokens[i].start);
                        temp_string[tokens[i].end - tokens[i].start] = '\0';

                        if(strcmp(temp_string, "null") == 0)
                        {
                            new_member->value_type      = JSMNTREE_NULL;
                            new_member->value           = NULL;
                        }
                        else if(strcmp(temp_string, "true") == 0 ||
                                strcmp(temp_string, "false") == 0)
                        {
                            new_member->value_type      = JSMNTREE_BOOLEAN;
                            new_member->value           = jsmntree_alloc(JSMNTREE_BOOLEAN, 1);
                            jsmntree_init(new_member->value, JSMNTREE_BOOLEAN, 1);

                            *(int *)new_member->value   =
                                ((strcmp(temp_string, "true") == 0) ? 1 : 0);
                        }
                        else
                        {
                            new_member->value_type      = JSMNTREE_NUMBER;
                            new_member->value           = jsmntree_alloc(JSMNTREE_NUMBER, 1);
                            jsmntree_init(new_member->value, JSMNTREE_NUMBER, 1);

                            *(int *)new_member->value   = atoi(temp_string);
                        }

                        ++base_object->size;
                    }
                    break;

                case JSMNTREE_ARRAY:
                    {
                        jsmntree_array *    base_array          = (jsmntree_array *)tsc->c;
                        jsmntree_element ** new_element_array   = base_array->elements;
                        new_element_array[base_array->size]     = jsmntree_alloc(JSMNTREE_ELEMENT, 1);
                        jsmntree_init(new_element_array[base_array->size], JSMNTREE_ELEMENT, 1);

                        jsmntree_element *  new_element = new_element_array[base_array->size];

                        strncpy(temp_string, &js[tokens[i].start], tokens[i].end - tokens[i].start);
                        temp_string[tokens[i].end - tokens[i].start] = '\0';

                        if(strcmp(temp_string, "null") == 0)
                        {
                            new_element->value_type     = JSMNTREE_NULL;
                            new_element->value          = NULL;
                        }
                        else if(strcmp(temp_string, "true") == 0 ||
                                strcmp(temp_string, "false") == 0)
                        {
                            new_element->value_type     = JSMNTREE_BOOLEAN;
                            new_element->value          = jsmntree_alloc(JSMNTREE_BOOLEAN, 1);
                            jsmntree_init(new_element->value, JSMNTREE_BOOLEAN, 1);

                            *(int *)new_element->value  =
                                ((strcmp(temp_string, "true") == 0) ? 1 : 0);
                        }
                        else
                        {
                            new_element->value_type     = JSMNTREE_NUMBER;
                            new_element->value          = jsmntree_alloc(JSMNTREE_NUMBER, 1);
                            jsmntree_init(new_element->value, JSMNTREE_NUMBER, 1);

                            *(int *)new_element->value  = atoi(temp_string);
                        }

                        ++base_array->size;
                    }
                    break;
                }
            }
            break;
        
#if 0
        default:
            /* Destroy all nodes */
            return NULL;
            break;
#endif
        }
    }

    adt_stack_destroy(s);

    return root;
}

static void jsmntree_free_object(jsmntree_object *);
static void jsmntree_free_array(jsmntree_array *);

void
jsmntree_free_tree(jsmntree_object * object)
{
    jsmntree_free_object(object);
    jsmntree_dealloc(object);
}

static void
jsmntree_free_object(jsmntree_object * object)
{
    if(object == NULL)
        return;

    while(object->size > 0)
    {
        jsmntree_dealloc(object->members[object->size - 1]->name);

        switch(object->members[object->size - 1]->value_type)
        {
        case JSMNTREE_OBJECT:
            jsmntree_free_object(object->members[object->size - 1]->value);
            break;

        case JSMNTREE_ARRAY:
            jsmntree_free_array(object->members[object->size - 1]->value);
            break;
        }

        jsmntree_dealloc(object->members[object->size - 1]->value);
        jsmntree_dealloc(object->members[object->size - 1]);
        --object->size;
    }

    jsmntree_dealloc(object->members);
}

static void
jsmntree_free_array(jsmntree_array * array)
{
    if(array == NULL)
        return;

    while(array->size > 0)
    {
        switch(array->elements[array->size - 1]->value_type)
        {
        case JSMNTREE_OBJECT:
            jsmntree_free_object(array->elements[array->size - 1]->value);
            break;

        case JSMNTREE_ARRAY:
            jsmntree_free_array(array->elements[array->size - 1]->value);
            break;
        }

        jsmntree_dealloc(array->elements[array->size - 1]->value);
        jsmntree_dealloc(array->elements[array->size - 1]);
        --array->size;
    }

    jsmntree_dealloc(array->elements);
}

static void jsmntree_fprint_object(FILE *, jsmntree_object *);
static void jsmntree_fprint_array(FILE *, jsmntree_array *);

void
jsmntree_fprint_tree(FILE * stream, jsmntree_object * object)
{
    jsmntree_fprint_object(stream, object);
    fprintf(stream, "\n");
}

static void
jsmntree_fprint_object(FILE * stream, jsmntree_object * object)
{
    if(object == NULL)
        return;

    fprintf(stream, "{ ");

    int i;

    for(i = 0; i < object->size; ++i)
    {
        fprintf(stream, "\"%s\": ", object->members[i]->name);

        switch(object->members[i]->value_type)
        {
        case JSMNTREE_OBJECT:
            jsmntree_fprint_object(stream, object->members[i]->value);
            break;

        case JSMNTREE_ARRAY:
            jsmntree_fprint_array(stream, object->members[i]->value);
            break;

        case JSMNTREE_STRING:
            fprintf(stream, "\"%s\"", (char *)object->members[i]->value);
            break;

        case JSMNTREE_NUMBER:
            fprintf(stream, "%d", *(int *)object->members[i]->value);
            break;

        case JSMNTREE_BOOLEAN:
            fprintf(stream, "%s", (((*(int *)object->members[i]->value) == 0) ? "false" : "true"));
            break;
            
        case JSMNTREE_NULL:
            fprintf(stream, "null");
            break;
        }

        if(i < object->size - 1)
            fprintf(stream, ", ");
    }
    fprintf(stream, " }");
}

static void
jsmntree_fprint_array(FILE * stream, jsmntree_array * array)
{
    if(array == NULL)
        return;

    fprintf(stream, "[ ");

    int i;

    for(i = 0; i < array->size; ++i)
    {
        switch(array->elements[i]->value_type)
        {
        case JSMNTREE_OBJECT:
            jsmntree_fprint_object(stream, array->elements[i]->value);
            break;

        case JSMNTREE_ARRAY:
            jsmntree_fprint_array(stream, array->elements[i]->value);
            break;

        case JSMNTREE_STRING:
            fprintf(stream, "\"%s\"", (char *)array->elements[i]->value);
            break;

        case JSMNTREE_NUMBER:
            fprintf(stream, "%d", *(int *)array->elements[i]->value);
            break;

        case JSMNTREE_BOOLEAN:
            fprintf(stream, "%s", (((*(int *)array->elements[i]->value) == 0) ? "false" : "true"));
            break;
            
        case JSMNTREE_NULL:
            fprintf(stream, "null");
            break;
        }

        if(i < array->size - 1)
            fprintf(stream, ", ");
    }
    fprintf(stream, " ]");
}
