//doomgeneric for RISCovite

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"
#include "i_video.h"
#include "i_riscovitesound.h"

#include <stdio.h>
#include <unistd.h>

#include <stdbool.h>
#include <riscovite.h>
#include <string.h>

#include "doomgeneric_riscovite.h"

#define KEYQUEUE_SIZE 16

static struct framebuffer_desc fb_desc;
static uint64_t bgai_hnd;
static bool advance_frame = false;

struct button_events_report {
  uint32_t count;
  struct riscovite_button_event events[32];
};

static const uint16_t input_report_desc[1] = {INPUT_REPORT_BUTTON_INPUT_BUF};
static struct button_events_report button_events;
static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char convertToDoomKey(uint16_t key){
  if ((key >> 8) != 0x07) {
    return 0; // not a keyboard button
  }
  key = key & 0xff;

  if (key >= 0x04 && key <= 0x1d) {
    // Letter keys all get translated to their lowercase ASCII equivalents
    // assuming a QWERTY keyboard layout. Doom cares about the physical
    // positions of the keys rather than what they are labelled.
    return key - 0x04 + 'a';
  }
  if (key >= 0x1e && key <= 0x26) {
    // Digits 1-9
    return key - 0x1e + '1';
  }

  switch (key) {
  case 0x28:
      return KEY_ENTER;
  case 0x29:
      return KEY_ESCAPE;
  case 0x50:
      return KEY_LEFTARROW;
  case 0x4f:
      return KEY_RIGHTARROW;
  case 0x52:
      return KEY_UPARROW;
  case 0x51:
      return KEY_DOWNARROW;
  case 0xe0:
  case 0xe4:
      return KEY_FIRE;
  case 0x2c:
      return KEY_USE;
  case 0xe1:
  case 0xe5:
      return KEY_RSHIFT;
  case 0xe2:
  case 0xe6:
      return KEY_LALT;
  case 0x3a:
      return KEY_F1;
  case 0x3b:
      return KEY_F2;
  case 0x3c:
      return KEY_F3;
  case 0x3d:
      return KEY_F4;
  case 0x3e:
      return KEY_F5;
  case 0x3f:
      return KEY_F6;
  case 0x40:
      return KEY_F7;
  case 0x41:
      return KEY_F8;
  case 0x42:
      return KEY_F9;
  case 0x43:
      return KEY_F10;
  case 0x44:
      return KEY_F11;
  case 0x45:
      return KEY_F12;
  case 0x2e:
      return KEY_EQUALS;
  case 0x2d:
      return KEY_MINUS;
  case 0x27:
      return '0';
  case 0x2b:
      return KEY_TAB;
  case 0x2a:
      return KEY_BACKSPACE;
  case 0x48:
      return KEY_PAUSE;
  default:
      return 0;
  }
}

