//doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"
#include "i_video.h"

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

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char convertToDoomKey(unsigned int key){
  key = KEY_ENTER;
  return key;
  /*
  switch (key)
    {
    case SDLK_RETURN:
      key = KEY_ENTER;
      break;
    case SDLK_ESCAPE:
      key = KEY_ESCAPE;
      break;
    case SDLK_LEFT:
      key = KEY_LEFTARROW;
      break;
    case SDLK_RIGHT:
      key = KEY_RIGHTARROW;
      break;
    case SDLK_UP:
      key = KEY_UPARROW;
      break;
    case SDLK_DOWN:
      key = KEY_DOWNARROW;
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      key = KEY_FIRE;
      break;
    case SDLK_SPACE:
      key = KEY_USE;
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      key = KEY_RSHIFT;
      break;
    case SDLK_LALT:
    case SDLK_RALT:
      key = KEY_LALT;
      break;
    case SDLK_F2:
      key = KEY_F2;
      break;
    case SDLK_F3:
      key = KEY_F3;
      break;
    case SDLK_F4:
      key = KEY_F4;
      break;
    case SDLK_F5:
      key = KEY_F5;
      break;
    case SDLK_F6:
      key = KEY_F6;
      break;
    case SDLK_F7:
      key = KEY_F7;
      break;
    case SDLK_F8:
      key = KEY_F8;
      break;
    case SDLK_F9:
      key = KEY_F9;
      break;
    case SDLK_F10:
      key = KEY_F10;
      break;
    case SDLK_F11:
      key = KEY_F11;
      break;
    case SDLK_EQUALS:
    case SDLK_PLUS:
      key = KEY_EQUALS;
      break;
    case SDLK_MINUS:
      key = KEY_MINUS;
      break;
    default:
      key = tolower(key);
      break;
    }

  return key;
  */
}

static void addKeyToQueue(int pressed, unsigned int keyCode){
  unsigned char key = convertToDoomKey(keyCode);

  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

static void handleKeyInput(){
  /*
  SDL_Event e;
  while (SDL_PollEvent(&e)){
    if (e.type == SDL_QUIT){
      puts("Quit requested");
      atexit(SDL_Quit);
      exit(1);
    }
    if (e.type == SDL_KEYDOWN) {
      //KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
      //printf("KeyPress:%d sym:%d\n", e.xkey.keycode, sym);
      addKeyToQueue(1, e.key.keysym.sym);
    } else if (e.type == SDL_KEYUP) {
      //KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
      //printf("KeyRelease:%d sym:%d\n", e.xkey.keycode, sym);
      addKeyToQueue(0, e.key.keysym.sym);
    }
  }
  */
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

    r_v = present(bgai_hnd, PRESENT_COLORMAP|PRESENT_PIXBUF|CLEAR_FRAME_INTERRUPT, NULL, 0);
    CHECK_ERROR(r_v, "failed to present framebuffer");
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
  /*
  SDL_UpdateTexture(texture, NULL, DG_ScreenBuffer, DOOMGENERIC_RESX*sizeof(uint32_t));

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  handleKeyInput();
  */
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
  /*
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }
  */

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