#include "lvgl.h"

long lv_tell(lv_fs_file_t *fd)
{
    uint32_t pos = 0;
    lv_fs_tell(fd, &pos);
    rt_kprintf("\tcur pos is: %d\n", pos);
    return pos;
}

void lvgl_fs_test(void)
{
    char rbuf[30] = {0};
    uint32_t rsize = 0;
    lv_fs_file_t fd;
    lv_fs_res_t res;

    res = lv_fs_open(&fd, "S:/tmp.txt", LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        rt_kprintf("open S:/tmp.txt ERROR\n");
        return ;
    }
    lv_tell(&fd);

    lv_fs_seek(&fd, 3);
    lv_tell(&fd);

    res = lv_fs_read(&fd, rbuf, 100, &rsize);
    if (res != LV_FS_RES_OK) {
        rt_kprintf("read ERROR\n");
        return ;
    }
    lv_tell(&fd);
    rt_kprintf("READ(%d): %s",rsize , rbuf);

    lv_fs_close(&fd);
}