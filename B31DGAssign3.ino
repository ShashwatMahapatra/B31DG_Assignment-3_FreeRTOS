
//Define Libraries for FreeRTOS and Arduino
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>



//**Declaring Pins for Task 1**
int led_1 = 32;    //Output Pin for Signal LED in Task1

//**Declaring Pins & variables for Task2**
int in    = 15;     //Pin for Frequency input for Task 2
int hi;            //integer for storing time for which wave is high for task 2
int lo;            //integer for storing time for which wave is low for task 2
int total;         //integer for storing total time of one cycle for task 2
int fr;            //integer for storing frequency of wave for task 2

//**Declaring Pins & variables for Task 3**
int in1   = 2;    //Pin for Frequency input for Task 3
int hi1;           //integer for storing time for which wave is high for task 3
int lo1;           //integer for storing time for which wave is low for task 3
int fr1;           //integer for storing frequency of wave for task 3

//**Declaring Pins and global variables for Task 4**
int led_2 = 14;    //Output Pin for Error LED
int p     = 4;     //Input Pin for Potentiometer input
int v0;            //integer for storing Voltage reading from Potentiometer
int v1;            //integer for storing Voltage reading from Potentiometer
int v2;            //integer for storing Voltage reading from Potentiometer
int v3;            //integer for storing Voltage reading from Potentiometer
unsigned long avg; //integer for storing average of last 4 voltage readings
int val;           //Analog Value input from the Potentiometer
int volt;          //integer value for analog value converted to voltage

//**Declaring Pins & variables for Task 5**
int feqmin = 333;  //integer for storing Minimum reference frequency for Task2 
int feqmax = 1000; //integer for storing Maximum reference frequency for Task2 
int feqmin1= 500;  //integer for storing Minimum reference frequency for Task3
int feqmax1= 1000; //integer for storing Maximum reference frequency for Task3  
int a;             //integer for storing fscale value of Task2 in loop for Task5  
int a1;            //integer for storing fscale1 value of Task3 in loop for Task5

//**Declaring Pins for Button Debouncing task**
#define BUTTON_PIN 14
#define LED_PIN 27

//**Declaring task handles and Queue for Button debouncing task**
TaskHandle_t monitorInputTaskHandle;
TaskHandle_t controlLEDTaskHandle;
TaskHandle_t Task1;
QueueHandle_t eventQueue;
//Let Initial Condition for LED state be False
bool ledOn = false;

//**Declaring Semaphore for communication between Tasks 2,3 and 5**
SemaphoreHandle_t FrequencySem;

//**Declaring structured data for values to be stored in by the Semaphore**
typedef struct {
  int fre_t2;      //Frequency reading from Task 2
  int fre_t3;      //Frequency reading from Task 3
} Frequen;
Frequen freqs;



void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    pinMode(led_1, OUTPUT);
  
    pinMode(in, INPUT);
    pinMode(in1, INPUT);
  
  Serial.begin(9600);
  
  //Initialising Queue for Button debouncing, Queue can carry 5 8-bit integer values)
  eventQueue = xQueueCreate(5, sizeof(uint8_t));                                                         

  //Initialise Frequency values in structured data to 0 as it is required for Semaphore intialisation conditions
   freqs.fre_t2 = 0;
   freqs.fre_t3 = 0;

  //Initialise Semaphore for communication between Tasks 2, 3 and 5
   FrequencySem = xSemaphoreCreateMutex();
  
  //Initialsing Tasks in FreeRTOS syntax with assigned priorites, stack sizes and core designation. All Tasks are running on the same core
   /*xTaskCreatePinnedToCore(t1, 
                          "tak", //Task handle
                           1024,      //Stack Size
                           NULL,      //Pointer, Null designation
                           1,         //Priority assignment, alloted as per frequency of task in hyperperiod
                           &Task1,      //Pointer, Null designation
                           1);        //ESP 32 Core number.
   */
   xTaskCreatePinnedToCore(t2, "Monitor Frequency2", 1024, NULL, 1, NULL,1);
   xTaskCreatePinnedToCore(t3, "Monitor Frequency3", 1024, NULL, 2, NULL,1);
   //xTaskCreatePinnedToCore(t4, "Monitor Voltage", 1024, NULL, 1, NULL,1);
   xTaskCreatePinnedToCore(t5, "Record Freq", 2048, NULL, 1, NULL,1);
   xTaskCreatePinnedToCore(monitorInputTask, "Monitor Input Task", 2048, NULL, 1, &monitorInputTaskHandle,1);
   xTaskCreatePinnedToCore(controlLEDTask, "Control LED Task", 2048, NULL, 1, &controlLEDTaskHandle,1);

}

void t1(void * parameter) {
   while(1){
   
   digitalWrite(led_1, HIGH);
   delayMicroseconds(200);
   digitalWrite(led_1,LOW);
   delayMicroseconds(50);
   digitalWrite(led_1, HIGH);
   delayMicroseconds(30);
   digitalWrite(led_1,LOW);
  
   vTaskDelay(20 / portTICK_PERIOD_MS);
   }
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

    //Semaphore initialised, Semaphore will record frequency value if there are values present in the structured data
    if(xSemaphoreTake(FrequencySem, portMAX_DELAY) == pdTRUE){
      //declaring pointer to shared variable of structured data to store task 2 frequency.
      freqs.fre_t2 = fr;
      
      //initialise semaphore prompt to return and store new value in the data structure
      xSemaphoreGive(FrequencySem);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
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
    vTaskDelay(8 / portTICK_PERIOD_MS);
    }
}

void t4(void * parameter) {
  while(1){
  val    = analogRead(p);                       //analogRead() takes input from pin in analog and stores it as such in the microcontroller
  volt   = val * (3.3/4095.0);                  //convert analog reading which is in the range of 0 t0 4025 to a voltage between 0 to 3.3V
                       
  v0     = v1;
  v1     = v2;
  v2     = v3; 
  v3     = volt;

  avg    = (v0 + v1 + v2 + v3)/4;
  
  if (avg > 1.65){
    digitalWrite(led_2, HIGH);
    delay(25);
    digitalWrite(led_2, LOW);
    delay(25);
    
  }
  vTaskDelay(20 / portTICK_PERIOD_MS);
  //Serial.printf("Input Voltage avg(in V): %d \n",avg);
}
}

void t5(void * parameter) {
  
  while(1){
    
    //Here the Semaphore is initialised o take value from the Data structure to the critical region of Task 5
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
   vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  
 
}

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

//Loop left untouched
void loop() {}
