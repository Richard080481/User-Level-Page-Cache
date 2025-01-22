#include <string.h>
#include <error.h>
#include "upage.h"
#include "cache_api.h"
#include "spdk.h"

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

    ufree(test_string);
    ufree(read_string);

    exit_page_cache();
    return 0;
}
// #include "cache_api.h"
// #include "spdk.h"

// /**
//  *  @author Hyam
//  *  @date 2023/04/13
//  *  @date 2023/05/29 (modify)
//  *  @brief 壓力測試 submit pio 4KB
//  */

// // 100w  50% 301s
// // 100w 100% 65s
// // 100w 200% 55s
// // after iovector
// // 100w  50% 219s (應該要更慢才對吧? 測量日期問題?)
// // 100w 100% 64s
// // 100w 200% 54s
// #define TEST_ROUND 200000
// #define EXCEPT_HIT_RATIO 200
// #define FILE_NAME "testfile"
// const unsigned long long MAX_PAGE_INDEX =
//     (CACHE_BLOCK_NUMBER * CACHE_BLOCK_SIZE * 100ull) / (1024 * EXCEPT_HIT_RATIO * 4ull);

// // 隨機4K讀寫
// static void write_pio(void* buffer) {
//     operate operation = WRITE;
//     memset(buffer, 'A' + rand() % 25, PAGE_SIZE);
//     printf("write = %s\n", (char*)buffer);
//     unsigned pio_cnt = 1;
//     struct pio* head = create_pio(FILE_NAME, 0, 0, operation, buffer, pio_cnt);
//     submit_pio(head);
//     free_pio(head);
// }

// static void read_pio(void* buffer) {
//     operate operation = READ;
//     unsigned pio_cnt = 1;
//     struct pio* head = create_pio(FILE_NAME, 0, 0, operation, buffer, pio_cnt);
//     submit_pio(head);
//     free_pio(head);
// }

// int main(int argc, char* argv[]) {
//     // 開啟 ssd-cache
//     force_exit_ssd_cache();
//     printf("%d init rc = %d\n", getpid(), init_ssd_cache());

//     void* buffer1 = alloc_dma_buffer(PAGE_SIZE);
//     write_pio(buffer1);
//     void* buffer2 = alloc_dma_buffer(PAGE_SIZE);
//     read_pio(buffer2);
//     printf("read = %s\n", (char*)buffer2);
//     free_dma_buffer(buffer1);
//     free_dma_buffer(buffer2);
//     // 關閉 ssd-cache
//     info_ssd_cache();
//     printf("%d exit rc = %d\n", getpid(), exit_ssd_cache());
//     return 0;
// }