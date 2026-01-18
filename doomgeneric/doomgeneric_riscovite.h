#pragma once

#include <stdint.h>
#include <riscovite.h>

// BGAI claim interface
#define SYS_CLAIM_CONSOLE(slot) (~((slot) | (0x00000001 << 4)))

// Console mux interface
#define SYS_SELECT_RESOURCES(slot) (~((slot) | (0x00000001 << 4)))

// Console BGAI interface
#define SYS_CREATE_GRAPHICS_FRAMEBUFFER(slot) (~((slot) | (0x00000011 << 4)))
#define SYS_CREATE_AUDIO_OUTPUT(slot) (~((slot) | (0x00000021 << 4)))
#define SYS_CREATE_INPUT_CONTEXT(slot) (~((slot) | (0x00000031 << 4)))

// BGAI graphics interface
#define SYS_CREATE_SOURCE_BUFFER(slot) (~((slot) | (0x00000f07 << 4)))
#define SYS_UPDATE(slot) (~((slot) | (0x00000001 << 4)))
#define SYS_SET_SOURCE_BUFFERS(slot) (~((slot) | (0x00000002 << 4)))
#define SYS_ENABLE_GRAPHICS_EVENTS(slot) (~((slot) | (0x00000003 << 4)))
#define SYS_DISABLE_GRAPHICS_EVENTS(slot) (~((slot) | (0x00000004 << 4)))
#define SYS_DISMISS_GRAPHICS_EVENTS(slot) (~((slot) | (0x00000005 << 4)))
#define EVT_GRAPHICS_FRAME_INTERRUPT(slot) (~((slot) | (0x00000001 << 4)))
#define GRAPHICS_PRESENT ((uint64_t)(1UL << 63))
#define GRAPHICS_CLEAR_FRAME_INTERRUPT ((uint64_t)(1UL << 62))

// BGAI audio interface
#define SYS_REQ_AUDIO_BUFFER_INTERRUPT(slot) (~((slot) | (0x00000f01 << 4)))
#define SYS_ENABLE_AUDIO_INTERRUPTS(slot) (~((slot) | (0x00000002 << 4)))

// BGAI input interface
#define SYS_GET_REPORT(slot) (~((slot) | (0x00000001 << 4)))
#define SYS_GET_REPORT_INLINE(slot) (~((slot) | (0x00000002 << 4)))
#define SYS_REQUEST_STRING_INPUT(slot) (~((slot) | (0x00000003 << 4)))
#define SYS_SET_MOUSE_MODE(slot) (~((slot) | (0x00000004 << 4)))
#define SYS_SET_TEXT_INPUT_HANDLER(slot) (~((slot) | (0x000000f1 << 4)))
#define SYS_SET_BUTTON_INPUT_HANDLER(slot) (~((slot) | (0x000000f2 << 4)))
#define SYS_SET_ACCESS_TOOL_ACTION_HANDLER(slot) (~((slot) | (0x000000f3 << 4)))
#define SYS_SET_MOUSE_MOVEMENT_HANDLER(slot) (~((slot) | (0x000000f4 << 4)))
#define SYS_SET_POINTER_MOVEMENT_HANDLER(slot) (~((slot) | (0x000000f5 << 4)))
#define SYS_SET_GAMEPAD_CONNECTION_HANDLER(slot) (~((slot) | (0x000000f6 << 4)))
#define SYS_SET_REPORT_DESCRIPTOR(slot) (~((slot) | (0x00000f01 << 4)))
#define SYS_SET_TEXT_INPUT_BUFFER(slot) (~((slot) | (0x00000f02 << 4)))
#define SYS_SET_BUTTON_INPUT_BUFFER(slot) (~((slot) | (0x00000f03 << 4)))
#define SYS_SET_ACCESS_TOOL_ACTION_BUFFER(slot) (~((slot) | (0x00000f04 << 4)))
#define INPUT_REPORT_CONSUME (0b1)

// BGAI input report field ids
#define INPUT_REPORT_BUTTON_INPUT_BUF 0x01
#define INPUT_REPORT_ABS_POINTER_POS 0x05
#define INPUT_REPORT_POINTER_BUTTONS 0x06
#define INPUT_REPORT_TIMESTAMP 0x07

#define UINT128(hi, lo) (((__uint128_t)(hi)) << 64 | (lo))
#define CHECK_ERROR(result, msg_prefix)                                        \
  ({                                                                           \
    if ((result).error != 0) {                                                 \
      fprintf(stderr, "%s: %s\n", (msg_prefix), strerror((result).error));     \
      riscovite_exit(1);                                                       \
    }                                                                          \
    0;                                                                         \
  })
#define CHECK_RESULT(result, msg_prefix)                                       \
  ({                                                                           \
    if ((result).error != 0) {                                                 \
      fprintf(stderr, "%s: %s\n", (msg_prefix), strerror((result).error));     \
      riscovite_exit(1);                                                       \
    }                                                                          \
    (result).value;                                                            \
  })

static inline struct riscovite_result_uint64 claim_console(uint64_t hnd,
                                                       uint64_t flags) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _flags asm("a1") = (uint64_t)flags;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_flags), [FUNCNUM] "n"(SYS_CLAIM_CONSOLE(0))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

