#include "olbin.h"


Olbin::Olbin(int protocol_version, uint16_t malloc_buf_size)
{
    this->protocol_version = protocol_version;
    if(malloc_buf_size > 0)
    {
        p_packet_buf_ = new uint8_t[malloc_buf_size];
        if(p_packet_buf_ != nullptr)
        {
            packet_buf_capacity_ = malloc_buf_size;
            is_buf_malloced_ = true;
        }
    }
    info_tx_packet_.is_init = true;
    info_rx_packet_.is_init = true;
}

InfoToMakeDXLPacket_t  
Olbin::get_command(uint8_t id, uint16_t addr, const uint8_t *p_data, uint16_t data_length)
{
    bool ret = false;
    DXLLibErrorCode_t err = DXL_LIB_OK;
    uint8_t param_len = 0;
    param_len = 2;
    begin_make_dxl_packet(&info_tx_packet_, id, protocol_version,
        DXL_INST_WRITE, 0, p_packet_buf_, packet_buf_capacity_);
    add_param_to_dxl_packet(&info_tx_packet_, (uint8_t*)&addr, param_len);
    param_len = data_length;
    add_param_to_dxl_packet(&info_tx_packet_, (uint8_t*)p_data, param_len);
    err = end_make_dxl_packet(&info_tx_packet_);

    return info_tx_packet_;
}

InfoToMakeDXLPacket_t 
Olbin::command_set_motor_speed(int motorIdx, MOTOR_DIRECTION direction, int32_t speed)
{
    if(direction == MOTOR_DIRECTION::CCW)
    {
        speed+= 1024;
    }

    uint16_t addr;
    if(motorIdx == 1)
    {
        addr = ADDRESS::MOTOR1;
    }
    else
    {
        addr = ADDRESS::MOTOR2;
    }

    return get_command(200, addr, (uint8_t*)&speed, 2);
}