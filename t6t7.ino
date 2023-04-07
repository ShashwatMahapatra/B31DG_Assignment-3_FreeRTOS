
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>



#define BUTTON_PIN 27
#define LED_PIN 16

TaskHandle_t monitorInputTaskHandle;
TaskHandle_t controlLEDTaskHandle;
QueueHandle_t eventQueue;

bool ledOn = false;

void monitorInputTask(void* parameter) {
  bool lastButtonState = false;
  while (1) {
    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
      delay(5);
      buttonState = digitalRead(BUTTON_PIN);
    }
    if (buttonState != lastButtonState) {
      lastButtonState = buttonState;
      if (buttonState == LOW) {
        // Button pressed, send event to control LED task
        uint8_t event = 1;
        xQueueSend(eventQueue, &event, portMAX_DELAY);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Poll every 10 ms
  }
}

void controlLEDTask(void* parameter) {
  uint8_t event;
  while (1) {
    if (xQueueReceive(eventQueue, &event, portMAX_DELAY) == pdPASS) {
      // Toggle the LED state
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn);
    }
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);

  eventQueue = xQueueCreate(5, sizeof(uint8_t));

  xTaskCreate(monitorInputTask, "Monitor Input Task", 2048, NULL, 1, &monitorInputTaskHandle);
  xTaskCreate(controlLEDTask, "Control LED Task", 2048, NULL, 1, &controlLEDTaskHandle);
}

void loop() {
  // Nothing to do here as all work is done in tasks
}