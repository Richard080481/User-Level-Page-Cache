#include <string.h>
#include <error.h>
#include "upage.h"
#include "cache_api.h"
#include "spdk.h"

int main(int argc, char* argv[])
{
    init_page_cache();

    // int* test_write = (int*)malloc(sizeof(int) *2);
    // int* test_read = (int*)malloc(sizeof(int) *2);
    // test_write[0] = 10;
    // test_write[1] = 11;
    // uFILE* file1 = uopen("test_output1.txt", "w");
    // uwrite(test_write, sizeof(int), 1, file1);
    // uclose(file1);
    // file1 = uopen("test_output1.txt", "r");
    // uread(test_read, sizeof(int), 1, file1);
    // uclose(file1);
    // printf("test read[0] = %d\n", test_read[0]);
    // printf("test read[1] = %d\n", test_read[1]);

    // int STRING_LENGTH = 100;
    // char* test_string = (char*)umalloc_dma(PAGE_SIZE);

    // for (int i = 0; i < STRING_LENGTH; i++)
    // {
    //     test_string[i] = '0' + (i % 10);
    // }
    // test_string[STRING_LENGTH] = '\0';

    // printf("test = %s\n", test_string);

    // void* pio_buffer = umalloc_dma(10 + STRING_LENGTH + 1);

    // // 將 unsigned int 轉換為 32 位翻轉字串的函數
    // int index = 0;
    // unsigned int temp = 1;
    // char* len = (char*)umalloc_dma(PAGE_HEADER_SIZE);

    // // 將數字的每一位按從最小位到最大位的順序存入 buffer
    // do {
    //     len[index] = '0' + (temp % 10);
    //     temp /= 10;
    //     index++;
    // } while (temp > 0);

    // // 用 '0' 填充剩餘的位數，直到 32 位
    // while (index < 10) {
    //     len[index++] = '0';
    // }
    
    // memcpy(pio_buffer, len, 10);
    // memcpy(pio_buffer + 10, test_string, STRING_LENGTH+1);

    // ufree(len);

    // printf("test = %s\n", (char*)pio_buffer);


    // struct pio* head = create_pio("testfile", 0, 0, WRITE, pio_buffer, 1);
    // submit_pio(head);
    // free_pio(head);

    // void* read_buf = umalloc_dma(PAGE_SIZE);
    // struct pio* head2 = create_pio("testfile", 0, 0, READ, read_buf, 1);
    // submit_pio(head2);
    // free_pio(head2);

    // //printf("%d\n", *pg_cnt);
    // printf("test = %s\n", (char*)read_buf);




    int STRING_LENGTH = 50000;
    char* test_string = (char*)umalloc_dma(STRING_LENGTH + 1);

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

    char* read_string = (char*)umalloc_dma(STRING_LENGTH + 1);
    memset(read_string, '\0', STRING_LENGTH);
    if (read_string == NULL)
    {
        printf("Memory allocation failed.\n");
        ufree(test_string);
        return 1;
    }
    file = uopen("test_output.txt", "r");
    if (file == NULL)
    {
        printf("Failed to open file for reading.\n");
        ufree(test_string);
        ufree(read_string);
        return 1;
    }
    unsigned int read_size = uread(read_string, sizeof(char), STRING_LENGTH, file);
    printf("read size = %d\n", read_size);
    read_string[STRING_LENGTH] = '\0';
    uclose(file);
    printf("this is test string : %s\n", test_string);
    printf("this is read string : %s\n", read_string);
    if (strcmp(test_string, read_string) == 0)
    {
        printf("Read and write are correct.\n");
    } else
    {
        printf("Mismatch in read and write.\n");
    }
    
    ufree(test_string);
    ufree(read_string);

    exit_page_cache();
    return 0;
}