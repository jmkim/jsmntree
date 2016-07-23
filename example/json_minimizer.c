#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <mcheck.h>

#include "../lib/jsmntree.h"

#define TOKENS_CAPACITY     20000

int
main(const int argc, const char * const argv[])
{
    /* malloc tracing */
    mtrace();

    char    fpath[PATH_MAX + 1];
    char *  buffer;
    jsmntree_object * jsontree = NULL;

    if(argc < 2)
    {
        fprintf(stderr, "Argument error\n");
        exit(1);
    }
    strcpy(fpath, argv[1]);

    {
        FILE *  fp;
        long    fsize;
        size_t  result;

        /* Open the file */
        fp = fopen(fpath, "rb");
        if(fp == NULL)
        {
            fprintf(stderr, "File error\n");
            exit(2);
        }

        /* Obtain file size */
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);

        /* Allocate memory to contain the whole file */
        buffer = (char *)malloc(sizeof(char) * fsize);
        if(buffer == NULL)
        {
            fprintf(stderr, "Memory error\n");
            exit(3);
        }

        /* Copy the file into the buffer */
        result = fread(buffer, 1, fsize, fp);
        if(result != fsize)
        {
            fprintf(stderr, "Read error\n");
            exit(4);
        }

        /* Close the file */
        fclose(fp);
    }

    {
        jsmn_parser parser;
        char buf[strlen(buffer)];

        jsmn_init(&parser);

        /* Initialise tokens */
        jsmntok_t tokens[TOKENS_CAPACITY] = { JSMN_UNDEFINED, };

        /* Parse the JSON file */
        jsmn_parse(&parser, buffer, strlen(buffer), tokens, TOKENS_CAPACITY);

#if 0
        /* Print all tokens */
        {
            int i;
            for(i = 0; tokens[i].type != JSMN_UNDEFINED; ++i)
            {
                strncpy(buf, &buffer[tokens[i].start], tokens[i].end - tokens[i].start);
                buf[tokens[i].end - tokens[i].start] = '\0';
                
                printf("%d: [%d] %d -> %d (%d): %s\n"
                        , i
                        , tokens[i].type
                        , tokens[i].start
                        , tokens[i].end
                        , tokens[i].size
                        , buf);
            }
        }
#endif

        /* Make a JSON tree */
        jsontree = jsmntree_make_tree(buffer, strlen(buffer), tokens, TOKENS_CAPACITY);

        /* Make a new JSON file using JSON tree */
        jsmntree_fprint_tree(stdout, jsontree);
    }

    /* Free the JSON tree */
    jsmntree_free_tree(jsontree);
    free(buffer);

    /* malloc tracing */
    muntrace();
    return 0;
}

#undef TOKENS_CAPACITY
