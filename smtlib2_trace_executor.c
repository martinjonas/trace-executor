/* -*- C++ -*-
 *
 * A simple SMT-LIB v2 trace executor.
 *
 * Author: Tjark Weber <tjark.weber@it.uu.se> (2014-2015)
 * Author: Alberto Griggio <griggio@fbk.eu>
 *
 * Copyright (C) 2011 Alberto Griggio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

typedef enum { STATUS_CMD, STATUS_EOF, STATUS_CHECK } send_status;


typedef struct {
    char *data;
    size_t idx;
    size_t size;
} Buffer;


static void init_buf(Buffer *buf)
{
    buf->data = malloc(4096);
    buf->size = 4096;
    buf->idx = 0;
}


static void putbuf(Buffer *buf, char c)
{
    assert(buf->data);
    if (buf->idx == buf->size) {
        buf->size *= 2;
        buf->data = realloc(buf->data, buf->size);
    }
    buf->data[buf->idx++] = c;
}


static send_status get_next_cmd(FILE *in, Buffer *sendbuf)
{
    send_status status = STATUS_CMD;
    int first = 1;
    int comment = 0;
    int parens = 0;
    int escaped = 0;
    int quoted = 0;
    const char *check_sat_cmd = "check-sat";
    int check_sat_idx = -1;

    while (1) {
        int e;
        int c = fgetc(in);
        if (c == EOF) {
            return STATUS_EOF;
        }
        if (comment) {
            check_sat_idx = -1;
            if (c == '\n') {
                comment = 0;
            }
            goto sendc;
        }
        if (c == '\\') {
            escaped = !escaped;
            goto sendc;
        }
        e = escaped;
        escaped = 0;
        if (!e || quoted) {
            if (c == '|') {
                check_sat_idx = -1;
                quoted = !quoted;
                goto sendc;
            }
            if (!quoted) {
                switch (c) {
                case ';':
                    comment = 1;
                    check_sat_idx = -1;
                    break;
                case '|':
                    quoted = 1;
                    check_sat_idx = -1;
                    break;
                case '(':
                    ++parens;
                    first = 0;
                    if (parens == 1) {
                        check_sat_idx = 0;
                    } else {
                        check_sat_idx = -1;
                    }
                    break;
                case ')':
                    --parens;
                    if (parens == 0 && check_sat_idx == 9) {
                        status = STATUS_CHECK;
                    }
                    check_sat_idx = -1;
                    break;
                default:
                    if (check_sat_idx >= 0 && check_sat_idx < 9) {
                        if (!isspace(c)) {
                            if (c == check_sat_cmd[check_sat_idx]) {
                                ++check_sat_idx;
                            } else {
                                check_sat_idx = -1;
                            }
                        }
                    } else if (check_sat_idx == 9) {
                        if (!isspace(c)) {
                            check_sat_idx = -1;
                        }
                    } else {
                        check_sat_idx = -1;
                    }
                }
            }
        } else {
            check_sat_idx = -1;
        }

      sendc:
        putbuf(sendbuf, c);
        if (parens == 0 && !first) {
            return status;
        }
    }
}


static int str_eq_skipws(const char *s, const char *target)
{
    const char *c = s;
    size_t l = strlen(target);
    while (*c && isspace(*c)) ++c;
    if (strncmp(c, target, l) == 0) {
        c += l;
        while (*c) {
            if (!isspace(*c)) return 0;
            ++c;
        }
        return 1;
    } else {
        return 0;
    }
}


static const char *get_line(FILE *src, Buffer *buf)
{
    assert(buf->data);

    buf->idx = 0;
    while (1) {
        int c = fgetc(src);
        if (c == EOF) {
            break;
        }
        putbuf(buf, c);
        if (c == '\n') {
            break;
        }
    }
    putbuf(buf, 0);
    return buf->data;
}


#define SKIPWS(s, out)                              \
    while (*s) {                                    \
        if (!isspace(*s)) {                         \
            fputc(*s, out);                         \
        }                                           \
        ++s;                                        \
    }


#define EXIT_WRONG_RESULT 2
#define EXIT_ERROR        1


static void usage(const char *progname)
{
    printf("Usage: %s SOLVER BENCHMARK_WITH_SOLUTIONS\n", progname);
    exit(EXIT_ERROR);
}


#define READ_END 0
#define WRITE_END 1


int main(int argc, char **argv)
{
    pid_t pid;
    int fds_to[2];
    int fds_from[2];
    FILE *to_child;
    FILE *from_child;
    send_status st;
    const char *response = NULL;
    FILE *benchmark = NULL;
    Buffer sendbuf;
    Buffer recvbuf;
    Buffer solbuf;
    int query_count = 0;
    int max_query_count = 0;

    if (argc != 3) {
        usage(argv[0]);
    }

    if (pipe(fds_to) != 0 || pipe(fds_from) != 0) {
        return EXIT_ERROR;
    }

    pid = fork();
    if (pid == 0) {  // child
        close(fds_to[WRITE_END]);
        dup2(fds_to[READ_END], STDIN_FILENO);
        close(fds_from[READ_END]);
        dup2(fds_from[WRITE_END], STDOUT_FILENO);

        // redirect stderr to /dev/null
        int tmp_fd = open("/dev/null", O_WRONLY);
        dup2(tmp_fd, STDERR_FILENO);

        char *execargs[2];
        execargs[0] = argv[1];
        execargs[1] = NULL;

        execv(execargs[0], execargs);
        printf("Cannot execute: %s\n", execargs[0]);
        return EXIT_ERROR;
    } else if (pid < 0) {
        return EXIT_ERROR;
    }

    close(fds_to[READ_END]);
    to_child = fdopen(fds_to[WRITE_END], "w");
    close(fds_from[WRITE_END]);
    from_child = fdopen(fds_from[0], "r");

    if (to_child == NULL || from_child == NULL) {
      return EXIT_ERROR;
    }

    init_buf(&sendbuf);
    init_buf(&recvbuf);
    init_buf(&solbuf);

    fputs("(set-option :print-success true)\n", to_child);
    fflush(to_child);
    do {  // ignore empty lines
      response = get_line(from_child, &recvbuf);
    } while (str_eq_skipws(response, ""));
    if (!str_eq_skipws(response, "success")) {
        printf("BAD response to set-option command: '%s'\n", response);
        return EXIT_ERROR;
    }

    benchmark = fopen(argv[2], "r");
    if (!benchmark) {
        printf("BAD benchmark (cannot open): '%s'\n", argv[2]);
        return EXIT_ERROR;
    }

    // parse expected solutions into solbuf
    while (1) {
        response = get_line(benchmark, &sendbuf);
        if (str_eq_skipws(response, "sat") || str_eq_skipws(response, "unsat")) {
            while (*response) {
                putbuf(&solbuf, *response);
                ++response;
            }
            putbuf(&solbuf, 0); // also put terminating 0
            ++max_query_count;
        } else {
            break;
        }
    }
    while (!str_eq_skipws(response, "--- BENCHMARK BEGINS HERE ---")) {
        if (!str_eq_skipws(response, "sat") &&
            !str_eq_skipws(response, "unsat") &&
            !str_eq_skipws(response, "unknown")) {
            printf("BAD expected result: '%s'\n", response);
            return EXIT_ERROR;
        }
        response = get_line(benchmark, &sendbuf);
    }
    sendbuf.idx = 0;
    solbuf.idx = 0;

    while (query_count < max_query_count) {
        st = get_next_cmd(benchmark, &sendbuf);

        if (st == STATUS_EOF) {
            printf("BAD benchmark: unexpected EOF\n");
            return EXIT_ERROR;
        }

        fwrite(sendbuf.data, 1, sendbuf.idx, to_child);
        sendbuf.idx = 0;
        fputc('\n', to_child);
        fflush(to_child);

        do {  // ignore empty lines
          response = get_line(from_child, &recvbuf);
        } while (str_eq_skipws(response, ""));

        switch (st) {
        case STATUS_EOF:
            assert(0);
            break;

        case STATUS_CMD:
            if (!str_eq_skipws(response, "success")) {
              printf("BAD response to status command: '%s'\n", response);
              return EXIT_ERROR;
            }
            break;

        case STATUS_CHECK:
            if (!str_eq_skipws(response, "sat") &&
                !str_eq_skipws(response, "unsat") &&
                !str_eq_skipws(response, "unknown")) {
              printf("BAD response to check command: '%s'\n", response);
              return EXIT_ERROR;
            }

            const char *expected = &solbuf.data[solbuf.idx];
            assert(str_eq_skipws(expected, "sat") || str_eq_skipws(expected, "unsat"));
            while (solbuf.data[solbuf.idx]) {
              ++solbuf.idx;
            }
            ++solbuf.idx; // skip over terminating 0

            if (str_eq_skipws(response, "sat") || str_eq_skipws(response, "unsat")) {
                if (!str_eq_skipws(response, expected)) {
                    const char *s = response;
                    printf("WRONG solver response: got ");
                    SKIPWS(s, stdout);
                    printf(", expected ");
                    s = expected;
                    SKIPWS(s, stdout);
                    fputc('\n', stdout);
                    return EXIT_WRONG_RESULT;
                }
            }

            ++query_count;

            const char *s = response;
            SKIPWS(s, stdout);
            fputc('\n', stdout);
            fflush(stdout);
            break;
        }
    }

    fputs("\n(exit)\n", to_child);
    fflush(to_child);
    wait(NULL);

    return 0;
}
