#pragma once

#include <stdint.h>
#include <riscovite.h>

// BGAI claim interface
#define SYS_OPEN(slot) (~((slot) | (0x00000001 << 4)))

// BGAI graphics interface
#define SYS_OPEN_FRAMEBUFFER(slot) (~((slot) | (0x00000f01 << 4)))
#define SYS_PRESENT(slot) (~((slot) | (0x00000001 << 4)))
#define SYS_WRITE_COLORMAP(slot) (~((slot) | (0x00000002 << 4)))
#define SYS_REQ_FRAME_INTERRUPT(slot) (~((slot) | (0x00000f05 << 4)))
#define SYS_ENABLE_INTERRUPTS(slot) (~((slot) | (0x00000003 << 4)))
#define PRESENT_PIXBUF (0b0001)
#define PRESENT_COLORMAP (0b0010)
#define CLEAR_FRAME_INTERRUPT (0b1000)

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

// BGAI input report field ids
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

static inline struct riscovite_result_uint64 open_bgai(uint64_t hnd,
                                                       uint64_t flags) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _flags asm("a1") = (uint64_t)flags;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_flags), [FUNCNUM] "n"(SYS_OPEN(0))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}

struct framebuffer_desc {
  uint8_t *buf;
  uint64_t row_pitch;
  uint16_t width;
  uint16_t height;
  uint8_t pixel_pitch;
  uint8_t _reserved[3];
};

static inline struct riscovite_result_void
open_framebuffer(uint64_t hnd, uint16_t width, uint16_t height,
                 uint16_t pixel_fmt, struct framebuffer_desc *result) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _width asm("a1") = (uint64_t)width;
  register uint64_t _height asm("a2") = (uint64_t)height;
  register uint64_t _pixel_fmt asm("a3") = (uint64_t)pixel_fmt;
  register uint64_t _result asm("a4") = (uint64_t)result;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_width), "r"(_height), "r"(_pixel_fmt),
                        "r"(_result), [FUNCNUM] "n"(SYS_OPEN_FRAMEBUFFER(0))
                      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_void
present(uint64_t hnd, uint64_t flags, uint64_t *rects, uint64_t rect_count) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _flags asm("a1") = (uint64_t)flags;
  register uint64_t _rects asm("a2") = (uint64_t)rects;
  register uint64_t _rect_count asm("a3") = (uint64_t)rect_count;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_flags), "r"(_rects),
                        "r"(_rect_count), [FUNCNUM] "n"(SYS_PRESENT(0))
                      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_void
write_colormap(uint64_t hnd, uint8_t *colors, uint64_t count, uint64_t offset) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _colors asm("a1") = (uint64_t)colors;
  register uint64_t _count asm("a2") = (uint64_t)count;
  register uint64_t _offset asm("a3") = (uint64_t)offset;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd), "r1"(_colors), "r"(_count),
                        "r"(_offset), [FUNCNUM] "n"(SYS_WRITE_COLORMAP(0))
                      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_void
request_frame_interrupt(uint64_t hnd, void *handler_addr, uint64_t flags,
                        uint64_t user_data) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _handler_addr asm("a1") = (uint64_t)handler_addr;
  register uint64_t _flags asm("a2") = (uint64_t)flags;
  register uint64_t _user_data asm("a3") = (uint64_t)user_data;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline(
      "li a7, %[FUNCNUM]\n"
      "ecall\n"
      : [RET] "=r"(__ret), [ERR] "=r"(__err)
      : "r0"(_hnd), "r1"(_handler_addr), "r"(_flags),
        "r"(_user_data), [FUNCNUM] "n"(SYS_REQ_FRAME_INTERRUPT(0))
      : "a7", "memory");
  return ((struct riscovite_result_void){__ret, __err});
}

static inline struct riscovite_result_void enable_interrupts(uint64_t hnd,
                                                             uint16_t mask) {
  register uint64_t _hnd asm("a0") = hnd;
  register uint64_t _mask asm("a1") = (uint64_t)mask;
  register uint64_t __ret asm("a0");
  register uint64_t __err asm("a1");
  asm volatile inline("li a7, %[FUNCNUM]\n"
                      "ecall\n"
                      : [RET] "=r"(__ret), [ERR] "=r"(__err)
                      : "r0"(_hnd),
                        "r1"(_mask), [FUNCNUM] "n"(SYS_ENABLE_INTERRUPTS(0))
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
        "r"(_field_count), [FUNCNUM] "n"(SYS_SET_REPORT_DESCRIPTOR(2))
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
                        "r"(_flags), [FUNCNUM] "n"(SYS_GET_REPORT(2))
                      : "a7", "memory");
  return ((struct riscovite_result_uint64){__ret, __err});
}
