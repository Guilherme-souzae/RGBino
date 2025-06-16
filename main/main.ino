#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define BRC ((F_CPU / 16 / BAUD) - 1)

#define PIN_RED_LED (1 << PD3)
#define PIN_GREEN_LED (1 << PD5)
#define PIN_BLUE_LED (1 << PD6)
#define TOGGLE_BUTTON (1 << PD2)

#define PWM_RED OCR2B
#define PWM_GREEN OCR0B
#define PWM_BLUE OCR0A

uint8_t curr_red = 255, curr_green = 0, curr_blue = 0;
bool leds_on = true;

// ---------------- UART Functions ----------------

void uart_init() {
  UBRR0H = (BRC >> 8);
  UBRR0L = BRC;
  UCSR0B = (1 << TXEN0) | (1 << RXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
}

void uart_send(char c) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
}

void uart_print(const char *str) {
  while (*str) {
    uart_send(*str++);
  }
}

char uart_receive() {
  while (!(UCSR0A & (1 << RXC0)));
  return UDR0;
}

void uart_read_line(char *buffer, uint8_t max_len) {
  uint8_t i = 0;
  char c;
  while (1) {
    c = uart_receive();
    uart_send(c); // echo
    if (c == '\r' || c == '\n') break;
    if (i < max_len - 1) buffer[i++] = c;
  }
  buffer[i] = '\0';
  uart_print("\r\n");
}

// ---------------- LED and Color Functions ----------------

ISR(INT0_vect) {
  if (leds_on) {
    PWM_RED = 0;
    PWM_GREEN = 0;
    PWM_BLUE = 0;
    leds_on = false;
  } else {
    PWM_RED = curr_red;
    PWM_GREEN = curr_green;
    PWM_BLUE = curr_blue;
    leds_on = true;
  }
}

void set_colors_rgb(int r, int g, int b) {
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;

  curr_red = r;
  curr_green = g;
  curr_blue = b;
  PWM_RED = r;
  PWM_GREEN = g;
  PWM_BLUE = b;
}

void set_colors_hsl(int h, int s, int l) {
  if (h < 0 || h > 360 || s < 0 || s > 100 || l < 0 || l > 100) return;

  float H = (float)h;
  float S = s / 100.0;
  float L = l / 100.0;

  float C = (1 - fabs(2 * L - 1)) * S;
  float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
  float m = L - C / 2;

  float r, g, b;

  if (H < 60) {
    r = C; g = X; b = 0;
  } else if (H < 120) {
    r = X; g = C; b = 0;
  } else if (H < 180) {
    r = 0; g = C; b = X;
  } else if (H < 240) {
    r = 0; g = X; b = C;
  } else if (H < 300) {
    r = X; g = 0; b = C;
  } else {
    r = C; g = 0; b = X;
  }

  set_colors_rgb((r + m) * 255, (g + m) * 255, (b + m) * 255);
}

void set_colors_hsv(int h, int s, int v) {
  if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) return;

  float H = (float)h;
  float S = s / 100.0;
  float V = v / 100.0;

  float C = V * S;
  float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
  float m = V - C;

  float r, g, b;

  if (H < 60) {
    r = C; g = X; b = 0;
  } else if (H < 120) {
    r = X; g = C; b = 0;
  } else if (H < 180) {
    r = 0; g = C; b = X;
  } else if (H < 240) {
    r = 0; g = X; b = C;
  } else if (H < 300) {
    r = X; g = 0; b = C;
  } else {
    r = C; g = 0; b = X;
  }

  set_colors_rgb((r + m) * 255, (g + m) * 255, (b + m) * 255);
}

// ---------------- Main ----------------

int main(void) {
  // LED Pins
  DDRD |= PIN_RED_LED | PIN_GREEN_LED | PIN_BLUE_LED;

  // INT0 - botão
  EICRA |= (1 << ISC01) | (1 << ISC00);  // INT0 na borda de subida
  EIMSK |= (1 << INT0);
  sei();

  // PWM Timer 0 (verde e azul)
  TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1) | (1 << COM0B1);
  TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64

  // PWM Timer 2 (vermelho)
  TCCR2A = (1 << WGM20) | (1 << WGM21) | (1 << COM2B1);
  TCCR2B = (1 << CS22); // Prescaler 64

  uart_init();

  char buffer[32];

  // Valor inicial
  set_colors_rgb(curr_red, curr_green, curr_blue);

  while (1) {
    uart_print("\r\n--- MENU DE CORES ---\r\n");
    uart_print("Digite o modo (RGB / HSL / HSV): ");
    uart_read_line(buffer, sizeof(buffer));

    if (strcasecmp(buffer, "RGB") == 0) {
      uart_print("Digite R (0-255): ");
      uart_read_line(buffer, sizeof(buffer));
      int r = atoi(buffer);

      uart_print("Digite G (0-255): ");
      uart_read_line(buffer, sizeof(buffer));
      int g = atoi(buffer);

      uart_print("Digite B (0-255): ");
      uart_read_line(buffer, sizeof(buffer));
      int b = atoi(buffer);

      set_colors_rgb(r, g, b);
    } 
    else if (strcasecmp(buffer, "HSL") == 0) {
      uart_print("Digite H (0-360): ");
      uart_read_line(buffer, sizeof(buffer));
      int h = atoi(buffer);

      uart_print("Digite S (0-100): ");
      uart_read_line(buffer, sizeof(buffer));
      int s = atoi(buffer);

      uart_print("Digite L (0-100): ");
      uart_read_line(buffer, sizeof(buffer));
      int l = atoi(buffer);

      set_colors_hsl(h, s, l);
    } 
    else if (strcasecmp(buffer, "HSV") == 0) {
      uart_print("Digite H (0-360): ");
      uart_read_line(buffer, sizeof(buffer));
      int h = atoi(buffer);

      uart_print("Digite S (0-100): ");
      uart_read_line(buffer, sizeof(buffer));
      int s = atoi(buffer);

      uart_print("Digite V (0-100): ");
      uart_read_line(buffer, sizeof(buffer));
      int v = atoi(buffer);

      set_colors_hsv(h, s, v);
    } 
    else {
      uart_print("Modo invalido. Use RGB, HSL ou HSV.\r\n");
    }
  }

  return 0;
}