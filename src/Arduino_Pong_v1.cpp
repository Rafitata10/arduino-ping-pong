/*************************************************** 
  PONG para Arduino Nano + Pantalla OLED I2C (4 pines)

  ─ Hardware ─
  • Arduino Nano (ATmega328P)
  • Pantalla OLED 128×64 I2C (4 pines → VCC, GND, SDA, SCL)
  • 2 potenciómetros (controles de las paletas)
  • Buzzer piezoeléctrico en el pin 3

  ─ Cableado rápido ─
  OLED SDA → A4   (Nano SDA)
  OLED SCL → A5   (Nano SCL)
  OLED VCC → 5 V
  OLED GND → GND
  Buzzer    → D3 + GND
  PotA      → A0 (+ 5 V & GND)
  PotB      → A1 (+ 5 V & GND)

  Si la pantalla no muestra nada, prueba cambiar la dirección
  I2C de 0x3C a 0x3D.

  Código original: Michael Teeuw | Xonay Labs
  Adaptado a OLED I2C sin pin RESET y a Arduino Nano.
  Licencia Apache 2.0
 ****************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>        // Para strlen en centerPrint()

// ─── Pines de entrada / salida ────────────────────────────────────────────────
#define BEEPER     3        // Buzzer piezo
#define CONTROL_A  A0       // Potenciómetro jugador A
#define CONTROL_B  A1       // Potenciómetro jugador B

// ─── Constantes de pantalla y juego ──────────────────────────────────────────
#define SCREEN_WIDTH   127  // 0‑127  → 128 píxeles reales
#define SCREEN_HEIGHT  63   // 0‑63   → 64 píxeles reales

#define FONT_SIZE      2
#define PADDLE_WIDTH   4
#define PADDLE_HEIGHT  10
#define PADDLE_PADDING 10
#define BALL_SIZE      3
#define SCORE_PADDING  10

#define EFFECT_SPEED   0.5
#define MIN_Y_SPEED    0.5
#define MAX_Y_SPEED    2

// ─── Objeto display: ancho, alto, Wire, sin RESET (‑1) ───────────────────────
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ─── Variables de estado ─────────────────────────────────────────────────────
int   paddleLocationA = 0;
int   paddleLocationB = 0;

float ballX      = SCREEN_WIDTH  / 2;
float ballY      = SCREEN_HEIGHT / 2;
float ballSpeedX = 2;
float ballSpeedY = 1;

int   lastPaddleLocationA = 0;
int   lastPaddleLocationB = 0;

int   scoreA = 0;
int   scoreB = 0;

// ──────────────────────────────────────────────────────────────────────────────
// SETUP
// ──────────────────────────────────────────────────────────────────────────────
void setup() {
  // Inicia pantalla OLED (0x3C es la dirección I2C más común)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Si falla, queda bloqueado aquí
    for (;;);
  }

  display.clearDisplay();
  display.display();
  display.setTextWrap(false);

  splash();                 // Pantalla de inicio

  display.setTextColor(WHITE);
  display.setTextSize(FONT_SIZE);
  display.clearDisplay();
}

// ──────────────────────────────────────────────────────────────────────────────
// BUCLE PRINCIPAL
// ──────────────────────────────────────────────────────────────────────────────
void loop() {
  calculateMovement();
  draw();
}

// ──────────────────────────────────────────────────────────────────────────────
// FUNCIÓN: Pantalla de bienvenida
// ──────────────────────────────────────────────────────────────────────────────
void splash() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  centerPrint("PONG v2", 0, 3);
  centerPrint("By Allan Alcorn", 24, 1);
  centerPrint("Ported by", 33, 1);
  centerPrint("Rafael Ramirez Salas", 42, 1);

  display.fillRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, WHITE);
  display.setTextColor(BLACK);
  centerPrint("Move paddle to start!", SCREEN_HEIGHT - 9, 1);

  display.display();

  int controlA = analogRead(CONTROL_A);
  int controlB = analogRead(CONTROL_B);

  // Espera a que alguno de los potenciómetros se mueva
  while (abs(controlA - analogRead(CONTROL_A) + controlB - analogRead(CONTROL_B)) < 10) {
    /* Nada */
  }

  soundStart();
}