struct framebuffer_desc {
  uint16_t width;
  uint16_t height;
};
struct source_buffer_desc {
  uint16_t row_pitch;
  uint8_t *buf;
};
struct graphics_xfer_req {
  uint16_t width;
  uint16_t height;
  uint16_t flags;
  uint16_t stride;
  uint32_t source;
  int16_t dst_x;
  int16_t dst_y;
};

static inline struct riscovite_result_uint64
open_framebuffer(uint64_t hnd, uint16_t width, uint16_t height,
                 uint16_t pixel_fmt, struct framebuffer_desc *result) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _width asm("a1") = (uint64_t)width;
  register uint64_t _height asm("a2") = (uint64_t)height;
  register uint64_t _pixel_fmt asm("a3") = (uint64_t)pixel_fmt;
  register uint64_t _flags asm("a4") = 0;
  register uint64_t _result asm("a5") = (uint64_t)result;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_width), "r"(_height), "r"(_flags), "r"(_pixel_fmt),
                        "r"(_result), [FUNCNUM] "n"(SYS_CREATE_GRAPHICS_FRAMEBUFFER(1))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_uint64
create_graphics_source_buffer(uint64_t hnd, uint64_t size) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _size asm("a1") = (uint64_t)size;
  register uint64_t _flags asm("a2") = 0;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_size), "r"(_flags),
                        [FUNCNUM] "n"(SYS_CREATE_SOURCE_BUFFER(0))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_uint64
open_audio_output(uint64_t hnd) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), [FUNCNUM] "n"(SYS_CREATE_AUDIO_OUTPUT(1))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_uint64
open_input(uint64_t hnd) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), [FUNCNUM] "n"(SYS_CREATE_INPUT_CONTEXT(1))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_void
update(uint64_t hnd, uint64_t flags, uint64_t *transfers, uint64_t transfer_count) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _flags asm("a1") = (uint64_t)flags;
  register uint64_t _transfers asm("a2") = (uint64_t)transfers;
  register uint64_t _transfer_count asm("a3") = (uint64_t)transfer_count;
  register uint64_t _access_tree_update asm("a4") = 0;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_flags), "r"(_transfers),
                        "r"(_transfer_count), "r"(_access_tree_update), [FUNCNUM] "n"(SYS_UPDATE(0))
                      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_void enable_graphics_interrupts(uint64_t hnd, uint16_t mask) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _mask asm("a1") = (uint64_t)mask;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd),
                        "r1"(_mask), [FUNCNUM] "n"(SYS_ENABLE_GRAPHICS_EVENTS(0))
                      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_uint64
set_input_report_descriptor(uint64_t hnd, uint64_t desc_id,
                            const uint16_t *addr, uint64_t field_count) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _desc_id asm("a1") = (uint64_t)desc_id;
  register uint64_t _addr asm("a2") = (uint64_t)addr;
  register uint64_t _field_count asm("a3") = (uint64_t)field_count;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline(
      "li a7, %[FUNCNUM]\n"
      "ecall\n"
      : [RET] "=r"(__ret), [ERR] "=r"(__err)
      : "r0"(_hnd), "r1"(_desc_id), "r"(_addr),
        "r"(_field_count), [FUNCNUM] "n"(SYS_SET_REPORT_DESCRIPTOR(0))
      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_uint64
set_button_input_buffer(uint64_t hnd, uint8_t size) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _size asm("a1") = (uint64_t)size;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline(
      "li a7, %[FUNCNUM]\n"
      "ecall\n"
      : [RET] "=r"(__ret), [ERR] "=r"(__err)
      : "r0"(_hnd), "r1"(_size), [FUNCNUM] "n"(SYS_SET_BUTTON_INPUT_BUFFER(0))
      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_uint64
get_input_report(uint64_t hnd, uint64_t desc_id, void *into, uint64_t flags) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _desc_id asm("a1") = (uint64_t)desc_id;
  register uint64_t _into asm("a2") = (uint64_t)into;
  register uint64_t _flags asm("a3") = (uint64_t)flags;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_desc_id), "r"(_into),
                        "r"(_flags), [FUNCNUM] "n"(SYS_GET_REPORT(0))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

static inline struct riscovite_result_void
request_sound_interrupt(uint64_t hnd, uint16_t threshold, void *handler_addr, uint64_t flags, uint64_t user_data) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _threshold asm("a1") = (uint64_t)threshold;
  register uint64_t _handler_addr asm("a2") = (uint64_t)handler_addr;
  register uint64_t _flags asm("a3") = (uint64_t)flags;
  register uint64_t _user_data asm("a4") = (uint64_t)user_data;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline(
      "li a7, %[FUNCNUM]\n"
      "ecall\n"
      : [RET] "=r"(__ret), [ERR] "=r"(__err)
      : "r0"(_hnd), "r1"(_threshold), "r"(_handler_addr), "r"(_flags),
        "r"(_user_data), [FUNCNUM] "n"(SYS_REQ_AUDIO_BUFFER_INTERRUPT(1))
      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_void enable_audio_interrupts(uint64_t hnd, uint16_t mask) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _mask asm("a1") = (uint64_t)mask;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd),
                        "r1"(_mask), [FUNCNUM] "n"(SYS_ENABLE_AUDIO_INTERRUPTS(1))
                      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

struct riscovite_button_event {
  uint16_t button;
  uint16_t flags;
  uint16_t pointer_x;
  uint16_t pointer_y;
  uint8_t text_start;
  uint8_t text_end;
  uint8_t padding[2];
};
