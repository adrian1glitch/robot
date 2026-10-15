/*
 * ASTI Robotic Challenge - Mini Sumo
 * Robot: Osoyoo M2 Mecanum
 * Estrategia: Radar con Fijación (Lock-on) + Embestida Ciega (Blind Charge)
 */

#include <Servo.h>

// --- CONFIGURACIÓN DE COMBATE TÁCTICO ---
#define SEARCH_SPEED 120  // Velocidad de giro para buscar
#define ATTACK_SPEED 120  // ¡TURBO! Máxima potencia (Excavadora)
#define ESCAPE_SPEED 200  // Velocidad para huir del borde negro
#define DIST_DETECCION 30 // (cm) Rango visual. Ajustado para no ver fuera del ring
#define DIST_ATAQUE 40    // (cm) Distancia para activar turbo ANTES del punto ciego

#define VALOR_BORDE LOW   // LOW = El sensor detecta la línea negra (absorbe la luz)

// --- VARIABLES DEL RADAR Y MEMORIA ---
int anguloServo = 90;
int pasoServo = 10;       // Velocidad de barrido del servo
int direccionMemoria = 0; // 0=Buscando, 1=Izq, 2=Der, 3=Frente
int tiempoPerdido = 100;  // Contador de ciclos desde que perdió de vista al rival

// --- PINES DE MOTORES ---
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

// --- SENSORES DE LÍNEA (IR) ---
#define sensor1 A4 // Extremo Izquierdo
#define sensor2 A3 // Centro Izquierdo
#define sensor3 A2 // Centro
#define sensor4 A1 // Centro Derecho
#define sensor5 A0 // Extremo Derecho

// --- SENSOR ULTRASÓNICO Y SERVO ---
#define Echo_PIN 31 
#define Trig_PIN 30 
#define SERVO_PIN 13

Servo head;

void setup() {
  // Configuración de pines de Motores
  pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT); pinMode(speedPinR, OUTPUT);
  pinMode(LeftMotorDirPin1, OUTPUT); pinMode(LeftMotorDirPin2, OUTPUT); pinMode(speedPinL, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT); pinMode(speedPinRB, OUTPUT);
  pinMode(LeftMotorDirPin1B, OUTPUT); pinMode(LeftMotorDirPin2B, OUTPUT); pinMode(speedPinLB, OUTPUT);

  // Configuración de pines de Sensores IR
  pinMode(sensor1, INPUT); pinMode(sensor2, INPUT); 
  pinMode(sensor3, INPUT); pinMode(sensor4, INPUT); pinMode(sensor5, INPUT);

  // Configuración de pines de Ultrasonido
  pinMode(Trig_PIN, OUTPUT);
  pinMode(Echo_PIN, INPUT);

  // Inicializar servo
  head.attach(SERVO_PIN);
  head.write(90); 

  Serial.begin(9600);
  
  // --- REGLA OFICIAL ASTI: 5 SEGUNDOS DE ESPERA ---
  stop_Stop();
  delay(5000); 
}

void loop() {
  // 1. LEER SENSORES DE BORDE (Supervivencia ante todo)
  bool s1_izq = (digitalRead(sensor1) == VALOR_BORDE); 
  bool s2_izq = (digitalRead(sensor2) == VALOR_BORDE);
  bool s3_cen = (digitalRead(sensor3) == VALOR_BORDE); 
  bool s4_der = (digitalRead(sensor4) == VALOR_BORDE);
  bool s5_der = (digitalRead(sensor5) == VALOR_BORDE);

  // ESCAPE DE EMERGENCIA
  if (s1_izq || s2_izq) {
    escapar_borde(1); 
  } 
  else if (s4_der || s5_der) {
    escapar_borde(2); 
  } 
  else if (s3_cen) {
    escapar_borde(3); 
  }
  
  // 2. MODO COMBATE: RADAR, FIJACIÓN Y EMBESTIDA
  else {
    int distancia = watch();

    if (distancia > 0 && distancia < DIST_DETECCION) {
      // ¡ENEMIGO A LA VISTA!
      
      // Guardar en memoria dónde lo ha visto
      if (anguloServo > 105) {
        direccionMemoria = 1; // Izquierda
      } else if (anguloServo < 75) {
        direccionMemoria = 2; // Derecha
      } else {
        direccionMemoria = 3; // Frente
      }

      // Lock-on: Fijar la mirada al frente
      anguloServo = 90;
      head.write(anguloServo);
      tiempoPerdido = 0; // Lo estamos viendo AHORA

      ejecutar_movimiento_combate(distancia);
    } 
    else {
      // NO LO VE AHORA MISMO (Desapareció o se metió en el punto ciego)
      
      // Si estaba de frente, le damos MUCHA memoria (40 ciclos) para que haga la Embestida Ciega.
      // Si estaba a los lados, memoria normal (15 ciclos) para encuadrarlo.
      int limiteMemoria = (direccionMemoria == 3) ? 40 : 15;

      if (tiempoPerdido < limiteMemoria) {
        tiempoPerdido++;
        // Enviamos 999 para indicarle a la función que es un ataque a ciegas
        ejecutar_movimiento_combate(999); 
      } 
      else {
        // Se ha escapado definitivamente. Reiniciar búsqueda.
        direccionMemoria = 0; 
        
        // Mover radar
        anguloServo += pasoServo;
        if (anguloServo >= 140 || anguloServo <= 40) {
          pasoServo = -pasoServo;
        }
        head.write(anguloServo);
        delay(20);

        // Girar cuerpo lentamente mientras busca
        set_Motorspeed(SEARCH_SPEED - 30, SEARCH_SPEED - 30, SEARCH_SPEED - 30, SEARCH_SPEED - 30);
        go_Right(); 
      }
    }
  }
}

