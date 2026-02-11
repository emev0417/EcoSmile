#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "NETLIFE_GABRIEL_2P"; 
const char* password = "1800363812MGBC";

// URL 1: Consulta
String urlConsulta = "https://script.google.com/macros/s/AKfycbya0KpUc-ta6cKGWNbRmdWkqrmYoQQAfFiB4FJwdeCLv_6Q1HYgIpZqoEWFpkT56WE3nQ/exec?pass=1234ABCD&id=";

// URL 2: Registro (CORREGIDA CON EL ?)
String urlRegistro = "https://script.google.com/macros/s/AKfycbzeUPeknXYFZmyd_a_Vaew1v2_NZ2lNdn0lUyt50tiZU2OxcuoWjsvPrAcnLqHoyd1RWQ/exec?";

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); 

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi OK");
}

void loop() {
  if (Serial2.available()) {
    String datosRecibidos = Serial2.readStringUntil('\n');
    datosRecibidos.trim();
    
    if (datosRecibidos.length() > 0) {
      if (datosRecibidos.startsWith("ENVIAR,")) {
        procesarRegistro(datosRecibidos);
      } 
      else {
        consultarNombre(datosRecibidos);
      }
    }
  }
}

void consultarNombre(String id) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String urlFinal = urlConsulta + id;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String respuesta = http.getString();
      Serial2.println(respuesta); 
    }
    http.end();
  }
}

void procesarRegistro(String trama) {
  // Limpieza de la trama para evitar errores de URL
  trama.replace("\r", "");
  trama.replace("\n", "");

  int primeraComa = trama.indexOf(',');
  int segundaComa = trama.indexOf(',', primeraComa + 1);
  
  String id = trama.substring(primeraComa + 1, segundaComa);
  String cantidad = trama.substring(segundaComa + 1);
  
  id.trim();
  cantidad.trim();

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Construcción de URL robusta
    // Si tu script requiere pass, úsalo: urlRegistro + "pass=1234ABCD&id=" + id...
    String urlFinal = urlRegistro + "pass=ABCD123&id=" + id + "&contador=" + cantidad;
    
    Serial.println("--- ENVIANDO REGISTRO ---");
    Serial.println("URL: " + urlFinal);
    
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String respuesta = http.getString();
      Serial.println("Respuesta de Google: " + respuesta);
    } else {
      Serial.println("Error HTTP: " + String(httpCode));
    }
    http.end();
  }
}
