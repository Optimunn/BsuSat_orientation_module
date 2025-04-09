#ifndef SAT_FRAME_HPP__
#define SAT_FRAME_HPP__

#include "pico/stdlib.h"
#include "mcp2515.h"
#include "can.h"

#define USE_CAN_STANDARD false

namespace frame
{
    //* can id structure
    typedef struct sat_s
    {
        uint32_t sad_flag : 3;
        uint32_t priority : 2;
        //* cmd ->
        uint32_t cmd_resp : 1;
        uint32_t cmd_id : 9;
        uint32_t cmd_last : 1;
        uint32_t cmd_chunks : 8;
        //* cmd <-
        uint32_t dst : 4; // адрес получателя
        uint32_t src : 4; // адрес отправителя
    } sat_t;

    //* frame types ->
    typedef enum type_e : uint8_t
    {
        EFF_FLAG = 0x4,
        RTR_FLAG = 0x2,
        ERR_FLAG = 0x1,
        NULL_FLAG = 0x0
    } type_t;

    typedef uint8_t type_u_t;
    //* frame type <-

    //* can read errors
    typedef enum read_ex_e : uint8_t
    {
        REQUEST_OK = 0,
        RESPONSE_OK = 1,
        READ_ERROR = 2,
        MODULE_ERROR = 3
    } read_ex_t;

    //* decode messages errors
    typedef enum decode_e : uint8_t
    {
        DECODE_END_OK = 0,
        DECODE_NOT_END = 1,
        DECODE_ID_ERROR = 2,
    } decode_t;

    void make(sat_t *frame, struct can_frame *can, type_u_t type);
    void parse(sat_t *frame, struct can_frame *can);
    inline bool check(struct can_frame *can, const uint8_t module_id);
    read_ex_t calculate(struct can_frame *can, sat_t *frame, const uint8_t module_id);
    decode_t decode(sat_t *frame, struct can_frame *can, uint8_t *data, uint16_t *len);

    namespace can
    {
        bool response(MCP2515 &can_bus, sat_t *frame, uint8_t *data, uint16_t len); // команда-ответ
        bool request(MCP2515 &can_bus, sat_t *frame, uint8_t use_29_bit_id);
    #if !USE_CAN_STANDARD 
        bool request(MCP2515 &can_bus, sat_t *frame, struct can_frame *can, uint8_t use_29_bit_id);
    #endif
        read_ex_t read(MCP2515 &can_bus, sat_t *frame, struct can_frame *can, const uint8_t module_id);
    }
}

#endif