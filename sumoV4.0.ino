/*
 * ASTI Robotic Challenge - Mini Sumo
 * Robot: Osoyoo M2 Mecanum
 * Estrategia: Radar Fijo + Embestida Controlada + Anti-Stalemate (5s)
 */

// --- CONFIGURACIÓN DE COMBATE TÁCTICO (VELOCIDADES REDUCIDAS) ---
#define SEARCH_SPEED 100  // Velocidad de giro fluida para no saltarse al rival ni perder tracción
#define ATTACK_SPEED 140  // Velocidad de impacto reducida para evitar que la inercia nos saque del ring
#define ESCAPE_SPEED 140  // Velocidad de retroceso controlada para salvar el borde con seguridad
#define DIST_DETECCION 45 // (cm) Rango visual frontal
#define DIST_ATAQUE 20    // (cm) Distancia para activar embestida
#define TIEMPO_MEMORIA 25 // Ciclos de loop para la embestida ciega

#define VALOR_BORDE LOW   // LOW = Sensor detecta línea negra

// --- VARIABLES DE MEMORIA Y TEMPORIZADORES ---
int tiempoPerdido = 100;  
bool enemigoDetectado = false; 

// Temporizadores no bloqueantes para el comportamiento de búsqueda
unsigned long tiempoInicioBusqueda = 0;
bool modoAvanzarBusqueda = false;
unsigned long tiempoInicioAvance = 0;

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

// --- SENSOR ULTRASÓNICO (FIJO) ---
#define Echo_PIN 31 
#define Trig_PIN 30 

void setup() {
  // Configuración de pines de Motores
  pinMode(RightMotorDirPin1, OUTPUT); pinMode(RightMotorDirPin2, OUTPUT); pinMode(speedPinR, OUTPUT);
  pinMode(LeftMotorDirPin1, OUTPUT);  pinMode(LeftMotorDirPin2, OUTPUT);  pinMode(speedPinL, OUTPUT);
  pinMode(RightMotorDirPin1B, OUTPUT); pinMode(RightMotorDirPin2B, OUTPUT); pinMode(speedPinRB, OUTPUT);
  pinMode(LeftMotorDirPin1B, OUTPUT);  pinMode(LeftMotorDirPin2B, OUTPUT);  pinMode(speedPinLB, OUTPUT);

  // Configuración de pines de Sensores IR
  pinMode(sensor1, INPUT); pinMode(sensor2, INPUT); 
  pinMode(sensor3, INPUT); pinMode(sensor4, INPUT); pinMode(sensor5, INPUT);

  // Configuración de pines de Ultrasonido
  pinMode(Trig_PIN, OUTPUT);
  pinMode(Echo_PIN, INPUT);

  Serial.begin(9600);
  
  // --- REGLA OFICIAL ASTI: 5 SEGUNDOS DE ESPERA ---
  stop_Stop();
  delay(5000); 
  
  // Inicializar el tiempo de búsqueda justo al arrancar
  tiempoInicioBusqueda = millis();
}

void loop() {
  // 1. LEER SENSORES DE BORDE (MÁXIMA PRIORIDAD)
  bool s1_izq = (digitalRead(sensor1) == VALOR_BORDE); 
  bool s2_izq = (digitalRead(sensor2) == VALOR_BORDE);
  bool s3_cen = (digitalRead(sensor3) == VALOR_BORDE); 
  bool s4_der = (digitalRead(sensor4) == VALOR_BORDE);
  bool s5_der = (digitalRead(sensor5) == VALOR_BORDE);

  // 2. CONTROL DE SUPERVIVENCIA (Si toca la línea, se ejecuta inmediatamente)
  if (s1_izq || s2_izq || s3_cen || s4_der || s5_der) {
    // Identificar dirección de escape predominante
    if (s1_izq || s2_izq) escapar_borde(1); 
    else if (s4_der || s5_der) escapar_borde(2); 
    else escapar_borde(3); 
  }
  
  // 3. MODO COMBATE / BÚSQUEDA
  else {
    int distancia = watch();

    if (distancia > 0 && distancia < DIST_DETECCION) {
      // ¡ENEMIGO DETECTADO! 
      enemigoDetectado = true;
      tiempoPerdido = 0; 
      
      // Resetear estados de búsqueda pasiva ya que encontramos objetivo
      modoAvanzarBusqueda = false;
      tiempoInicioBusqueda = millis(); 

      if (distancia < DIST_ATAQUE) {
        set_Motorspeed(ATTACK_SPEED, ATTACK_SPEED, ATTACK_SPEED, ATTACK_SPEED); 
      } else {
        set_Motorspeed(SEARCH_SPEED + 30, SEARCH_SPEED + 30, SEARCH_SPEED + 30, SEARCH_SPEED + 30); 
      }
      go_Advance();
    } 
    else {
      // NO HAY DETECCIÓN DIRECTA
      
      if (enemigoDetectado && tiempoPerdido < TIEMPO_MEMORIA) {
        // EMBESTIDA CIEGA (Mantiene la trayectoria de ataque un instante)
        tiempoPerdido++;
        set_Motorspeed(ATTACK_SPEED, ATTACK_SPEED, ATTACK_SPEED, ATTACK_SPEED); 
        go_Advance();
        tiempoInicioBusqueda = millis(); // Mantener reseteado el clock de búsqueda estática
      } 
      else {
        // MODO BÚSQUEDA ACTIVA (Nadie a la vista)
        enemigoDetectado = false;
        
        if (modoAvanzarBusqueda) {
          // Sub-estado: Avanzar un poco para romper el bucle de giros (Máximo 500ms)
          if (millis() - tiempoInicioAvance < 500) {
            set_Motorspeed(SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED);
            go_Advance();
          } else {
            // Fin del avance de recolocación, volver a girar
            modoAvanzarBusqueda = false;
            tiempoInicioBusqueda = millis(); 
          }
        } 
        else {
          // Comportamiento por defecto: Girar sobre su propio eje buscando
          set_Motorspeed(SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED, SEARCH_SPEED);
          go_Right(); 
          
          // MEJORA: Si pasa 5 segundos (5000 ms) girando en el mismo sitio sin ver nada
          if (millis() - tiempoInicioBusqueda > 5000) {
            modoAvanzarBusqueda = true;
            tiempoInicioAvance = millis(); 
          }
        }
      }
    }
  }
}

