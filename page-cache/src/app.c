#include <string.h>
#include <error.h>
#include <upage.h>

int main(int argc, char* argv[])
{
    init_page_cache();
/*
    int* test_write = (int*)malloc(sizeof(int) *2);
    int* test_read = (int*)malloc(sizeof(int) *2);
    test_write[0] = 10;
    test_write[1] = 11;
    uFILE* file = uopen("test_output.txt", "w");
    uwrite(test_write, sizeof(int), 1, file);
    uclose(file);
    file = uopen("test_output.txt", "r");
    uread(test_read, sizeof(int), 1, file);
    uclose(file);
    printf("test read[0] = %d\n", test_read[0]);
    printf("test read[1] = %d\n", test_read[1]);
*/
    int STRING_LENGTH = 13000;
    char* test_string = (char*)malloc(STRING_LENGTH + 1);

    if (test_string == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < STRING_LENGTH; i++)
    {
        test_string[i] = '0' + (i % 10);
    }
    test_string[STRING_LENGTH] = '\0';

    uFILE* file = uopen("test_output.txt", "w");
    if (file == NULL)
    {
        printf("Failed to open file for writing.\n");
        free(test_string);
        return 1;
    }
    if(uwrite(test_string, sizeof(char), STRING_LENGTH, file) != (size_t)STRING_LENGTH) {printf("ERROR: uwrite\n");}

    uclose(file);
    printf("String written to file successfully.\n");

    char* read_string = (char *)malloc(STRING_LENGTH + 1);
    if (read_string == NULL)
    {
        printf("Memory allocation failed.\n");
        free(test_string);
        return 1;
    }
    file = uopen("test_output.txt", "r");
    if (file == NULL)
    {
        printf("Failed to open file for reading.\n");
        free(test_string);
        free(read_string);
        return 1;
    }
    uread(read_string, sizeof(char), STRING_LENGTH, file);
    read_string[STRING_LENGTH] = '\0';
    uclose(file);
    //printf("this is test string : %s\n", test_string);
    //printf("this is read string : %s\n", read_string);
    if (strcmp(test_string, read_string) == 0)
    {
        printf("Read and write are correct.\n");
    } else
    {
        printf("Mismatch in read and write.\n");
    }

    free(test_string);
    free(read_string);

    exit_page_cache();
    return 0;
}