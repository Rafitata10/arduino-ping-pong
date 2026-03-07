/***************************************************
  PONG v2 – Arduino Nano + OLED I2C + Botón Pausa/Iniciar

  ─ Hardware ─
  • Arduino Nano (ATmega328P)
  • Pantalla OLED SSD1306 128×64 I2C (4 pines)
  • 2 potenciómetros (paletas)  → A0 y A1
  • Altavoz (Buzzer pasivo)     → D3
  • Botón pulsador (pausa)      → D2 + GND

  ─ Conexiones ─
  OLED VCC  → 5V
  OLED GND  → GND
  OLED SDA  → A4
  OLED SCL  → A5

  POT A centro → A0
  POT B centro → A1

  BOTÓN → D2 y GND
  BUZZER + → D3
  BUZZER - → GND

  Dirección OLED habitual: 0x3C
***************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>

/* ─── Pines ───────────────────────────────────────── */

#define BUTTON_PIN 2
#define BEEPER 3
#define CONTROL_A A0
#define CONTROL_B A1

/* ─── Constantes del juego ───────────────────────── */

#define SCREEN_WIDTH 127
#define SCREEN_HEIGHT 63

#define FONT_SIZE 2
#define PADDLE_WIDTH 4
#define PADDLE_HEIGHT 10
#define PADDLE_PADDING 10
#define BALL_SIZE 3
#define SCORE_PADDING 10

#define EFFECT_SPEED 0.5
#define MIN_Y_SPEED 0.5
#define MAX_Y_SPEED 2

/* ─── Pantalla ───────────────────────────────────── */

Adafruit_SSD1306 display(128, 64, &Wire, -1);

/* ─── Variables globales ─────────────────────────── */

volatile bool buttonEvent = false;

bool gameStarted = false;
bool paused = true;

int paddleLocationA = 0;
int paddleLocationB = 0;

float ballX = SCREEN_WIDTH / 2;
float ballY = SCREEN_HEIGHT / 2;

float ballSpeedX = 2;
float ballSpeedY = 1;

int lastPaddleLocationA = 0;
int lastPaddleLocationB = 0;

int scoreA = 0;
int scoreB = 0;

/* ─── Prototipos ─────────────────────────────────── */

void handleButton();
void splash();
void calculateMovement();
void draw();
void addEffect(int paddleSpeed);

void soundStart();
void soundBounce();
void soundPoint();

void centerPrint(const char *text, int y, int size);

/* ────────────────────────────────────────────────── */
/* SETUP */
/* ────────────────────────────────────────────────── */

void setup() {

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BEEPER, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }

  display.setTextWrap(false);

  splash();
}

/* ────────────────────────────────────────────────── */
/* LOOP */
/* ────────────────────────────────────────────────── */

void loop() {

  if (buttonEvent) {

    buttonEvent = false;

    if (!gameStarted) {
      gameStarted = true;
      paused = false;
      soundStart();
    }
    else {
      paused = !paused;
    }
  }

  if (gameStarted && !paused) {
    calculateMovement();
  }

  draw();
}

/* ────────────────────────────────────────────────── */
/* INTERRUPCIÓN DEL BOTÓN */
/* ────────────────────────────────────────────────── */

void handleButton() {

  static unsigned long lastTime = 0;

  unsigned long now = millis();

  if (now - lastTime > 200) {

    buttonEvent = true;
    lastTime = now;
  }
}

/* ────────────────────────────────────────────────── */
/* SPLASH SCREEN */
/* ────────────────────────────────────────────────── */

void splash() {

  display.clearDisplay();
  display.setTextColor(WHITE);

  centerPrint("PONG", 0, 3);
  centerPrint("By Allan Alcorn", 24, 1);
  centerPrint("Ported by", 33, 1);
  centerPrint("Rafael Ramirez Salas", 42, 1);

  display.fillRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, WHITE);

  display.setTextColor(BLACK);
  centerPrint("Press button!", SCREEN_HEIGHT - 9, 1);

  display.display();

  while (!gameStarted && !buttonEvent) {
    delay(10);
  }
}

/* ────────────────────────────────────────────────── */
/* MOVIMIENTO DEL JUEGO */
/* ────────────────────────────────────────────────── */

