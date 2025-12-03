#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"

// ===========================
// WiFi
// ===========================
const char *ssid = "SENAC-Mesh";
const char *password = "09080706";

// ===========================
// URL do servidor Python
// ===========================
const char* serverURL = "https://esp32-camera-server-pks3.onrender.com";  
// → Exemplo: Flask recebendo via /upload

void startCameraServer();
void setupLedFlash();

unsigned long lastSend = 0;
const unsigned long INTERVALO_ENVIO = 15000; // 15 segundos

// ===========================
// ENVIO DA FOTO PARA O SERVIDOR
// ===========================
void sendPhotoToServer() {
  Serial.println("\n📸 Capturando imagem...");

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Erro ao capturar imagem!");
    return;
  }

  Serial.printf("📦 Tamanho da imagem: %u bytes\n", fb->len);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi desconectado!");
    esp_camera_fb_return(fb);
    return;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "image/jpeg");
  http.setTimeout(7000);

  Serial.println("📤 Enviando imagem para servidor...");

  int httpResponseCode = http.POST(fb->buf, fb->len);

  if (httpResponseCode > 0) {
    Serial.printf("✅ Servidor respondeu: %d\n", httpResponseCode);
    Serial.println(http.getString());
  } else {
    Serial.printf("❌ Erro no envio: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  esp_camera_fb_return(fb);
}

// ===========================
// SETUP
// ===========================
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  Serial.println("🎥 Inicializando câmera...");

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Falha ao inicializar câmera!");
    return;
  }

  Serial.println("📶 Conectando ao WiFi...");

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\n✅ WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  startCameraServer();
  Serial.println("🌐 Servidor da câmera iniciado!");
}

// ===========================
// LOOP PRINCIPAL
// ===========================
void loop() {
  unsigned long now = millis();

  if (now - lastSend >= INTERVALO_ENVIO) {
    lastSend = now;
    sendPhotoToServer();
  }

  delay(10);
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}
