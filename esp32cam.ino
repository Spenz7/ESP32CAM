#include <WiFi.h>
//http post method as images are larger in size
#include <HTTPClient.h>
#include "esp_camera.h"
//encoded using base64 to prevent possible conflict of data, will have to decode via node-red
#include "base64.h"

// WiFi credentials
const char *ssid = ""; 
const char *password = "";

// Server details
const char *serverUrl = "";

// Pin definition for the ESP32-CAM module
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Camera settings
const int JPEG_QUALITY = 2;  // JPEG quality (0-63)
//const framesize_t FRAME_SIZE = FRAMESIZE_UXGA;  // Frame size (e.g., FRAMESIZE_UXGA, FRAMESIZE_SXGA, etc.)
const framesize_t FRAME_SIZE = FRAMESIZE_SVGA;  // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA

// Interval for capturing and sending images (milliseconds)
const unsigned long captureInterval = ; 

// MAC address buffer
char macAddress[18];

void setup() {
  //baud rate aka speed of transfer of data
  Serial.begin(115200);
  connectToWiFi();
  initCamera();
}

void loop() {
  captureAndSendImage();
  delay(captureInterval);
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

void initCamera() {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAME_SIZE;
  config.jpeg_quality = JPEG_QUALITY;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void captureAndSendImage() {
  Serial.println("Capturing image...");
  HTTPClient http;
  http.begin(serverUrl);

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to capture image");
    http.end();
    return;
  }

  // Encode the image to base64
  String base64Image = base64::encode(fb->buf, fb->len);

// Get MAC address of ESP32-CAM
  String camID = WiFi.macAddress();
  camID.toCharArray(macAddress, 18);

  // Prepare JSON payload
  String jsonPayload = "{\"camID\":\"" + String(macAddress) + "\",\"image\":\"" + base64Image + "\"}";

  // Send the JSON payload in the HTTP POST request
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  esp_camera_fb_return(fb);
  http.end();
}
