#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

int in    = 15;     //Pin for Frequency input for Task 2
int hi;            //integer for storing time for which wave is high for task 2
int lo;            //integer for storing time for which wave is low for task 2
int total;         //integer for storing total time of one cycle for task 2
int fr;            //integer for storing frequency of wave for task 2

int in1   = 2;    //Pin for Frequency input for Task 3
int hi1;           //integer for storing time for which wave is high for task 3
int lo1;           //integer for storing time for which wave is low for task 3
int fr1;           //integer for storing frequency of wave for task 3

int feqmin = 333;  //integer for storing Minimum reference frequency for Task2 
int feqmax = 1000; //integer for storing Maximum reference frequency for Task2
 
int feqmin1= 500;  //integer for storing Minimum reference frequency for Task3
int feqmax1= 1000; //integer for storing Maximum reference frequency for Task3  
int a;             //integer for storing fscale value of Task2 in loop for Task5  
int a1;            //integer for storing fscale1 value of Task3 in loop for Task5 

//TaskHandle_t Frequency2;
//askHandle_t Frequency3;
//QueueHandle_t FrequencyQueue;
SemaphoreHandle_t FrequencySem;
typedef struct {
  int fre_t2;
  int fre_t3;
} Frequen;

Frequen freqs;



void setup() {
  // put your setup code here, to run once:
  
   pinMode(in, INPUT);
   pinMode(in1, INPUT);
  
  Serial.begin(9600);

  freqs.fre_t2 = 0;
  freqs.fre_t3 = 0;

  FrequencySem = xSemaphoreCreateMutex();

   xTaskCreatePinnedToCore(t2, "Monitor Frequency2", 1024, NULL, 1, NULL,1);
   xTaskCreatePinnedToCore(t3, "Monitor Frequency3", 1024, NULL, 1, NULL,1);
   xTaskCreatePinnedToCore(t5, "Monitor Frequency3", 4096, NULL, 1, NULL,1);

}

void t2(void * parameter) {
    
  while(1){
    
    digitalRead(in);
    hi     = 0;
    hi     = pulseIn(in,HIGH, 3000);
    if (hi > 0){
      fr   = 1000000/(2*hi);
    }
    else{
      fr   = 333;   
    }
    if(xSemaphoreTake(FrequencySem, portMAX_DELAY) == pdTRUE){
      freqs.fre_t2 = fr;
      xSemaphoreGive(FrequencySem);
    }
    vTaskDelay(4 / portTICK_PERIOD_MS);
  }
}

void t3(void * parameter) {
  
  while(1){
    
    digitalRead(in1);
    hi1     = 0;
    hi1     = pulseIn(in1,HIGH, 3000);
    if (hi1 > 0){
        fr1   = 1000000/(2*hi1);
      }
      else{
        fr1   = 500;   
     }
    if(xSemaphoreTake(FrequencySem, portMAX_DELAY) == pdTRUE){
      freqs.fre_t3 = fr1;
      xSemaphoreGive(FrequencySem);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void t5(void * parameter) {
  
  while(1){
    
    if (xSemaphoreTake(FrequencySem, portMAX_DELAY) == pdTRUE) 
    {
      
      int fscale = (99*(fr - feqmin))/(feqmax - feqmin) ;
      a = fscale;
        if (fscale <= 0){
          a = 0;
          }

        if(fscale>99){
          a = 99;
        }
    };

  
    int fscale1 = (99*(fr1 - feqmin1))/(feqmax1 - feqmin1) ;
    a1 = fscale1;
    if (fscale1 <= 0){
      a1= 0;
    }
  
    if(fscale1>99){
      a1 = 99;
    }
  Serial.printf(" %d , %d \n", a, a1);
  xSemaphoreGive(FrequencySem);
   vTaskDelay(8 / portTICK_PERIOD_MS);
  }
  
 
}




void loop() {
  // put your main code here, to run repeatedly:

}
