#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error I need a cross compiler
#endif

#if !defined(__i386__)
#error I need compiler that targeting 32-bit x86 architectures
#endif

typedef enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_YELLOW = 14,
  VGA_COLOR_WHITE = 15,
} vga_color_t;

void*
memcpy (void* destination, void const* source, size_t count)
{
  uint8_t* dst = (uint8_t*) destination;
  uint8_t const* src = (uint8_t const*) source;
  for (; count != 0; --count)
  {
    *(dst++) = *(src++);
  }
  return destination;
}

void*
memmove (void* destination, void const* source, size_t count)
{
  uint8_t* dst = (uint8_t*) destination;
  uint8_t const* src = (uint8_t const*) source;
  dst += count;
  src += count;
  for (; count != 0; --count)
  {
    *(dst--) = *(src--);
  }
  return destination;
}

void*
xmemcpy (void* destination, void const* source, size_t count)
{
  unsigned long* wdst       = (unsigned long*) destination;
  unsigned long const* wsrc = (unsigned long const*) source;
  uint8_t* cdst;
  uint8_t const* csrc;
  int i;
  for (i = count / sizeof(unsigned long); i != 0; --i)
  {
    *(wdst++) = *(wsrc++);
  }
  cdst = (uint8_t*) wdst;
  csrc = (uint8_t const*) wsrc;
  for (i = count % sizeof(unsigned long); i != 0; --i)
  {
    *(cdst++) = *(csrc++);
  }
  return destination;
}

void*
xmemmove (void* destination, void const* source, size_t count)
{
  unsigned long* wdst       = (unsigned long*) (((uint8_t*) destination) + count);
  unsigned long const* wsrc = (unsigned long const*) (((uint8_t*) source) + count);
  uint8_t* cdst;
  uint8_t const* csrc;
  int i;
  for (i = count / sizeof(unsigned long); i != 0; --i)
  {
    *(wdst--) = *(wsrc--);
  }
  cdst = (uint8_t*) wdst;
  csrc = (uint8_t const*) wsrc;
  for (i = count % sizeof(unsigned long); i != 0; --i)
  {
    *(cdst--) = *(csrc--);
  }
  return destination;
}

uint8_t
vga_entry_color (vga_color_t fg, vga_color_t bg)
{
  return fg | bg << 4;
}

uint16_t
vga_entry (uint8_t ch, uint8_t color)
{
  return (uint16_t) ch | (uint16_t) color << 8;
}

size_t
strlen (char const* str)
{
  size_t result = 0;
  while (str[result])
  {
    ++result;
  }
  return result;
}

size_t const VGA_WIDTH  = 80;
size_t const VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void
clrscr (void)
{
  size_t x, y;
  for (y = 0; y < VGA_HEIGHT; ++y)
  {
    for (x = 0; x < VGA_WIDTH; ++x)
    {
      size_t const index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void
initialize (void)
{
  terminal_row    = 0;
  terminal_column = 0;
  terminal_color  = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  terminal_buffer = (uint16_t*) 0xB8000;
  clrscr();
}

void
textbackground (vga_color_t new_color)
{
  terminal_color = (terminal_color & 0xF0) | new_color;
}

void
textcolor (vga_color_t new_color)
{
  terminal_color = (terminal_color & 0x0F) | new_color << 4;
}

void
gotoxy (int x, int y)
{
  terminal_row = y;
  terminal_column = x;
}

void
terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y)
{
  if (c == '\a' ||
      c == '\b' ||
      c == '\f' ||
      c == '\n' ||
      c == '\r')
  {
    return;
  }
  size_t const index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void
terminal_scroll (void)
{
  xmemmove(terminal_buffer,
           terminal_buffer + VGA_WIDTH,
           VGA_WIDTH * (VGA_HEIGHT - 1) * sizeof(uint16_t));
  size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH;
  size_t x;
  for (x = 0; x < VGA_WIDTH; ++x)
  {
    terminal_buffer[index + x] = vga_entry(' ', terminal_color);
  }
  --terminal_row;
}

void
putchar (char c)
{
  terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
  if (++terminal_column == VGA_WIDTH)
  {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT)
    {
      terminal_scroll();
    }
  }
  switch (c)
  {
  case '\b':
    if (terminal_column > 1)
    {
      terminal_column -= 2;
    }
    break;
  case '\n':
    terminal_column = 0;
    ++terminal_row;
    break;
  case '\f':
    clrscr();
    break;
  case '\r':
    terminal_column = 0;
    break;
  }
}

void
terminal_write (char const* data, size_t count)
{
  for (size_t i = 0; i < count; ++i)
  {
    putchar(data[i]);
  }
}

void
puts (char const* str)
{
  terminal_write(str, strlen(str));
}

inline
void
disable_interrupts (void)
{
  asm("cli");
}

inline
void
enable_interrupts (void)
{
  asm("sti");
}

inline
void
halt (void)
{
  asm("hlt");
}

void
kmain (void)
{
  initialize();
  textbackground(VGA_COLOR_LIGHT_BLUE);
  textcolor(VGA_COLOR_LIGHT_BLUE);
  clrscr();
  textbackground(VGA_COLOR_YELLOW);
  textcolor(VGA_COLOR_YELLOW);
  gotoxy(0, VGA_HEIGHT / 2);
  size_t row = terminal_row;
  for (; row != 0; --row)
  {
    size_t col = VGA_WIDTH;
    terminal_column = 0;
    for (; col != 0; --col)
    {
      putchar(' ');
    }
  }
  gotoxy(0, VGA_HEIGHT - 1);
  textbackground(VGA_COLOR_LIGHT_BLUE);
  textcolor(VGA_COLOR_BLACK);
  puts("| Slava Ukraine | Glory to Ukraine | russkiy korabl idi nahui |  UKRAINIAN OS |");
  disable_interrupts();
  for (;;)
  {
    halt();
  }
}