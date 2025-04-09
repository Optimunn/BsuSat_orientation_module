#include "satframe.hpp"

void frame::make(sat_t *frame, struct can_frame *can, type_u_t type)
{
    can->can_id = (type << 29) |
                  (frame->priority << 27) |
                  (frame->cmd_resp << 26) |
                  (frame->cmd_id << 17) |
                  (frame->cmd_last << 16) |
                  (frame->cmd_chunks << 8) |
                  (frame->dst << 4) |
                  (frame->src);
}

void frame::parse(sat_t *frame, struct can_frame *can)
{
    frame->sad_flag = (can->can_id >> 29) & 0x7;
    frame->priority = (can->can_id >> 27) & 0x3;
    frame->cmd_resp = (can->can_id >> 26) & 0x1;
    frame->cmd_id = (can->can_id >> 17) & 0x1FF;
    frame->cmd_last = (can->can_id >> 16) & 0x1;
    frame->cmd_chunks = (can->can_id >> 8) & 0xFF;
    frame->dst = (can->can_id >> 4) & 0xF;
    frame->src = (can->can_id) & 0xF;
}

inline bool frame::check(struct can_frame *can, const uint8_t module_id)
{
    return (((can->can_id >> 4) & 0xF) != module_id) ? 1 : 0;
}

frame::read_ex_t frame::calculate(struct can_frame *can, sat_t *frame, const uint8_t module_id)
{
    if (check(can, module_id))
        return MODULE_ERROR;

    parse(frame, can);

    if ((frame->cmd_resp) == 0x1)
        return RESPONSE_OK;
    return REQUEST_OK;
}

frame::decode_t frame::decode(sat_t *frame, struct can_frame *can, uint8_t *data, uint16_t *len)
{
    static uint8_t cmd_status = 0;
    static uint16_t current_cmd;
    static uint16_t current_item = 0;
    *len = 0;

    if(frame->cmd_id != current_cmd && cmd_status == 0)
    {
        current_cmd = frame->cmd_id;
        cmd_status = 1;
    }

    if(frame->cmd_id != current_cmd)
        return DECODE_ID_ERROR;

    for(uint8_t item = 0; item < can->can_dlc; item++)
    {
        *(data + current_item) = can->data[item];
        current_item += 1;
    }

    if(frame->cmd_last)
    {
        *len = current_item;
        current_item = 0;
        cmd_status = 0;
        return DECODE_END_OK;
    }
    return DECODE_NOT_END;
}

bool frame::can::response(MCP2515 &can_bus, sat_t *frame, uint8_t *data, uint16_t len) // команда-ответ
{
    if(len == 0)
        return 0;

    sat_t frame_to_send{
        .priority = frame->priority,
        .cmd_resp = 0x1,
        .cmd_id = frame->cmd_id,
        .cmd_last = 0x0,
        .dst = frame->src,
        .src = frame->dst};

    uint8_t chunks = len >> 3;
    if ((len & 7) != 0)
        chunks++;

    uint16_t current_item = 0;

    struct can_frame transmitter;

    for (uint8_t item = 0; item < chunks; item++)
    {
        if (len > 8)
        {
            transmitter.can_dlc = 8;
            len -= 8;
        }
        else
            transmitter.can_dlc = len;

        for (uint8_t item = 0; item < transmitter.can_dlc; item++)
        {
            transmitter.data[item] = *(data + current_item);
            current_item++;
        }
        if (item == chunks - 1)
            frame_to_send.cmd_last = 0x1;
        frame_to_send.cmd_chunks = item;

        make(&frame_to_send, &transmitter, EFF_FLAG);
        if (can_bus.sendMessage(&transmitter) != MCP2515::ERROR_OK)
            return 1;

        busy_wait_ms(1); //! Fuck oh no!
    }
    return 0;
}

bool frame::can::request(MCP2515 &can_bus, sat_t *frame, uint8_t use_29_bit_id)
{
    struct can_frame transmitter;
    transmitter.can_dlc = 0;

    make(frame, &transmitter, (use_29_bit_id ? (EFF_FLAG | RTR_FLAG) : RTR_FLAG));
    if (can_bus.sendMessage(&transmitter) != MCP2515::ERROR_OK)
        return 1;

    return 0;
}

#if !USE_CAN_STANDARD
bool frame::can::request(MCP2515 &can_bus, sat_t *frame, struct can_frame *can, uint8_t use_29_bit_id)
{
    make(frame, can, (use_29_bit_id ? (EFF_FLAG | NULL_FLAG) : NULL_FLAG));
    if (can_bus.sendMessage(can) != MCP2515::ERROR_OK)
        return 1;

    return 0;
}
#endif

frame::read_ex_t frame::can::read(MCP2515 &can_bus, sat_t *frame, struct can_frame *can, const uint8_t module_id)
{
    if (can_bus.readMessage(can) != MCP2515::ERROR_OK)
        return READ_ERROR;

    return calculate(can, frame, module_id);
}