// --- FUNCIONES DE TÁCTICA Y COMBATE ---

void ejecutar_movimiento_combate(int dist) {
  if (direccionMemoria == 1) {
    // Girar a la izquierda para meterlo en el centro
    set_Motorspeed(SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED);
    go_Left();
  } 
  else if (direccionMemoria == 2) {
    // Girar a la derecha para meterlo en el centro
    set_Motorspeed(SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED);
    go_Right();
  } 
  else if (direccionMemoria == 3) {
    // LO TENEMOS DE FRENTE
    // Si la distancia es corta para el impacto, O si estamos en plena "Embestida Ciega" (999)
    if (dist < DIST_ATAQUE || dist == 999) {
      set_Motorspeed(ATTACK_SPEED, ATTACK_SPEED, ATTACK_SPEED, ATTACK_SPEED); // MÁXIMA POTENCIA
    } else {
      // Acercarse rápido pero controlable
      set_Motorspeed(SEARCH_SPEED + 40, SEARCH_SPEED + 40, SEARCH_SPEED + 40, SEARCH_SPEED + 40); 
    }
    go_Advance();
  }
}

void escapar_borde(int direccion) {
  stop_Stop();
  delay(50); // Proteger motores del cambio brusco
  
  // Marcha atrás
  set_Motorspeed(ESCAPE_SPEED, ESCAPE_SPEED, ESCAPE_SPEED, ESCAPE_SPEED);
  go_Back();
  delay(300);

  // Girar hacia el centro del ring
  if (direccion == 1) {
    go_Right(); 
    delay(400);
  } else if (direccion == 2) {
    go_Left();  
    delay(400);
  } else {
    go_Right(); 
    delay(600); 
  }
  
  // RESET DE COMBATE: Al salvarse, reseteamos la memoria y el servo
  tiempoPerdido = 100; 
  direccionMemoria = 0;
  anguloServo = 90; 
  head.write(90);
  stop_Stop();
}

int watch() {
  digitalWrite(Trig_PIN, LOW); delayMicroseconds(2);
  digitalWrite(Trig_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(Trig_PIN, LOW);
  long duration = pulseIn(Echo_PIN, HIGH, 30000); // 30ms timeout 
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

// --- FUNCIONES DE MOVIMIENTO BÁSICO ---

void set_Motorspeed(int LF, int RF, int LB, int RB) {
  analogWrite(speedPinL, LF); analogWrite(speedPinR, RF);
  analogWrite(speedPinLB, LB); analogWrite(speedPinRB, RB);
}

void go_Advance() {
  digitalWrite(RightMotorDirPin1, LOW); digitalWrite(RightMotorDirPin2, HIGH);
  digitalWrite(LeftMotorDirPin1, LOW);  digitalWrite(LeftMotorDirPin2, HIGH);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, HIGH);
  digitalWrite(LeftMotorDirPin1B, LOW);  digitalWrite(LeftMotorDirPin2B, HIGH);
}

void go_Back() {
  digitalWrite(RightMotorDirPin1, HIGH); digitalWrite(RightMotorDirPin2, LOW);
  digitalWrite(LeftMotorDirPin1, HIGH);  digitalWrite(LeftMotorDirPin2, LOW);
  digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B, HIGH);  digitalWrite(LeftMotorDirPin2B, LOW);
}

void go_Right() { 
  digitalWrite(RightMotorDirPin1, HIGH); digitalWrite(RightMotorDirPin2, LOW);
  digitalWrite(LeftMotorDirPin1, LOW);  digitalWrite(LeftMotorDirPin2, HIGH);
  digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B, LOW);  digitalWrite(LeftMotorDirPin2B, HIGH);
}

void go_Left() { 
  digitalWrite(RightMotorDirPin1, LOW); digitalWrite(RightMotorDirPin2, HIGH);
  digitalWrite(LeftMotorDirPin1, HIGH);  digitalWrite(LeftMotorDirPin2, LOW);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, HIGH);
  digitalWrite(LeftMotorDirPin1B, HIGH);  digitalWrite(LeftMotorDirPin2B, LOW);
}

void stop_Stop() {
  set_Motorspeed(0, 0, 0, 0);
}
