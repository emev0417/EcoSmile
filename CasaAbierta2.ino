#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include <Keypad.h>
#include <Servo.h> 

// --- CONFIGURACIÓN DISPLAY 20x4 I2C ---
// Dirección común 0x27. Si no enciende, prueba con 0x3F.
const int LCD_COLUMNAS = 20; 
const int LCD_FILAS = 4;
LiquidCrystal_I2C lcd(0x27, LCD_COLUMNAS, LCD_FILAS); 

// --- CONFIGURACIÓN SERVO ---
Servo tapaServo;
const int pinServo = 5;

// --- CONFIGURACIÓN TECLADO ---
const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'}, {'4','5','6','B'},
  {'7','8','9','C'}, {'*','0','#','D'}
};
byte pinesFilas[FILAS] = {28, 29, 30, 31};    
byte pinesColumnas[COLUMNAS] = {32, 33, 34, 35}; 
Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// --- CONFIGURACIÓN SENSOR Y VARIABLES ---
const int pinSensor = 2;   
int contadorBotellas = 0;  
bool objetoDetectado = false; 
String inputID = "";

// --- PROTOTIPOS DE FUNCIONES ---
void mostrarInicio();
void validarUsuario();
bool esRespuestaValida(String res);
void iniciarSesionReciclaje(String nombre);
void finalizarYEnviar();

void setup() {
  Serial.begin(9600);    // Monitor Serie PC
  Serial1.begin(9600);   // Comunicación con ESP32 (TX1/RX1)
  
  pinMode(pinSensor, INPUT); 
  tapaServo.attach(pinServo);    
  tapaServo.write(5);    // Inicia cerrado
  
  // Inicialización del nuevo Display I2C
  lcd.init();            
  lcd.backlight();       
  
  mostrarInicio();
  Serial.println("SISTEMA MEGA V5 - LISTO");
}

void loop() {
  char tecla = teclado.getKey();

  if (tecla) {
    if (tecla >= '0' && tecla <= '9') {
      inputID += tecla;
      lcd.setCursor(0, 1);
      lcd.print(inputID);
    } 
    else if (tecla == 'C') { 
      mostrarInicio();
    } 
    else if (tecla == 'B') { 
      if (inputID.length() > 0) {
        validarUsuario();
      }
    }
  }
}

void mostrarInicio() {
  inputID = "";
  contadorBotellas = 0; 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INGRESE ID:");
  Serial.println("Esperando ID...");
}

void validarUsuario() {
  while(Serial1.available() > 0) { Serial1.read(); }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Buscando...");
  Serial1.println(inputID); 

  unsigned long tiempoInicio = millis();
  bool respuestaRecibida = false;
  String nombreRespuesta = "";

  while (millis() - tiempoInicio < 8000) {
    if (Serial1.available() > 0) {
      delay(150); 
      nombreRespuesta = Serial1.readStringUntil('\n');
      nombreRespuesta.trim();
      if (nombreRespuesta.length() > 0) {
        respuestaRecibida = true;
        break; 
      }
    }
  }

  if (respuestaRecibida && esRespuestaValida(nombreRespuesta)) {
    iniciarSesionReciclaje(nombreRespuesta);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ID NO VALIDO");
    delay(3000);
    mostrarInicio();
  }
}

bool esRespuestaValida(String res) {
  res.toLowerCase();
  if (res.indexOf("incorrecto") == -1 && 
      res.indexOf("error") == -1 && 
      res.indexOf("no encontrado") == -1) {
    return true;
  }
  return false;
}

void iniciarSesionReciclaje(String nombre) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hola " + nombre);
  tapaServo.write(90); // Abrir tapa
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOTELLAS: 0");
  lcd.setCursor(0, 1);
  lcd.print("C: TERMINAR");

  while(true) {
    char t = teclado.getKey();
    if (t == 'C') break; 

    if (digitalRead(pinSensor) == LOW && !objetoDetectado) {
      contadorBotellas++;
      objetoDetectado = true; 
      
      lcd.setCursor(10, 0);
      lcd.print(contadorBotellas);
      Serial.print("Botella: ");
      Serial.println(contadorBotellas);
      delay(100); 
    }
    
    if (digitalRead(pinSensor) == HIGH) {
      objetoDetectado = false; 
    }
  }

  finalizarYEnviar();
}

void finalizarYEnviar() {
  tapaServo.write(0); 
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enviando datos...");

  Serial1.print("ENVIAR,");
  Serial1.print(inputID);
  Serial1.print(",");
  Serial1.println(contadorBotellas);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SESION FINALIZADA");
  lcd.setCursor(0, 1);
  lcd.print("TOTAL: ");
  lcd.print(contadorBotellas);
  
  delay(4000);
  mostrarInicio();
}
