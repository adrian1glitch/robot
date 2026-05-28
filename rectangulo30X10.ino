/* === CÓDIGO: EL CUADRADO (CORREGIDO PARA MECANUM) === */

// --- CALIBRACIÓN DE TIEMPOS ---
const unsigned long TIEMPO_1CM = 75;     // Tiempo en ms para avanzar 1 cm
const unsigned long TIEMPO_GIRO_1G = 6;  // Tiempo en ms para rotar 1 grado

#define MAX_SPEED 160     // Velocidad para avanzar en recta
#define ROTATE_SPEED 130  // Velocidad para rotar en las esquinas

// --- TUS PINES DE CONFIGURACIÓN ---
#define speedPinR 9 
#define RightMotorDirPin1 22 
#define RightMotorDirPin2 24 
#define LeftMotorDirPin1 26 
#define LeftMotorDirPin2 28 
#define speedPinL 10 
#define speedPinRB 11 
#define RightMotorDirPin1B 5 
#define RightMotorDirPin2B 6 
#define LeftMotorDirPin1B 7 
#define LeftMotorDirPin2B 8 
#define speedPinLB 12 

void setup() {
  init_GPIO();
  delay(3000); // 3 segundos para colocar el robot en el papel

  // --- TRAZAR CUADRADO (4 lados de 20cm, giros de 90°) ---
  for (int i = 0; i < 2; i++) {
    avanzar(2);
    delay(400); // Pausa de estabilización mecánica
    girarIzquierda(72);
    delay(400);
    avanzar(7);
    delay(400);
    girarIzquierda(72);
    delay(400);
  }
}

void loop() {}

void avanzar(int centimetros) {
  forward(MAX_SPEED, MAX_SPEED); // Usa tu función nativa de avanzar
  delay(centimetros * TIEMPO_1CM);
  stop_bot();
}

void girarIzquierda(int grados) {
  sharpLeftTurn(ROTATE_SPEED, ROTATE_SPEED); // Usa tu función nativa de giro
  delay(grados * TIEMPO_GIRO_1G);
  stop_bot();
}

// --- TUS FUNCIONES NATIVAS DE MOTOR ---
void forward(int speed_left, int speed_right) {
  digitalWrite(LeftMotorDirPin1, LOW); digitalWrite(LeftMotorDirPin2, HIGH); 
  analogWrite(speedPinL, speed_left);
  digitalWrite(LeftMotorDirPin1B, LOW); digitalWrite(LeftMotorDirPin2B, HIGH); 
  analogWrite(speedPinLB, speed_left);
  
  digitalWrite(RightMotorDirPin1, LOW); digitalWrite(RightMotorDirPin2, HIGH); 
  analogWrite(speedPinR, speed_right);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, HIGH); 
  analogWrite(speedPinRB, speed_right);
}

void sharpLeftTurn(int sl, int sr) {
  // Lado Izquierdo hacia ATRÁS (Invertido respecto a forward)
  digitalWrite(LeftMotorDirPin1, HIGH); digitalWrite(LeftMotorDirPin2, LOW); 
  analogWrite(speedPinL, sl);
  digitalWrite(LeftMotorDirPin1B, HIGH); digitalWrite(LeftMotorDirPin2B, LOW); 
  analogWrite(speedPinLB, sl);
  
  // Lado Derecho hacia ADELANTE
  digitalWrite(RightMotorDirPin1, LOW); digitalWrite(RightMotorDirPin2, HIGH); 
  analogWrite(speedPinR, sr);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, HIGH); 
  analogWrite(speedPinRB, sr);
}

void stop_bot() {
  analogWrite(speedPinLB,0); analogWrite(speedPinRB,0);
  analogWrite(speedPinL,0);  analogWrite(speedPinR,0);
}

void init_GPIO() {
  pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT); pinMode(speedPinL, OUTPUT);  
  pinMode(LeftMotorDirPin1, OUTPUT); pinMode(LeftMotorDirPin2, OUTPUT); pinMode(speedPinR, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT); pinMode(speedPinLB, OUTPUT);  
  pinMode(LeftMotorDirPin1B, OUTPUT); pinMode(LeftMotorDirPin2B, OUTPUT); pinMode(speedPinRB, OUTPUT);
}