// --- FUNCIONES TÁCTICAS ---

void escapar_borde(int direccion) {
  stop_Stop();
  delay(40); // Pequeña transición para proteger la transmisión
  
  // Marcha atrás de emergencia
  set_Motorspeed(ESCAPE_SPEED, ESCAPE_SPEED, ESCAPE_SPEED, ESCAPE_SPEED);
  go_Back();
  delay(350); // Tiempo físico de retroceso para alejarse del peligro

  // Giro de seguridad hacia el interior del ring
  if (direccion == 1) {
    go_Right(); delay(350);
  } else if (direccion == 2) {
    go_Left();  delay(350);
  } else {
    go_Right(); delay(500); 
  }
  
  // Reset completo de estados de combate y de hilos de tiempo
  enemigoDetectado = false;
  tiempoPerdido = 100; 
  modoAvanzarBusqueda = false;
  tiempoInicioBusqueda = millis(); 
  stop_Stop();
}

int watch() {
  digitalWrite(Trig_PIN, LOW); delayMicroseconds(2);
  digitalWrite(Trig_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(Trig_PIN, LOW);
  
  long duration = pulseIn(Echo_PIN, HIGH, 25000); // Timeout optimizado a 25ms
  delay(35); // Ventana de estabilización acústica indispensable para el sensor fijo
  
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

// --- DIRECCIONES DE MOTORIZACIÓN MECANUM ---

void set_Motorspeed(int LF, int RF, int LB, int RB) {
  analogWrite(speedPinL, LF);   analogWrite(speedPinR, RF);
  analogWrite(speedPinLB, LB);  analogWrite(speedPinRB, RB);
}

void go_Advance() {
  digitalWrite(RightMotorDirPin1, LOW);  digitalWrite(RightMotorDirPin2, HIGH);
  digitalWrite(LeftMotorDirPin1, LOW);   digitalWrite(LeftMotorDirPin2, HIGH);
  digitalWrite(RightMotorDirPin1B, LOW); digitalWrite(RightMotorDirPin2B, HIGH);
  digitalWrite(LeftMotorDirPin1B, LOW);  digitalWrite(LeftMotorDirPin2B, HIGH);
}

void go_Back() {
  digitalWrite(RightMotorDirPin1, HIGH);  digitalWrite(RightMotorDirPin2, LOW);
  digitalWrite(LeftMotorDirPin1, HIGH);   digitalWrite(LeftMotorDirPin2, LOW);
  digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B, HIGH);  digitalWrite(LeftMotorDirPin2B, LOW);
}

void go_Right() { 
  digitalWrite(RightMotorDirPin1, HIGH);  digitalWrite(RightMotorDirPin2, LOW);
  digitalWrite(LeftMotorDirPin1, LOW);    digitalWrite(LeftMotorDirPin2, HIGH);
  digitalWrite(RightMotorDirPin1B, HIGH); digitalWrite(RightMotorDirPin2B, LOW);
  digitalWrite(LeftMotorDirPin1B, LOW);   digitalWrite(LeftMotorDirPin2B, HIGH);
}

void go_Left() { 
  digitalWrite(RightMotorDirPin1, LOW);   digitalWrite(RightMotorDirPin2, HIGH);
  digitalWrite(LeftMotorDirPin1, HIGH);   digitalWrite(LeftMotorDirPin2, LOW);
  digitalWrite(RightMotorDirPin1B, LOW);  digitalWrite(RightMotorDirPin2B, HIGH);
  digitalWrite(LeftMotorDirPin1B, HIGH);  digitalWrite(LeftMotorDirPin2B, LOW);
}

void stop_Stop() {
  set_Motorspeed(0, 0, 0, 0);
}