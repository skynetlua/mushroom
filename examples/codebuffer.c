

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mrsocket.h"


int main(int argc, char* argv[])
{
    mr_mem_detect(0xFFFF);

    struct mr_buffer* buffer = mr_buffer_create(4);

    char* txt = "hello world!";
    int snd_id = 0;
    for (; snd_id < 100; ++snd_id)
    {
        char tmp[128] = {0};
        char* ptr = tmp;
        uint32_t id = 16888;
        ptr = mr_encode32u(ptr, id);
        uint32_t time = mr_clock();
        ptr = mr_encode32u(ptr, time);
        ptr = mr_encode32u(ptr, snd_id);

        short txtlen = (short)strlen(txt);
        ptr = mr_encode16u(ptr, txtlen);
        memcpy(ptr, txt, txtlen);

        mr_buffer_write_push(buffer, tmp, ptr-tmp+txtlen);

        mr_buffer_write_pack(buffer);
        char* snddata = buffer->write_data;
        int sndsize = buffer->write_len;
        mr_buffer_read_push(buffer, snddata, sndsize);
    }

    int rcv_id = 0;
    while(1){
         int ret = mr_buffer_read_pack(buffer);
        if (ret > 0){
            const char* ptr = buffer->read_data;
            int read_len = buffer->read_len;

            uint32_t id = 0;
            ptr = mr_decode32u(ptr, &id);
            assert(id == 16888);

            uint32_t sndtime = 0;
            ptr = mr_decode32u(ptr, &sndtime);
            uint32_t curtime = mr_clock();
            printf("delta_time:%d\n", curtime-sndtime);

            uint32_t snd_id = 0;
            ptr = mr_decode32u(ptr, &snd_id);
            assert(rcv_id == snd_id);
            rcv_id++;

            unsigned short txtlen = 0;
            ptr = mr_decode16u(ptr, &txtlen);
            char tmp[128] = {0};
            memcpy(tmp, ptr, txtlen);
            assert(memcmp(tmp, txt, txtlen) == 0);

            assert(read_len == (ptr-buffer->read_data)+txtlen);
		}
		else {
			break;
		}
    }

    mr_buffer_free(buffer);

    mr_mem_info();
    return 0;
}



