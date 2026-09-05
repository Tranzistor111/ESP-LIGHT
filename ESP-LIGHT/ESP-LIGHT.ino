#include <Adafruit_NeoPixel.h>

// Настройки светодиодов 
#define LED_COUNT   1   // Количество LED на одном модуле
#define NUM_STRIPS  5   // Количество светодиодов 
#define BRIGHTNESS  100 // Начальная яркость при включении 

// Настройки яркости
#define BRIGHT_STEP 25  // Шаг изменения яркости за одно нажатие 
#define BRIGHT_MIN  10  // Минимальная яркость 
#define BRIGHT_MAX  255 // Максимальная яркость

// Пины для LED 
const int ledPins[NUM_STRIPS] = {D1, D2, D3, D4, D5};

// Пины кнопок 
#define BTN_OK_PIN    3    // RX
#define BTN_PLUS_PIN  D6   // D6 
#define BTN_MINUS_PIN D7   // D7 

Adafruit_NeoPixel strips[NUM_STRIPS] = {
  Adafruit_NeoPixel(LED_COUNT, ledPins[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, ledPins[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, ledPins[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, ledPins[3], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, ledPins[4], NEO_GRB + NEO_KHZ800)
};


enum Mode {
  MODE_RAINBOW,   
  MODE_RED,       
  MODE_ORANGE,    
  MODE_YELLOW,    
  MODE_GREEN,     
  MODE_CYAN,      
  MODE_BLUE,      
  MODE_PURPLE,    
  MODE_STROBE,    
  MODE_COUNT      
};

Mode currentMode = MODE_RAINBOW; 
int currentBrightness = BRIGHTNESS;
float rainbowHue = 0;             

const uint32_t colors[] = {
  0,          
  0xFF0000,   
  0xFF7F00,  
  0xFFFF00,   
  0x00FF00,   
  0x00FFFF,  
  0x0000FF,   
  0xFF00FF,   
  0           
};

unsigned long tOk = 0, tPlus = 0, tMinus = 0;

const unsigned long debounce = 200;

bool okPressed = false, plusPressed = false, minusPressed = false;

void setup() {
  pinMode(BTN_OK_PIN, INPUT_PULLUP);
  pinMode(BTN_PLUS_PIN, INPUT_PULLUP);
  pinMode(BTN_MINUS_PIN, INPUT_PULLUP);
  
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].begin();                    // Запуск ленты
    strips[i].setBrightness(currentBrightness); // Установка начальной яркости
    strips[i].show();                     // Выключить все LED (чёрный цвет)
  }
}

void loop() {
  unsigned long now = millis(); 

  if (digitalRead(BTN_OK_PIN) == LOW) {
    if (!okPressed && (now - tOk > debounce)) {
      okPressed = true;    
      tOk = now;      
      
      currentMode = (Mode)((currentMode + 1) % MODE_COUNT);
      
      if (currentMode == MODE_RAINBOW) rainbowHue = 0;
    }
  } else {
    okPressed = false;
  }

  if (digitalRead(BTN_PLUS_PIN) == LOW) {
    if (!plusPressed && (now - tPlus > debounce)) {
      plusPressed = true;
      tPlus = now;
      
      currentBrightness += BRIGHT_STEP;
      if (currentBrightness > BRIGHT_MAX) currentBrightness = BRIGHT_MAX;
      
      setBrightnessAll(currentBrightness);
    }
  } else {
    plusPressed = false;
  }

  if (digitalRead(BTN_MINUS_PIN) == LOW) {
    if (!minusPressed && (now - tMinus > debounce)) {
      minusPressed = true;
      tMinus = now;
      
      currentBrightness -= BRIGHT_STEP;
      if (currentBrightness < BRIGHT_MIN) currentBrightness = BRIGHT_MIN;
      
      setBrightnessAll(currentBrightness);
    }
  } else {
    minusPressed = false;
  }

  switch (currentMode) {
    case MODE_RAINBOW:
      smoothRainbow(10); 
      break;
      
    case MODE_STROBE:
      strobeEffect();    
      break;
      
    default:
      fillAll(colors[currentMode]);
      delay(10);          
      break;
  }
}

void smoothRainbow(int speedDelay) {
  uint32_t c = HSVtoRGB(rainbowHue, 1.0, 1.0);
  
  fillAll(c);          
  
  rainbowHue += 0.002;  
  if (rainbowHue >= 1.0) rainbowHue = 0; 
  
  delay(speedDelay);    
}

void strobeEffect() {
  fillAll(0xFF0000);  
  delay(80);
  fillAll(0x000000);  
  delay(80);
  fillAll(0x0000FF);  
  delay(80);
  fillAll(0x000000);  
  delay(80);
}

void fillAll(uint32_t c) {
  for (int s = 0; s < NUM_STRIPS; s++) {     
    for (int p = 0; p < LED_COUNT; p++) {     
      strips[s].setPixelColor(p, c);          
    }
    strips[s].show();                        
  }
}

void setBrightnessAll(int brightness) {
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].setBrightness(brightness);        
  }
  
  if (currentMode != MODE_RAINBOW) {
    fillAll(colors[currentMode]);
  }
}

uint32_t HSVtoRGB(float h, float s, float v) {
  int i = floor(h * 6);           
  float f = h * 6 - i;           
  float p = v * (1 - s);          
  float q = v * (1 - f * s);      
  float t = v * (1 - (1 - f) * s); 
  
  float r, g, b;
  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;  
    case 1: r = q; g = v; b = p; break; 
    case 2: r = p; g = v; b = t; break;  
    case 3: r = p; g = q; b = v; break; 
    case 4: r = t; g = p; b = v; break;  
    case 5: r = v; g = p; b = q; break;  
    default: r = g = b = 0; break;      
  }
  
  return strips[0].Color(r * 255, g * 255, b * 255);
}