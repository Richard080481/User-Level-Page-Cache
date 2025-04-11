#include <string.h>
#include <error.h>
#include "upage.h"
#include "cache_api.h"
#include "spdk.h"

int main(int argc, char* argv[])
{
    init_page_cache();

    uFILE* file = uopen("testfile", "w");
    void* temp_page = umalloc_dma(4096);
    memset(temp_page, 'a', 4096);

    unsigned long long int i = 227193;

    for (int j = 0;j < 2;j++){
        useek(file, i << 12);
        uwrite(temp_page, sizeof(char), 4096, file);
        printf("i = %llu\n", i);
    }

    ufree(temp_page);

    // file = uopen("testfile", "r");
    // temp_page = umalloc_dma(4096);

    // for (unsigned long long int i = 0;i < 262144;i++)
    // {
    //     useek(file, i << 12);
    //     uread(temp_page, sizeof(char), 4096, file);
    //     printf("i = %llu\n", i);
    // }
    // ufree(temp_page);

    exit_page_cache();
    return 0;
}

// int STRING_LENGTH = 100000;
// char* test_string = (char*)umalloc_dma(STRING_LENGTH + 1);

// if (test_string == NULL)
// {
//     printf("Memory allocation failed.\n");
//     return 1;
// }

// for (int i = 0; i < STRING_LENGTH; i++)
// {
//     test_string[i] = '0' + (i % 10);
// }
// test_string[STRING_LENGTH] = '\0';

// uFILE* file = uopen("test_output.txt", "w");
// if (file == NULL)
// {
//     printf("Failed to open file for writing.\n");
//     free(test_string);
//     return 1;
// }
// if(uwrite(test_string, sizeof(char), STRING_LENGTH, file) != (size_t)STRING_LENGTH) {printf("ERROR: uwrite\n");}

// uclose(file);
// printf("String written to file successfully.\n");

// char* read_string = (char*)umalloc_dma(STRING_LENGTH + 1);
// memset(read_string, '\0', STRING_LENGTH);
// if (read_string == NULL)
// {
//     printf("Memory allocation failed.\n");
//     ufree(test_string);
//     return 1;
// }
// file = uopen("test_output.txt", "r");
// if (file == NULL)
// {
//     printf("Failed to open file for reading.\n");
//     ufree(test_string);
//     ufree(read_string);
//     return 1;
// }
// if(uread(read_string, sizeof(char), STRING_LENGTH, file) != (size_t)STRING_LENGTH) {printf("ERROR: uread\n");}
// read_string[STRING_LENGTH] = '\0';
// uclose(file);
// printf("this is test string : %s\n", test_string);
// printf("this is read string : %s\n", read_string);
// if (strcmp(test_string, read_string) == 0)
// {
//     printf("Read and write are correct.\n");
// } else
// {
//     printf("Mismatch in read and write.\n");
// }

// ufree(test_string);
// ufree(read_string);