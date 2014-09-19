#ifndef H_VLLA
#define H_VLLA

#define VLLA_WIDTH  60
#define VLLA_HEIGHT 32

// TODO store width and height
typedef struct VLLA {
    int ser1_fd;
    int ser2_fd;
    uint32_t* pixels;
} VLLA;

VLLA* get_vlla(void);
VLLA* vlla_init(char* ser1, char* ser2);
void vlla_update(VLLA*);
void vlla_close(VLLA*);

#endif