void calculateMovement() {

  int controlA = analogRead(CONTROL_A);
  int controlB = analogRead(CONTROL_B);

  paddleLocationA = map(controlA, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  paddleLocationB = map(controlB, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);

  int paddleSpeedA = paddleLocationA - lastPaddleLocationA;
  int paddleSpeedB = paddleLocationB - lastPaddleLocationB;

  ballX += ballSpeedX;
  ballY += ballSpeedY;

  /* Rebote en bordes */

  if (ballY >= SCREEN_HEIGHT - BALL_SIZE || ballY <= 0) {

    ballSpeedY = -ballSpeedY;
    soundBounce();
  }

  /* Rebote en paleta A */

  if (ballX <= PADDLE_PADDING + PADDLE_WIDTH &&
      ballX >= PADDLE_PADDING &&
      ballSpeedX < 0) {

    if (ballY >= paddleLocationA - BALL_SIZE &&
        ballY <= paddleLocationA + PADDLE_HEIGHT) {

      ballSpeedX = -ballSpeedX;
      addEffect(paddleSpeedA);
      soundBounce();
    }
  }

  /* Rebote en paleta B */

  if (ballX >= SCREEN_WIDTH - PADDLE_WIDTH - PADDLE_PADDING - BALL_SIZE &&
      ballSpeedX > 0) {

    if (ballY >= paddleLocationB - BALL_SIZE &&
        ballY <= paddleLocationB + PADDLE_HEIGHT) {

      ballSpeedX = -ballSpeedX;
      addEffect(paddleSpeedB);
      soundBounce();
    }
  }

  /* Punto */

  if (ballX >= SCREEN_WIDTH - BALL_SIZE || ballX <= 0) {

    if (ballSpeedX > 0) {
      scoreA++;
      ballX = SCREEN_WIDTH / 4;
    }
    else {
      scoreB++;
      ballX = SCREEN_WIDTH * 3 / 4;
    }

    ballSpeedX = -ballSpeedX;

    soundPoint();
  }

  lastPaddleLocationA = paddleLocationA;
  lastPaddleLocationB = paddleLocationB;
}

/* ────────────────────────────────────────────────── */
/* DIBUJO */
/* ────────────────────────────────────────────────── */

void draw() {

  display.clearDisplay();

  /* Paletas */

  display.fillRect(
    PADDLE_PADDING,
    paddleLocationA,
    PADDLE_WIDTH,
    PADDLE_HEIGHT,
    WHITE);

  display.fillRect(
    SCREEN_WIDTH - PADDLE_WIDTH - PADDLE_PADDING,
    paddleLocationB,
    PADDLE_WIDTH,
    PADDLE_HEIGHT,
    WHITE);

  /* Línea central */

  for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
    display.drawFastVLine(SCREEN_WIDTH / 2, i, 2, WHITE);
  }

  /* Bola */

  display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, WHITE);

  /* Marcador */

  display.setTextColor(WHITE);
  display.setTextSize(FONT_SIZE);

  display.setCursor(SCREEN_WIDTH / 2 - SCORE_PADDING - 20, 0);
  display.print(scoreA);

  display.setCursor(SCREEN_WIDTH / 2 + SCORE_PADDING + 1, 0);
  display.print(scoreB);

  if (paused) {
    centerPrint("PAUSED", SCREEN_HEIGHT / 2 - 8, 2);
  }

  display.display();
}

/* ────────────────────────────────────────────────── */
/* EFECTO DE SPIN */
/* ────────────────────────────────────────────────── */

void addEffect(int paddleSpeed) {

  ballSpeedY += paddleSpeed * EFFECT_SPEED;

  if (ballSpeedY > MAX_Y_SPEED) ballSpeedY = MAX_Y_SPEED;
  if (ballSpeedY < -MAX_Y_SPEED) ballSpeedY = -MAX_Y_SPEED;

  if (ballSpeedY > 0 && ballSpeedY < MIN_Y_SPEED)
    ballSpeedY = MIN_Y_SPEED;

  if (ballSpeedY < 0 && ballSpeedY > -MIN_Y_SPEED)
    ballSpeedY = -MIN_Y_SPEED;
}

/* ────────────────────────────────────────────────── */
/* SONIDOS */
/* ────────────────────────────────────────────────── */

void soundStart() {

  tone(BEEPER, 250);
  delay(100);

  tone(BEEPER, 500);
  delay(100);

  tone(BEEPER, 1000);
  delay(100);

  noTone(BEEPER);
}

void soundBounce() {

  tone(BEEPER, 500, 50);
}

void soundPoint() {

  tone(BEEPER, 150, 150);
}

/* ────────────────────────────────────────────────── */
/* TEXTO CENTRADO */
/* ────────────────────────────────────────────────── */

void centerPrint(const char *text, int y, int size) {

  display.setTextSize(size);

  int x = SCREEN_WIDTH / 2 - (strlen(text) * 6 * size) / 2;

  display.setCursor(x, y);

  display.print(text);
}
