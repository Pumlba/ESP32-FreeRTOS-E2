
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#define POT 34
#define LED 18
#define BUZZER 19
#define TEST_MODE 4
// #define TEST_MODE 3
// #define TEST_MODE 2
// #define TEST_MODE 0
// 0: Binh thuong
// 2: Mutex contention
// 3: Queue timeout
// 4: Du lieu loi

#define PERIOD 100

struct SensorData {
  float temp;
  bool valid;
};

enum State { NORMAL, ALARM, FAULT };

QueueHandle_t sensorQueue;
SemaphoreHandle_t serialMutex;

State state = NORMAL;
// dùng để chạy Ca1 - 0
// void logMsg(const char *msg) {
//   if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(20))) {
// #if TEST_MODE == 2
//     vTaskDelay(pdMS_TO_TICKS(5));
// #endif
//     Serial.println(msg);
//     xSemaphoreGive(serialMutex);
//   }
// }

void logMsg(const char *msg) {
  int64_t start = esp_timer_get_time();

  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(20))) {
    int64_t wait = esp_timer_get_time() - start;

    Serial.printf("[Mutex] Wait = %lld us | ", wait);
    Serial.println(msg);

    xSemaphoreGive(serialMutex);
  }
}


void output(bool on) {
  digitalWrite(LED, on);
  if (on) ledcWriteTone(BUZZER, 2000);
  else ledcWriteTone(BUZZER, 0);
}

void ReadSensor(void *pv) {
  TickType_t last = xTaskGetTickCount();
  int64_t previous = 0;
  int count = 0;

  while (true) {
#if TEST_MODE == 3
    if (esp_timer_get_time() >= 10000000)
      vTaskSuspend(NULL);
#endif

    int64_t start = esp_timer_get_time();

    int raw = analogRead(POT);
    SensorData data;
    data.temp = raw * 100.0 / 4095.0;
    data.valid = true;

#if TEST_MODE == 4
    if (++count == 50) data.valid = false;
#endif

    xQueueSend(sensorQueue, &data, 0);

    int64_t period = previous ? start - previous : 0;
    int64_t jitter = previous ? period - 100000 : 0;
    previous = start;

    char msg[180];
    snprintf(msg, sizeof(msg),
      "[ReadSensor] Temp=%.1f C | Period=%lld us | Jitter=%lld us | Execution=%lld us",
      data.temp, period, jitter,
      esp_timer_get_time() - start);

    logMsg(msg);

    vTaskDelayUntil(&last, pdMS_TO_TICKS(PERIOD));
  }
}

void Control(void *pv) {
  SensorData data;
// Sau // là dùng để chạy 0
  while (true) {
    if (xQueueReceive(sensorQueue, &data,
                      pdMS_TO_TICKS(300)) != pdTRUE) {
      state = FAULT;
      // logMsg("FAULT: Queue timeout");
      controlLog("FAULT: Queue timeout");
    
    }
    else if (!data.valid) {
      state = FAULT;
      // logMsg("FAULT: Invalid sensor data");
      controlLog("FAULT: Invalid sensor data");
    }
    else if (state != FAULT) {
      if (data.temp > 50) state = ALARM;
      else if (state == ALARM && data.temp > 45)
        state = ALARM;
      else state = NORMAL;

      if (state == NORMAL) {
        output(false);
        // logMsg("NORMAL: LED OFF, BUZZER OFF");
        controlLog("NORMAL: LED OFF, BUZZER OFF");
      }
      else {
        output(true);
        // logMsg("ALARM: LED ON, BUZZER ON");
        controlLog("ALARM: LED ON, BUZZER ON");
      }
    }
  }
}
// ca1-0 k cần dùng cái này , hãm này để chạy ca 
void controlLog(const char *msg) {
  int64_t start = esp_timer_get_time();

  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(20))) {
    int64_t wait = esp_timer_get_time() - start;

    // Control giu Mutex them 5ms
    vTaskDelay(pdMS_TO_TICKS(5));

    Serial.printf("[Control] Mutex Wait = %lld us | ",
                  (long long)wait);
    Serial.println(msg);

    xSemaphoreGive(serialMutex);
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(POT, INPUT);
  pinMode(LED, OUTPUT);

  ledcAttach(BUZZER, 2000, 10);
  output(false);

  sensorQueue = xQueueCreate(5, sizeof(SensorData));
  serialMutex = xSemaphoreCreateMutex();

  xTaskCreate(Control, "Control", 4096, NULL, 2, NULL);
  xTaskCreate(ReadSensor, "ReadSensor", 4096, NULL, 1, NULL);

  Serial.println("ESP32 FreeRTOS E2 Started");
}

void loop() {
  delay(100);
}
