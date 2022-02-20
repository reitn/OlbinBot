#ifndef _OLBIN_H_
#define _OLBIN_H_

#include "protocol.h"
enum MOTOR_DIRECTION
{
    CW,
    CCW
};

enum ADDRESS
{
    MOTOR1 = 136,
    MOTOR2 = 138,
};

class Olbin
{
    public:
        Olbin(int protocol_version = 2, uint16_t malloc_buf_size = 256);
        InfoToMakeDXLPacket_t get_command(uint8_t id, uint16_t addr, const uint8_t *p_data, uint16_t data_length);
        InfoToMakeDXLPacket_t command_set_motor_speed(int motorIdx, MOTOR_DIRECTION direction, int32_t speed);
        

    private:
        int protocol_version;
        bool is_buf_malloced_;
        uint8_t *p_packet_buf_;
        uint16_t packet_buf_capacity_;
        InfoToMakeDXLPacket_t info_tx_packet_;
        InfoToParseDXLPacket_t info_rx_packet_;

        DXLLibErrorCode_t last_lib_err_;
};


#endif