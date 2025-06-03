#define PIN_RED_LED (1 << PD3)
#define PIN_GREEN_LED (1 << PD5)
#define PIN_BLUE_LED (1 << PD6)
#define TOGGLE_BUTTON (1 << PD2)

#define PWM_RED OCR2B
#define PWM_GREEN OCR0B
#define PWM_BLUE OCR0A

uint8_t curr_red, curr_green, curr_blue; // valores RGB utilizados na memoria
bool leds_on = false; // variavel global utilizada para verificar se os leds estao ligados

// ------------------------ Metodo de interrupcao, liga e desliga o led sem alterar as cores na memoria
ISR(INT0_vect)
{
  if (leds_on)
  {
    PWM_RED = 0;
    PWM_GREEN = 0;
    PWM_BLUE = 0;
    leds_on = false;
  }
  else
  {
    PWM_RED = curr_red;
    PWM_GREEN = curr_green;
    PWM_BLUE = curr_blue;
    leds_on = true;
  }
}

// ------------------------ Metodo auxiliar para setar a cor RGB na memoria
void set_colors_rgb(int r, int g, int b)
{
  curr_red = r;
  curr_green = g;
  curr_blue = b;
}

// ------------------------ Metodo auxiliar para setar a cor HSL na memoria, realizando uma conversao
void set_colors_hsl(int h, int s, int l)
{
  if (h < 0 || h > 360 || s < 0 || s > 100 || l < 0 || l > 100) return;
  float H = (float)h;
  float S = (float)s / 100.0;
  float L = (float)l / 100.0;

  float C = (1 - fabs(2 * L - 1)) * S;
  float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
  float m = L - C / 2;

  float r_prime, g_prime, b_prime;

  if (H >= 0 && H < 60) 
  {
    r_prime = C;
    g_prime = X;
    b_prime = 0;
  } 
  else if (H >= 60 && H < 120) 
  {
    r_prime = X;
    g_prime = C;
    b_prime = 0;
  } 
  else if (H >= 120 && H < 180) 
  {
    r_prime = 0;
    g_prime = C;
    b_prime = X;
  } 
  else if (H >= 180 && H < 240) 
  {
    r_prime = 0;
    g_prime = X;
    b_prime = C;
  } 
  else if (H >= 240 && H < 300) 
  {
    r_prime = X;
    g_prime = 0;
    b_prime = C;
  } 
  else 
  {
    r_prime = C;
    g_prime = 0;
    b_prime = X;
  }

  curr_red = (uint8_t)((r_prime + m) * 255);
  curr_green = (uint8_t)((g_prime + m) * 255);
  curr_blue = (uint8_t)((b_prime + m) * 255);

  PWM_RED = curr_red;
  PWM_GREEN = curr_green;
  PWM_BLUE = curr_blue;
}

// ------------------------ Metodo auxiliar para setar a cor HSV na memoria, realizando uma conversao
void set_colors_hsv(int h, int s, int v)
{
  if (h < 0 || h > 360 || s < 0 || s > 100 || v < 0 || v > 100) return;
  float H = (float)h;
  float S = (float)s / 100.0;
  float V = (float)v / 100.0;

  float C = V * S;
  float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
  float m = V - C;

  float r_prime, g_prime, b_prime;

  if (H >= 0 && H < 60)
  {
    r_prime = C;
    g_prime = X;
    b_prime = 0;
  } 
  else if (H >= 60 && H < 120)
  {
    r_prime = X;
    g_prime = C;
    b_prime = 0;
  } 
  else if (H >= 120 && H < 180)
  {
    r_prime = 0;
    g_prime = C;
    b_prime = X;
  } 
  else if (H >= 180 && H < 240)
  {
    r_prime = 0;
    g_prime = X;
    b_prime = C;
  } 
  else if (H >= 240 && H < 300)
  {
    r_prime = X;
    g_prime = 0;
    b_prime = C;
  } 
  else
  {
    r_prime = C;
    g_prime = 0;
    b_prime = X;
  }

  curr_red = (uint8_t)((r_prime + m) * 255);
  curr_green = (uint8_t)((g_prime + m) * 255);
  curr_blue = (uint8_t)((b_prime + m) * 255);
}

int main()
{
  DDRD |= PIN_RED_LED | PIN_GREEN_LED | PIN_BLUE_LED; // Define os pinos como OUTPUT

  // ------------------------ Configuracao da Interrupcao ------------------------
  EICRA |= (1 << ISC01) | (1 << ISC00);  // rising edge INT0
  EIMSK |= (1 << INT0); // Habilita a interrupcao externa INT0
  sei(); // Habilita interrupcoes globais

  // ------------------------ Configuração do PWM --------------------------------
  TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1) | (1 << COM0B1); // fast PWM nao invertido
  TCCR0B = (1 << CS00) | (1 << CS01); // prescaler 64

  TCCR2A = (1 << WGM20) | (1 << WGM21) | (1 << COM2B1); // fast PWM nao invertido
  TCCR2B = (1 << CS22); // prescaler 64

  set_colors_hsv(0,100,100);
  while (1) { }
  return 0;
}