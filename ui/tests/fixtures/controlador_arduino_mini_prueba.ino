// ======================================================
// Arduino Nano + 2x IBT-2 / BTS7960 + 2x Encoder - Prueba sin motores
// ======================================================

// ----------------------
// MOTOR 1: DRIVER
// ----------------------
#define PIN_M1_RPWM 11
#define PIN_M1_LPWM 10
#define PIN_M1_REN  12
#define PIN_M1_LEN  13

// ----------------------
// MOTOR 2: DRIVER
// ----------------------
#define PIN_M2_RPWM 3
#define PIN_M2_LPWM 5
#define PIN_M2_REN  4
#define PIN_M2_LEN  6

// ----------------------
// ENCODER MOTOR 1
// ----------------------
#define PIN_M1_ENCODER_A 8
#define PIN_M1_ENCODER_B 9

// ----------------------
// ENCODER MOTOR 2
// Reemplazá por tus pines definitivos.
// ----------------------
#define PIN_M2_ENCODER_A 2   
#define PIN_M2_ENCODER_B 7


// ======================================================
// CONFIGURACIÓN
// ======================================================

const bool MOTOR_TEST_ENABLED = true;
const int PWM_BAJO = 50;
const int PWM_MEDIO = 120;
const unsigned long INTERVALO_PWM_MS = 3000UL;
// D13 se mantiene HIGH como habilitacion LEN del driver 1.

// ======================================================
// VARIABLES DE ENCODERS
// ======================================================

long encoderM1Count = 0;
long encoderM2Count = 0;

int encoderM1Anterior = 0;
int encoderM2Anterior = 0;

const int8_t encoderTable[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

// ======================================================
// VARIABLES DE MOTORES
// ======================================================

int pwmActual = PWM_BAJO;
unsigned long ultimoCambioPWM = 0;

// ======================================================
// SETUP
// ======================================================

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("================================");
  Serial.println("Rover - Prueba de dos drivers sin motores");
  Serial.println("================================");

  // Encoders
  pinMode(PIN_M1_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_M1_ENCODER_B, INPUT_PULLUP);
  pinMode(PIN_M2_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_M2_ENCODER_B, INPUT_PULLUP);

  encoderM1Anterior =
    (digitalRead(PIN_M1_ENCODER_A) << 1) |
     digitalRead(PIN_M1_ENCODER_B);

  encoderM2Anterior =
    (digitalRead(PIN_M2_ENCODER_A) << 1) |
     digitalRead(PIN_M2_ENCODER_B);


  // Driver motor 1
  pinMode(PIN_M1_RPWM, OUTPUT);
  pinMode(PIN_M1_LPWM, OUTPUT);
  pinMode(PIN_M1_REN, OUTPUT);
  pinMode(PIN_M1_LEN, OUTPUT);

  // Driver motor 2
  pinMode(PIN_M2_RPWM, OUTPUT);
  pinMode(PIN_M2_LPWM, OUTPUT);
  pinMode(PIN_M2_REN, OUTPUT);
  pinMode(PIN_M2_LEN, OUTPUT);

  detenerMotores();

  if (MOTOR_TEST_ENABLED) {
    Serial.println("CONTROL DE DOS MOTORES ACTIVADO");

    digitalWrite(PIN_M1_REN, HIGH);
    digitalWrite(PIN_M1_LEN, HIGH);
    digitalWrite(PIN_M2_REN, HIGH);
    digitalWrite(PIN_M2_LEN, HIGH);
    moverMotores(pwmActual);
    informarPWM();
  }

  ultimoCambioPWM = millis();
}

// ======================================================
// LOOP
// ======================================================

void loop() {
  const unsigned long ahora = millis();
  leerEncoderM1();
  leerEncoderM2();

  if (MOTOR_TEST_ENABLED) {
    controlarPWMPrueba(ahora);
  }
}

// ======================================================
// LECTURA DE ENCODER: MOTOR 1
// ======================================================

void leerEncoderM1() {
  int encoderActual =
    (digitalRead(PIN_M1_ENCODER_A) << 1) |
     digitalRead(PIN_M1_ENCODER_B);

  int indice = (encoderM1Anterior << 2) | encoderActual;
  int movimiento = encoderTable[indice];

  if (movimiento != 0) {
    encoderM1Count += movimiento;
  }

  encoderM1Anterior = encoderActual;
}

// ======================================================
// LECTURA DE ENCODER: MOTOR 2
// ======================================================

void leerEncoderM2() {
  int encoderActual =
    (digitalRead(PIN_M2_ENCODER_A) << 1) |
     digitalRead(PIN_M2_ENCODER_B);

  int indice = (encoderM2Anterior << 2) | encoderActual;
  int movimiento = encoderTable[indice];

  if (movimiento != 0) {
    encoderM2Count += movimiento;
  }

  encoderM2Anterior = encoderActual;
}

// ======================================================
// CONTROL AUTOMATICO DE PWM PARA PRUEBA SIN MOTORES
// ======================================================

void controlarPWMPrueba(unsigned long ahora) {
  // La resta unsigned sigue funcionando cuando millis() desborda.
  if (ahora - ultimoCambioPWM >= INTERVALO_PWM_MS) {
    ultimoCambioPWM = ahora;
    pwmActual = (pwmActual == PWM_BAJO) ? PWM_MEDIO : PWM_BAJO;
    moverMotores(pwmActual);
    informarPWM();
  }
}

void informarPWM() {
  Serial.print("PWM: ");
  Serial.print(pwmActual);
  Serial.print(" | Encoder M1: ");
  Serial.print(encoderM1Count);
  Serial.print(" | Encoder M2: ");
  Serial.println(encoderM2Count);
}
// ======================================================
// MOVER LOS DOS MOTORES
// ======================================================

void moverMotores(int velocidad) {
  analogWrite(PIN_M1_RPWM, velocidad);
  analogWrite(PIN_M1_LPWM, 0);

  analogWrite(PIN_M2_RPWM, velocidad);
  analogWrite(PIN_M2_LPWM, 0);
}

void detenerMotores() {
  analogWrite(PIN_M1_RPWM, 0);
  analogWrite(PIN_M1_LPWM, 0);

  analogWrite(PIN_M2_RPWM, 0);
  analogWrite(PIN_M2_LPWM, 0);
}