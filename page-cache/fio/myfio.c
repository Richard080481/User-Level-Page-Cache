#include "config-host.h"
#include "fio.h"
#include "optgroup.h"
#include "spdk_internal/event.h"

#include "upage.h"
#include "umalloc.h"
#include "utypes.h"

static int myfio_init(struct thread_data *td) {
    if (init_page_cache() != 0) {
        fprintf(stderr, "Failed to initialize page cache\n");
        return 1;
    }
    fprintf(stderr, "Page cache initialized successfully\n");

    // THis is warm up
    // uFILE* file = uopen("testfile", "w");
    // void* temp_page = umalloc_dma(4096);
    // char data = 'a';
    // memset(temp_page, data, 4096);
    // for (unsigned long long int i = 0;i < 262144;i++)
    // {
    //     useek(file, i << 12);
    //     uwrite(temp_page, sizeof(char), 4096, file);
    // }
    // ufree(temp_page);
    // uclose(file);

    // printf("warm up finish!\n");

    return 0;
}

static void myfio_cleanup(struct thread_data *td) { exit_page_cache(); }

static int myfio_iomem_alloc(struct thread_data *td, size_t total_mem) {
    td->orig_buffer = alloc_dma_buffer(PAGE_SIZE << 3);
    if (td->orig_buffer == NULL) {
        fprintf(stderr, "Failed to allocate DMA memory\n");
        return 1;
    }
    return 0;
}

static void myfio_iomem_free(struct thread_data *td) { ufree(td->orig_buffer); }

static enum fio_q_status myfio_queue(struct thread_data *td, struct io_u *io_u) {
    uFILE* file = NULL;
    switch (io_u->ddir) {
        case DDIR_READ:
            file = uopen(io_u->file->file_name, "r");
            useek(file , io_u->offset);

            if (!file) {
                fprintf(stderr, "Failed to open file for reading: %s\n", io_u->file->file_name);
            }
            uread(io_u->xfer_buf, sizeof(char), io_u->buflen, file);
            break;
        case DDIR_WRITE:
            file = uopen(io_u->file->file_name, "w");
            useek(file , io_u->offset);
            if (!file) {
                fprintf(stderr, "Failed to open file for writing: %s\n", io_u->file->file_name);
            }
            uwrite(io_u->xfer_buf, sizeof(char), io_u->buflen, file);
            break;
        default:
            assert(false);
            break;
    }
    uclose(file);
    return 0;
}

static int myfio_setup(struct thread_data *td) { return 0; }

static int myfio_getevents(struct thread_data *td, unsigned int min, unsigned int max,
                            const struct timespec *t) {
    return 0;
}

static struct io_u *myfio_event(struct thread_data *td, int event) { return 0; }

/* FIO imports this structure using dlsym */
struct ioengine_ops ioengine = {.name = "myfio",
                                .version = FIO_IOOPS_VERSION,
                                .setup = myfio_setup,
                                .init = myfio_init,
                                .queue = myfio_queue,
                                .getevents = myfio_getevents,
                                .event = myfio_event,
                                .open_file = generic_open_file,
                                .close_file = generic_close_file,
                                .iomem_alloc = myfio_iomem_alloc,
                                .iomem_free = myfio_iomem_free,
                                .cleanup = myfio_cleanup};

static void fio_init spdk_fio_register(void) { register_ioengine(&ioengine); }

static void fio_exit spdk_fio_unregister(void) { unregister_ioengine(&ioengine); }