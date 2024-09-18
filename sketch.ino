#include <LedControl.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definir el objeto para la pantalla LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C y tamaño de la pantalla

// Definir la serpiente
typedef struct Snake Snake;
struct Snake {
  int head[2];     // (fila, columna) de la cabeza de la serpiente
  int body[40][2]; // Array que contiene las coordenadas (fila, columna)
  int len;         // Longitud de la serpiente
  int dir[2];      // Dirección en la que se mueve la serpiente
};

// Definir la manzana
typedef struct Apple Apple;
struct Apple {
  int rPos; // Índice de fila de la manzana
  int cPos; // Índice de columna de la manzana
};

// Configuración del MAX72XX para la matriz LED
const int DIN = 12;
const int CS = 11;
const int CLK = 10;
LedControl lc = LedControl(DIN, CLK, CS, 1);

// Pines del joystick
const int varXPin = A3; // Valor X del joystick
const int varYPin = A4; // Valor Y del joystick

byte pic[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Las 8 filas de la matriz LED

Snake snake = {{1, 5}, {{0, 5}, {1, 5}}, 2, {1, 0}}; // Inicializar la serpiente
Apple apple = {(int)random(0, 8), (int)random(0, 8)}; // Inicializar la manzana

// Variables para manejar el tiempo del juego
float oldTime = 0;
float timer = 0;
float updateRate = 3;

int puntos = 0; // Puntaje del jugador

void setup() {
  // Iniciar la matriz LED
  lc.shutdown(0, false); // Activar la matriz
  lc.setIntensity(0, 8); // Ajustar el brillo
  lc.clearDisplay(0);    // Limpiar la matriz

  // Configurar el joystick
  pinMode(varXPin, INPUT);
  pinMode(varYPin, INPUT);

  // Configurar la pantalla I2C
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Puntos:");
  lcd.setCursor(8, 0);
  lcd.print(puntos);
}

void loop() {
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  // Leer los valores del joystick
  int xVal = analogRead(varXPin);
  int yVal = analogRead(varYPin);

  // Cambiar la dirección de la serpiente según el joystick
  if (xVal < 100 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = -1;
  } else if (xVal > 920 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = 1;
  } else if (yVal < 100 && snake.dir[0] == 0) {
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  } else if (yVal > 920 && snake.dir[0] == 0) {
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  }

  // Actualizar el juego cada cierto intervalo
  if (timer > 1000 / updateRate) {
    timer = 0;
    Update();
  }

  // Renderizar la serpiente y la manzana
  Render();
}

float calculateDeltaTime() {
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset() {
  for (int j = 0; j < 8; j++) {
    pic[j] = 0;
  }
}

void Update() {
  reset(); // Limpiar la matriz LED

  int newHead[2] = {snake.head[0] + snake.dir[0], snake.head[1] + snake.dir[1]};

  // Manejar los bordes de la matriz
  if (newHead[0] == 8) {
    newHead[0] = 0;
  } else if (newHead[0] == -1) {
    newHead[0] = 7;
  } else if (newHead[1] == 8) {
    newHead[1] = 0;
  } else if (newHead[1] == -1) {
    newHead[1] = 7;
  }

  // Verificar si la serpiente se golpea a sí misma
  for (int j = 0; j < snake.len; j++) {
    if (snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]) {
      delay(1000); // Pausar el juego por 1 segundo y reiniciarlo
      snake = {{1, 5}, {{0, 5}, {1, 5}}, 2, {1, 0}}; // Reiniciar la serpiente
      apple = {(int)random(0, 8), (int)random(0, 8)}; // Reiniciar la manzana
      puntos = 0;
      lcd.setCursor(8, 0);
      lcd.print(puntos); // Mostrar el puntaje en la pantalla
      return;
    }
  }

  // Verificar si la serpiente comió la manzana
  if (newHead[0] == apple.rPos && newHead[1] == apple.cPos) {
    snake.len++;
    puntos++; // Incrementar los puntos
    lcd.setCursor(8, 0);
    lcd.print(puntos); // Mostrar los puntos actualizados en la pantalla
    apple.rPos = (int)random(0, 8);
    apple.cPos = (int)random(0, 8);
  } else {
    removeFirst(); // Desplazar el cuerpo de la serpiente
  }

  snake.body[snake.len - 1][0] = newHead[0];
  snake.body[snake.len - 1][1] = newHead[1];

  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];

  // Actualizar el array pic para mostrar la serpiente y la manzana
  for (int j = 0; j < snake.len; j++) {
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
}

void Render() {
  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, pic[i]); // Mostrar la matriz LED
  }
}

void removeFirst() {
  for (int j = 1; j < snake.len; j++) {
    snake.body[j - 1][0] = snake.body[j][0];
    snake.body[j - 1][1] = snake.body[j][1];
  }
}
