#include <string.h>
#include <stdio.h>
#include "nmea.h"

const char *NMEA_SENTENCES = "$GPGSV,3,1,10,23,38,230,44,29,71,156,47,07,29,116,41,08,09,081,36*7F";
/*
GSGSV,  概括句
        分句 1
        分句 2
        分句 3
        分句 4*XX
*/

void initial_gsv_info(GSV_INFO &gsv_info) {
    memset(gsv_info.snrs, 0, sizeof(gsv_info.snrs));
    gsv_info.snr_sum = 0;
    gsv_info.snr_cnt = 0;
    gsv_info.status = 0;
}

void output_gsv_info(GSV_INFO &gsv_info) {
    printf("snrs = ");
    for(u8 i = 0; i < 32; ++i) {
        printf("%d ", gsv_info.snrs[i]);
    }
    printf("\n");

    printf("snr_cnt = %d, snr_sum = %d\n", gsv_info.snr_cnt, gsv_info.snr_sum);

    printf("status %d\n", gsv_info.status);

}

u8 NMEA_Comma_Pos(u8 *buf, u8 cx) {
    // 返回到第 cx 个逗号的长度
    u8 *p = buf;
    while(cx) {
        if(*buf == '*' || *buf < ' ' || *buf > 'z')
            return 0XFF;
        if(*buf == ',')
            cx--;
        buf++;              // 指向逗号的后继
    }
    return buf - p;
}

u8 *NMEA_Str2num(u8 *buf, u8 &value) {
    // 返回第一个非数字字符的位置，并修改引用参数 value
    u8* p = buf;
    value = 0;
    while(IS_DIGITAL_INT(*p)) {
        value = value * 10 + CHAR_TO_DIGITAL(*p);
        p++;    // 指向第一个非数字字符(指向最后一个数字字符的后继)
    }
    p++;                        // 跳过第一个非数字字符
    return p;                   // p 指向第一个非数字字符
}

void classify(GSV_INFO &gsv_info) {
    // 1. 卫星颗数 >= 阈值，且 信噪比 >= 阈值，则为室外
    // 2. 卫星颗数 < 阈值，或 信噪比 < 阈值，则为室内
    // 3. 不确定状态，暂时不需要
    if(gsv_info.snr_cnt >= THRESHOLD_CNT && gsv_info.snr_sum >= THRESHOLD_SNR) {
        gsv_info.status = 2;
        return;
    }
    if(gsv_info.snr_cnt < THRESHOLD_CNT || gsv_info.snr_sum < THRESHOLD_SNR) {
        gsv_info.status = 0;
        return;
    }
    // TODO
    // if(condition) gsv_info.status = 1
}

void update(const u8 num, const u8 snr, GSV_INFO &gsv_info) {
    // 1. 无效卫星
    if(snr == 0)    return;             // 1. 无效卫星
    // 2. 新卫星
    if(gsv_info.snrs[num] == 0) {       // 2. 新卫星
        gsv_info.snrs[num] = snr;       // 添加可见卫星
        gsv_info.snr_cnt++;             // 更新卫星数量
        gsv_info.snr_sum += snr;        // 更新信噪比之和
        classify(gsv_info);             // 判断是内外
        return;
    }
    // 3. 旧卫星
    gsv_info.snr_sum += (snr - gsv_info.snrs[num]);
    gsv_info.snrs[num] = snr;           // 3. 旧卫星
    classify(gsv_info);
    return;
}

u8* NMEA_GPGSV_sentence(u8* p, GSV_INFO &gsv_info) {
    u8 num = 0;
    u8 snr = 0;
    u8 posx = 0;
    for(u8 _ = 0; _ < 4; ++_) {
        p = NMEA_Str2num(p, num);           // 提取 num，更新 p
        posx = NMEA_Comma_Pos(p, 2);        // p 到逗号的长度
        p += posx;                          // p 指向第 cx = 2 个逗号的后继
        p = NMEA_Str2num(p, snr);           // 提取 snr，更新 p，p 指向新的数字字符
        update(num, snr, gsv_info);
        
        if(gsv_info.status == 2)            // 如果检测到室外，则提退出
            break;
    }
    return p;                               // 返回字符指针位置
}

void NMEA_GPGSV_sentences(const char* buf, GSV_INFO &gsv_info) {
    u8 *p, posx = 0;
    p = (u8*)strstr((const char*)buf, "$GPGSV");
    while(*p != '\0') {
        posx = NMEA_Comma_Pos(p, 4);            // '$' 到 分句 1 的长度
        p += posx;                              // 指向分句 1
        p = NMEA_GPGSV_sentence(p, gsv_info);   // 提取分句 1 ～ 分句 4 的信息
        if(gsv_info.status == 2)
            break;
        p = (u8*)strstr((const char*)buf, "$GPGSV");
    }
    return;
}


int main() {
    GSV_INFO gsv_info;
    initial_gsv_info(gsv_info);

    NMEA_GPGSV_sentences(NMEA_SENTENCES, gsv_info);
    
    output_gsv_info(gsv_info);    
    return 0;
}
