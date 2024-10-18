#include "upage.h"
#include <error.h>
int init_page_cache(void)
{
    char* page_cache = spdk_malloc();
    if(init_ssd_cache())
    {
        perror("Error init ssd cache\n");
    }
}

int main()
{
    init_page_cache();
    return 0;
}