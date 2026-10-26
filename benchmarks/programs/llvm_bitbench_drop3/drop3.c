/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
/* Complete-output adapter around LLVM test-suite BitBench/drop3. */
#define main legacy_drop3_main
#include "upstream/drop3.c"
#undef main

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    unsigned char input[MAX_SIZE];
    unsigned char output[MAX_SIZE];
    FILE *stream = fopen(argv[1], "rb");
    if (!stream) return 2;
    size_t size = fread(input, 1, sizeof(input), stream);
    if (ferror(stream) || !feof(stream)) return 2;
    fclose(stream);
    int output_size = drop_0xx(input, output, size * 8);
    printf("%d\n", output_size);
    for (int i = 0; i < output_size; ++i) printf("%02x", output[i]);
    printf("\n");
    return 0;
}