// ──────────────────────────────────────────────────────────────────────────────
// FUNCIÓN: Física del juego
// ──────────────────────────────────────────────────────────────────────────────
void calculateMovement() {
  int controlA = analogRead(CONTROL_A);
  int controlB = analogRead(CONTROL_B);

  paddleLocationA = map(controlA, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  paddleLocationB = map(controlB, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);

  int paddleSpeedA = paddleLocationA - lastPaddleLocationA;
  int paddleSpeedB = paddleLocationB - lastPaddleLocationB;

  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Rebote en bordes superior / inferior
  if (ballY >= SCREEN_HEIGHT - BALL_SIZE || ballY <= 0) {
    ballSpeedY *= -1;
    soundBounce();
  }

  // Rebote en paleta A
  if (ballX >= PADDLE_PADDING && ballX <= PADDLE_PADDING + BALL_SIZE && ballSpeedX < 0) {
    if (ballY > paddleLocationA - BALL_SIZE && ballY < paddleLocationA + PADDLE_HEIGHT) {
      ballSpeedX *= -1;
      addEffect(paddleSpeedA);
      soundBounce();
    }
  }

  // Rebote en paleta B
  if (ballX >= SCREEN_WIDTH - PADDLE_WIDTH - PADDLE_PADDING - BALL_SIZE &&
      ballX <= SCREEN_WIDTH - PADDLE_PADDING - BALL_SIZE && ballSpeedX > 0) {
    if (ballY > paddleLocationB - BALL_SIZE && ballY < paddleLocationB + PADDLE_HEIGHT) {
      ballSpeedX *= -1;
      addEffect(paddleSpeedB);
      soundBounce();
    }
  }

  // Gol: la bola pasa detrás de una paleta
  if (ballX >= SCREEN_WIDTH - BALL_SIZE || ballX <= 0) {
    if (ballSpeedX > 0) {
      scoreA++;
      ballX = SCREEN_WIDTH / 4;                 // Saca desde la izquierda
    } else {
      scoreB++;
      ballX = SCREEN_WIDTH / 4 * 3;             // Saca desde la derecha
    }
    soundPoint();
  }

  // Guarda última posición de paletas
  lastPaddleLocationA = paddleLocationA;
  lastPaddleLocationB = paddleLocationB;
}

// ──────────────────────────────────────────────────────────────────────────────
// FUNCIÓN: Dibuja escena completa
// ──────────────────────────────────────────────────────────────────────────────
void draw() {
  display.clearDisplay();

  // Paleta A
  display.fillRect(PADDLE_PADDING, paddleLocationA, PADDLE_WIDTH, PADDLE_HEIGHT, WHITE);

  // Paleta B
  display.fillRect(SCREEN_WIDTH - PADDLE_WIDTH - PADDLE_PADDING, paddleLocationB,
                   PADDLE_WIDTH, PADDLE_HEIGHT, WHITE);

  // Línea central punteada
  for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
    display.drawFastVLine(SCREEN_WIDTH / 2, i, 2, WHITE);
  }

  // Bola
  display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, WHITE);

  // Marcador (alineación derecha manual para score A)
  int scoreAWidth = 5 * FONT_SIZE;
  if (scoreA > 9)    scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 99)   scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 999)  scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 9999) scoreAWidth += 6 * FONT_SIZE;

  display.setCursor(SCREEN_WIDTH / 2 - SCORE_PADDING - scoreAWidth, 0);
  display.print(scoreA);

  display.setCursor(SCREEN_WIDTH / 2 + SCORE_PADDING + 1, 0); // +1 por línea central
  display.print(scoreB);

  display.display();
}

// ──────────────────────────────────────────────────────────────────────────────
// FUNCIÓN: Efecto de efecto (spin) al golpear paleta en movimiento
// ──────────────────────────────────────────────────────────────────────────────
void addEffect(int paddleSpeed) {
  float oldBallSpeedY = ballSpeedY;

  // Cada pixel de movimiento de la paleta añade/quita velocidad
  for (int effect = 0; effect < abs(paddleSpeed); effect++) {
    ballSpeedY += (paddleSpeed > 0) ? EFFECT_SPEED : -EFFECT_SPEED;
  }

  // Límites mínimo
  if (ballSpeedY < 0 && ballSpeedY > -MIN_Y_SPEED) ballSpeedY = -MIN_Y_SPEED;
  if (ballSpeedY > 0 && ballSpeedY <  MIN_Y_SPEED) ballSpeedY =  MIN_Y_SPEED;
  if (ballSpeedY == 0) ballSpeedY = oldBallSpeedY;

  // Límites máximo
  if (ballSpeedY >  MAX_Y_SPEED) ballSpeedY =  MAX_Y_SPEED;
  if (ballSpeedY < -MAX_Y_SPEED) ballSpeedY = -MAX_Y_SPEED;
}

// ──────────────────────────────────────────────────────────────────────────────
// FX de sonido (requiere buzzer pasivo en D3)
// ──────────────────────────────────────────────────────────────────────────────
void soundStart() {
  tone(BEEPER, 250);  delay(100);
  tone(BEEPER, 500);  delay(100);
  tone(BEEPER, 1000); delay(100);
  noTone(BEEPER);
}

void soundBounce() {
  tone(BEEPER, 500, 50);
}

void soundPoint() {
  tone(BEEPER, 150, 150);
}

// ──────────────────────────────────────────────────────────────────────────────
// Helper: Imprime centrado horizontalmente
// ──────────────────────────────────────────────────────────────────────────────
void centerPrint(const char *text, int y, int size) {
  display.setTextSize(size);
  display.setCursor(SCREEN_WIDTH / 2 - (strlen(text) * 6 * size) / 2, y);
  display.print(text);
}
