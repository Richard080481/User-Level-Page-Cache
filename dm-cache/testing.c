#include "cache_api.h"
#include "spdk.h"

#define FILE_NAME "testfile"

// 隨機4K讀寫
static void send_pio(void* buffer) {
    unsigned page_index = 0;
    operate operation = WRITE;
    if (operation == WRITE) {
        memset(buffer, '\0', PAGE_SIZE / 2);
        memset(buffer + PAGE_SIZE / 2, 'A' + rand() % 25, PAGE_SIZE / 2);
    }
    unsigned pio_cnt = 1;
    struct pio* head = create_pio(FILE_NAME, 0, page_index, operation, buffer, pio_cnt);
    submit_pio(head);
    free_pio(head);
}

int main(int argc, char* argv[]) {
    // 開啟 ssd-cache
    force_exit_ssd_cache();
    printf("%d init rc = %d\n", getpid(), init_ssd_cache());

    void* buffer = alloc_dma_buffer(PAGE_SIZE);

    send_pio(buffer);

    free_dma_buffer(buffer);

    buffer = alloc_dma_buffer(PAGE_SIZE);

    operate operation = READ;
    struct pio* head = create_pio(FILE_NAME, 0, 0, operation, buffer, 1);
    submit_pio(head);
    free_pio(head);

    printf("first part : %s\n", (char*)buffer);
    printf("second part : %s\n", (char*)buffer + PAGE_SIZE / 2);

    free_dma_buffer(buffer);

    // 關閉 ssd-cache
    info_ssd_cache();
    printf("%d exit rc = %d\n", getpid(), exit_ssd_cache());
    return 0;
}