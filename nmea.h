#define IS_DIGITAL_INT(x) ((x) >= '0' && (x) <= '9')
#define CHAR_TO_DIGITAL(x) ((x) - '0')
#define DIGITAL_TO_CHAR(x) ((x) + '0')

#define THRESHOLD_CNT 4
#define THRESHOLD_SNR 20

typedef unsigned char u8;               // [0, 255];    2^8 - 1 = 255
typedef unsigned short u16;             // [0, ];       2^16 - 1 = 

typedef struct _GSV_INFO {
    u8 snrs[32];    // snrs[0~31] 为对应编号卫星的信噪比
    u16 snr_sum;   // 信噪比之和
    u8 snr_cnt;    // 可见卫星个数
    u8 status;     // 0 室内，1 不定，2 室外
} GSV_INFO;