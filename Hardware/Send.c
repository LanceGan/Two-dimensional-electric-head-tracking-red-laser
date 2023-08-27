#include "stm32f10x.h"                  // Device header


void K210_Data_Receive_Anl(uint8_t *data_buf,uint8_t num)
{
    static uint8_t cnt = 0;
    // static uint16_t preClass;
    uint8_t sum = 0;
    for(uint8_t i=0;i<(num-1);i++)  sum+=*(data_buf+i);
    if(!(sum==*(data_buf+num-1)))     return;//????????
    if(!(*(data_buf)==0xFF && *(data_buf+1)==0xFC))return;//???????
}

static uint8_t state[2] = {0};
static uint8_t _data_len[2] = {0},_data_cnt[2] = {0};
static uint8_t _buf[2][50];
/***************************************************
???: void K210_Data_Receive_Prepare(uint8_t data)
??:    K210???????
??:    uint8_t data-???????
??:    ?
??:    ?
??:    ????
****************************************************/
void K210_Data_Receive_Prepare(uint8_t data)
{
    uint8_t label=0;
    if(state[label]==0&&data==0xFF)//??1
    {
        state[label]=1;
        _buf[label][0]=data;
    }
    else if(state[label]==1&&data==0xFC)//??2
    {
        state[label]=2;
        _buf[label][1]=data;
    }
    else if(state[label]==2&&data<0XFF)//????
    {
        state[label]=3;
        _buf[label][2]=data;
    }
    else if(state[label]==3&&data<50)//????
    {
        state[label] = 4;
        _buf[label][3]=data;
        _data_len[label] = data;
        _data_cnt[label] = 0;
    }
    else if(state[label]==4&&_data_len>0)//???????,?????
    {
        _data_len[label]--;
        _buf[label][4+_data_cnt[label]++]=data;
        if(_data_len[label]==0) state[label] = 5;
    }
    else if(state[label]==5)//?????????
    {
        state[label] = 0;
        _buf[label][4+_data_cnt[label]]=data;
        K210_Data_Receive_Anl(_buf[label], _data_cnt[label] + 5);
        // Openmv_Data_Receive_Anl_1(_buf[label],_data_cnt[label]+5,&camera1);
    }
    else state[label] = 0;
}


