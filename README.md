# Robot Mini Sumo · Osoyoo M2 Mecanum

Programas en Arduino (C++) para un robot **Osoyoo M2** con ruedas mecanum, preparados para el **ASTI Robotics Challenge**. Incluye dos estrategias de mini sumo, un siguelíneas, trazado de figuras geométricas y control por WiFi.

## Hardware

| Componente | Uso |
|---|---|
| Arduino Mega + chasis Osoyoo M2 con 4 ruedas mecanum | Cuatro motores con velocidad y sentido independientes |
| 5 sensores infrarrojos de línea (A0–A4) | Detectar el borde negro del ring o la línea a seguir |
| Sensor de ultrasonidos (pines 30 y 31) | Medir la distancia al rival |
| Servo (pin 13) | Girar el sensor de ultrasonidos como un radar (solo en `sumoVdefinitiva`) |
| Módulo WiFi ESP8266 | Control remoto desde el móvil (`controlWifi`) |

## Programas

| Archivo | Qué hace |
|---|---|
| [`sumoV4.0.ino`](sumoV4.0.ino) | Mini sumo con sensor fijo: busca, embiste y se mantiene dentro del ring |
| [`sumoVdefinitiva.ino`](sumoVdefinitiva.ino) | Mini sumo con radar sobre servo y fijación del objetivo |
| [`siguelineasV3.ino`](siguelineasV3.ino) | Siguelíneas suave con memoria del último lado |
| [`cuadrado20x20.ino`](cuadrado20x20.ino), [`rectangulo30X10.ino`](rectangulo30X10.ino), [`trinangulo25x25.ino`](trinangulo25x25.ino) | Trazado de figuras con desplazamientos calibrados |
| [`controlWifi.ino`](controlWifi.ino) | Control por WiFi desde la app de Osoyoo (código base del fabricante) |
| [`robot.ino`](robot.ino) | Siguelíneas de ejemplo de Osoyoo con mis anotaciones (código base del fabricante) |

### Mini sumo · `sumoV4.0.ino`

El `loop()` funciona como una máquina de estados con prioridades:

1. **Supervivencia.** Si cualquiera de los 5 sensores IR ve el borde negro, el robot frena, retrocede 350 ms y gira hacia el interior del ring. El sentido del giro depende de qué lado ha tocado la línea.
2. **Ataque.** Si el ultrasonidos detecta al rival a menos de 45 cm, avanza hacia él, y por debajo de 20 cm pasa a velocidad de embestida.
3. **Embestida ciega.** Si pierde al rival justo después de verlo (por ejemplo, cuando ya está pegado y entra en el punto ciego del sensor), mantiene el ataque durante 25 ciclos más.
4. **Búsqueda.** Gira sobre sí mismo buscando. Si pasa 5 segundos girando sin ver nada, avanza medio segundo para cambiar de posición y no quedarse atascado.

Cumple la regla del reglamento de esperar 5 segundos parado al empezar. Las velocidades están reducidas a propósito para no salir del ring por inercia, y los temporizadores usan `millis()` para no bloquear la lectura de sensores.

### Mini sumo con radar · `sumoVdefinitiva.ino`

Monta el ultrasonidos sobre un servo que barre de 40° a 140°.

- Cuando ve al rival, guarda **en qué lado lo ha visto** (izquierda, frente o derecha), centra el sensor y gira el cuerpo hasta tenerlo de frente.
- Con el rival delante a menos de 40 cm, ataca a máxima potencia.
- La memoria de embestida ciega dura 40 ciclos si el rival estaba de frente y 15 si estaba a un lado.

### Siguelíneas · `siguelineasV3.ino`

Codifica los 5 sensores IR en un número de 5 bits (`0b00100` = línea centrada) y decide con un `switch`:

- Línea centrada: avance a máxima velocidad.
- Desviación leve: el motor del lado interior frena sin pararse del todo, para que las curvas sean suaves y el robot no vibre.
- Desviación fuerte: giro cerrado.
- Línea perdida: gira hacia el **último lado** donde la vio. Si no lo sabe, retrocede un poco.

### Figuras geométricas

El robot no tiene encoders, así que mide por tiempo: se calibró que avanza 1 cm en 75 ms y gira 1° en 6 ms. Con eso, `avanzar(cm)` y `girarIzquierda(grados)` trazan figuras sobre el papel, con pausas entre tramos para estabilizar el chasis.

## Cómo cargarlo

1. Instala el [Arduino IDE](https://www.arduino.cc/en/software).
2. Abre el `.ino` que quieras. El IDE pedirá moverlo a una carpeta con su mismo nombre: acepta.
3. Selecciona la placa **Arduino Mega 2560** y el puerto, y pulsa **Subir**.
4. `sumoVdefinitiva.ino` usa la librería `Servo`, que ya viene con el IDE.

## Créditos

`controlWifi.ino` y `robot.ino` parten de los tutoriales de [Osoyoo](https://osoyoo.com/?p=49235) para este chasis. Los programas de sumo, el siguelíneas y las figuras son míos.

---

Adrián Nasri Escarti · [Portfolio y otros proyectos](https://github.com/adrian1glitch)
