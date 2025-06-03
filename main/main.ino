#define PIN_RED_LED (1 << PD3)
#define PIN_GREEN_LED (1 << PD5)
#define PIN_BLUE_LED (1 << PD6)
#define TOGGLE_BUTTON (1 << PD2)

#define PWM_RED OCR2B
#define PWM_GREEN OCR0B
#define PWM_BLUE OCR0A

uint8_t curr_red, curr_green, curr_blue; // valores RGB utilizados na memoria
bool leds_on = true; // variavel global utilizada para verificar se os leds estao ligados

// ------------------------ Metodo de interrupcao, liga e desliga o led sem alterar as cores na memoria
ISR(INT0_vect)
{
  if (leds_on)
  {
    PWM_RED = 120;
    PWM_GREEN = 30;
    PWM_BLUE = 200;
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

int main()
{
  DDRD = PIN_RED_LED | PIN_GREEN_LED | PIN_BLUE_LED; // Define os pinos como OUTPUT

  // ------------------------ Configuracao da Interrupcao ------------------------
  EICRA |= (1 << ISC01) | (1 << ISC00);  // rising edge INT0
  EIMSK |= (1 << INT0); // Habilita a interrupcao externa INT0
  sei(); // Habilita interrupcoes globais

  // ------------------------ Configuração do PWM --------------------------------
  TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1) | (1 << COM0B1); // fast PWM nao invertido
  TCCR0B = (1 << CS00) | (1 << CS01); // prescaler 64

  TCCR2A = (1 << WGM20) | (1 << WGM21) | (1 << COM2B1); // fast PWM nao invertido
  TCCR2B = (1 << CS22); // prescaler 64

  while (1) { }
  return 0;
}