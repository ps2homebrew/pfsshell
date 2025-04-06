#include "util.h"


int parse_line(char *line, /* modified */
               char *(*tokens)[],
               size_t *count)
{
    enum {
        s_token_pending,      /* waiting for command to begin */
        s_parse_plain_token,  /* parsing plain token, such as foo... */
        s_parse_quoted_token, /* parsing quoted token, such as "foo... */
        s_parse_aposd_token   /* parsing quoted token, such as 'foo... */
    } state = s_token_pending;

    *count = 0;
    char *start = line;
    int term = 0;
    while (!term) {
        const char ch = *line;
        switch (state) {
            case s_token_pending:
                switch (ch) {
                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                    case '\0':
                        break; /* skip white-space */
                    case '\"':
                        state = s_parse_quoted_token;
                        start = line + 1;
                        break;
                    case '\'':
                        state = s_parse_aposd_token;
                        start = line + 1;
                        break;
                    default:
                        state = s_parse_plain_token;
                        start = line;
                        break;
                }
                break;

            case s_parse_plain_token:
                switch (ch) {
                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                    case '\0':
                        /* end of plain token */
                        *line = '\0';
                        (*tokens)[(*count)++] = start;
                        state = s_token_pending;
                        break;
                    case '\'':
                    case '\"':
                        return (-1); /* quotes are not allowed in the middle of token */
                }
                break;

            case s_parse_quoted_token:
                switch (ch) {
                    case '\0':
                        return (-1); /* missing closing quotes */
                    case '\"':
                        /* end of quoted token */
                        *line = '\0';
                        (*tokens)[(*count)++] = start;
                        state = s_token_pending;
                        break;
                }
                break;

            case s_parse_aposd_token:
                switch (ch) {
                    case '\0':
                        return (-1); /* missing closing quotes */
                    case '\'':
                        /* end of quoted token */
                        *line = '\0';
                        (*tokens)[(*count)++] = start;
                        state = s_token_pending;
                        break;
                }
                break;
        }

        ++line;
        term = (ch == '\0' || ch == '\r' || ch == '\n');
    }
    if (state != s_token_pending)
        return (-1);
    return (0);
}
