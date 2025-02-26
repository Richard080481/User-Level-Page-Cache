#include "config-host.h"
#include "fio.h"
#include "optgroup.h"
#include "spdk_internal/event.h"

#include "upage.h"
#include "umalloc.h"
#include "utypes.h"

static int myfio_init(struct thread_data *td) {
    init_page_cache();
    return 0;
}

static void myfio_cleanup(struct thread_data *td) { exit_page_cache(); }

static int myfio_iomem_alloc(struct thread_data *td, size_t total_mem) {
    td->orig_buffer = umalloc_dma(PAGE_SIZE);
    return td->orig_buffer == NULL;
}

static void myfio_iomem_free(struct thread_data *td) { ufree(td->orig_buffer); }

static enum fio_q_status myfio_queue(struct thread_data *td, struct io_u *io_u) {
    uFILE* file = NULL;
    switch (io_u->ddir) {
        case DDIR_READ:
            file = uopen(io_u->file->file_name, "r");
            break;
        case DDIR_WRITE:
            file = uopen(io_u->file->file_name, "w");
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