static void addKeyToQueue(int pressed, unsigned int keyCode){
  unsigned char key = convertToDoomKey(keyCode);
  if (key == 0) {
    return;
  }

  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

static void handleKeyInput(){
    struct riscovite_result_uint64 r_u64;
    r_u64 = get_input_report(bgai_hnd, 0, &button_events, INPUT_REPORT_CONSUME);
    CHECK_ERROR(r_u64, "failed to get input report");

    int count = button_events.count;
    struct riscovite_button_event *event = &button_events.events[0];
    for (int count = button_events.count; count != 0; count--) {
        int pressed = (event->flags & 0x100) != 0;
        addKeyToQueue(pressed, event->button);
        event++;
    }
}

void DG_Init(){
    struct riscovite_result_void r_v;
    struct riscovite_result_uint64 r_u64;

    r_u64 = riscovite_dup(RISCOVITE_HND_STDOUT);
    uint64_t hnd = CHECK_RESULT(r_u64, "failed to dup stdout");

    r_v = riscovite_select_interface(
        hnd, UINT128(0x2b4c7062cfae565e, 0xa80dd0e14e44dc3a), 0);
    CHECK_ERROR(r_v, "failed to select BGAI claim interface");

    r_u64 = open_bgai(hnd, 0b111);
    hnd = CHECK_RESULT(r_u64, "failed to open BGAI object");

    r_v = riscovite_select_interface(
        hnd, UINT128(0x68d60363c1f353e0, 0xa0b4ec596dd8a689), 0);
    CHECK_ERROR(r_v, "failed to select BGAI graphics interface");

    r_v = riscovite_select_interface(
        hnd, UINT128(0x9a360746043f5a68, 0x8fe7bd9bd407a4fb), 1);
    CHECK_ERROR(r_v, "failed to select BGAI audio interface");

    r_v = riscovite_select_interface(
        hnd, UINT128(0x7eeebf1448835940, 0x907d57f507530d69), 2);
    CHECK_ERROR(r_v, "failed to select BGAI input interface");

    // TODO: Also setup input report descriptor

    r_v = open_framebuffer(hnd, DOOMGENERIC_RESX, DOOMGENERIC_RESY, 2, &fb_desc);
    CHECK_ERROR(r_v, "failed to open framebuffer");

    // TEMP: Some debug information about the framebuffer
    printf("framebuffer:\n");
    printf("  address:     %p\n", fb_desc.buf);
    printf("  width:       %d\n", (int)fb_desc.width);
    printf("  height:      %d\n", (int)fb_desc.height);
    printf("  row_pitch:   %ld\n", (long int)fb_desc.row_pitch);
    printf("  pixel_pitch: %d\n", (int)fb_desc.pixel_pitch);

    bgai_hnd = hnd;
    riscovite_sound_handle = hnd; // used by i_riscovitesound.c and i_riscovitemusic.c

    r_v = present(bgai_hnd, PRESENT_COLORMAP|PRESENT_PIXBUF|CLEAR_FRAME_INTERRUPT, NULL, 0);
    CHECK_ERROR(r_v, "failed to present framebuffer");

    // We also need an input report descriptor so we can read the contents of
    // the button input buffer later.
    r_u64 = set_button_input_buffer(bgai_hnd, sizeof(button_events.events) / sizeof(button_events.events[0]));
    CHECK_ERROR(r_u64, "failed to set button input buffer size");
    r_u64 = set_input_report_descriptor(bgai_hnd, 0, &input_report_desc[0], sizeof(input_report_desc) / sizeof(input_report_desc[0]));
    CHECK_ERROR(r_u64, "failed to set input report descriptor");

    r_v = request_sound_interrupt(bgai_hnd, 3072, riscovite_sound_interrupt_handler, 10, 0);
    CHECK_ERROR(r_v, "failed to request sound buffer interrupt");
    r_v = enable_audio_interrupts(bgai_hnd, 0b1);
    CHECK_ERROR(r_v, "failed to enable audio interrupts");
}

void DG_DrawFrame()
{
    struct riscovite_result_void r_v;

    uint8_t *src = DG_ScreenBuffer;
    uint8_t *dst = fb_desc.buf;
    int src_stride = DOOMGENERIC_RESX;
    int dst_stride = fb_desc.row_pitch;
    for (int y = 0; y < DOOMGENERIC_RESY; y++) {
        //printf("memcpy(%p, %p, %d)\n", dst, src, DOOMGENERIC_RESX);
        memcpy(dst, src, DOOMGENERIC_RESX);
        src += src_stride;
        dst += dst_stride;
    }
    if (palette_changed) {
      uint8_t colormap[3 * 256];
      uint8_t *color_dst = &colormap[0];
      struct color *color_src = &colors[0];
      for (int i = 0; i < (sizeof(colors) / sizeof(colors[0])); i++) {
        color_dst[0] = color_src->b;
        color_dst[1] = color_src->g;
        color_dst[2] = color_src->r;
        color_dst += 3;
        color_src += 1;
      }
      r_v = write_colormap(bgai_hnd, &colormap[0], 256, 0);
      CHECK_ERROR(r_v, "failed to update colormap");
      r_v = present(bgai_hnd, PRESENT_COLORMAP|PRESENT_PIXBUF|CLEAR_FRAME_INTERRUPT, NULL, 0);
      CHECK_ERROR(r_v, "failed to present framebuffer");
    } else {
      r_v = present(bgai_hnd, PRESENT_PIXBUF|CLEAR_FRAME_INTERRUPT, NULL, 0);
      CHECK_ERROR(r_v, "failed to present framebuffer");
    }
    handleKeyInput();
}

void DG_SleepMs(uint32_t ms)
{
  /*
  SDL_Delay(ms);
  */
}

uint32_t DG_GetTicksMs()
{
  struct riscovite_result_uint64 r_u64 = riscovite_get_current_timestamp();
  CHECK_ERROR(r_u64, "failed to get current timestamp");

  // The system timestamp is in nanoseconds, but the caller wants milliseconds.
  return r_u64.value / 1000000;
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
        //key queue is empty
        return 0;
    } else {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

        *pressed = keyData >> 8;
        *doomKey = keyData & 0xFF;

        return 1;
    }

    return 0;
}

void DG_SetWindowTitle(const char * title)
{
  /*
  if (window != NULL){
    SDL_SetWindowTitle(window, title);
  }
  */
}

int main(int argc, char **argv)
{
    printf("DOOM for RISCovite\n");
    doomgeneric_Create(argc, argv);

    for (;;) {
        // FIXME: It would be better to use the frame interrupt to
        // let us sleep between frames, but we'll just spin as fast
        // as possible for now since that's how the other doomgeneric
        // ports are written.
        doomgeneric_Tick();
    }

    return 0;
}