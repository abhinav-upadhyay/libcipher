#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl" \
    "mnopqrstuvwxyz0123456789+/";

char *
base64_encode(const char *input, size_t len)
{
    // Allocate enough space to hold the base64 encoding
    // for each 6 bits of input the encoded output is 8 bits,
    // so the output is 4/3 times the input.
    // Additionally, since we encode 6 bits of input at a time,
    // if the input is not a multiple of 24 bits then we need
    // to add '=' to the end of the output to make it a multiple
    //  of 24 bits, so the output length is 4/3 times the input
    // length plus 2 for the '=' and one byte for the null terminator
    size_t output_len = (len * 4 / 3 + 1) + 2 + 1; 
    char *out = malloc(output_len);
    if (out == NULL)
        err(EXIT_FAILURE, "malloc failed");


    size_t idx = 0;

    // we scan 6 bits of input at a time and map it to one
    // of the bytes from base64_chars
    do {
        out[idx++] = b64_chars[(input[0] & 0xFC) >> 2];
        if (len == 1) {
            out[idx++] = b64_chars[(input[0] & 0x03) << 4];
            out[idx++] = '=';
            out[idx++] = '=';
            break;
        }

        out[idx++] = b64_chars[(input[0] & 0x03) << 4 | (input[1] & 0xF0) >> 4];
        if (len == 2) {
            out[idx++] = b64_chars[(input[1] & 0x0F) << 2];
            out[idx++] = '=';
            break;
        }

        out[idx++] = b64_chars[(input[1] & 0x0F) << 2 | (input[2] & 0xC0) >> 6];
        out[idx++] = b64_chars[input[2] & 0x3F];
        input += 3;

    } while (len -= 3);
    out[idx] = 0;
    return out;
}

static const int unbase64 [] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
  53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 0, -1, -1, -1,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
  42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

char *
base64_decode(const char *input, size_t len)
{
    if ((len & 0x03)) {
        warnx("input length expected to be a multiple of 4 bytes, got %zu bytes", len);
        return NULL;
    }
    size_t output_len = (len * 3) / 4 + 2;
    char *out = malloc(output_len);
    if (out == NULL)
        err(EXIT_FAILURE, "malloc failed");
    size_t offset = 0;
    
    do {
        for (int i = 0; i <= 3; i++) {
            if (input[i] > 127 || unbase64[input[i]]  == -1) {
                warnx("invalid base64 character %c:", input[i]);
                free(out);
                return NULL;
            }
        }

        out[offset++] = (unbase64[input[0]] << 2) | 
            ((unbase64[input[1]] & 0x30) >> 4);
        if (input[2] != '=') {
            out[offset++] = ((unbase64[input[1]] & 0x0F) << 4) | 
                ((unbase64[input[2]] & 0x3C) >> 2);
        }

        if (input[3] != '=') {
            out[offset++] = ((unbase64[input[2]] & 0x03) << 6) | 
                (unbase64[input[3]]);
        }
        input += 4;
    } while (len -= 4);
    return out;
}

static void
test_base64(int argc, char **argv)
{
    char *input = "apple";
    char *expected_output = "YXBwbGU=";
    char *output = base64_encode(input, 5);
    printf("output: %s\n, expected output: %s\n", output, expected_output);
    printf("decoded: %s\n", base64_decode(output, strlen(output)));
    free(output);
}
