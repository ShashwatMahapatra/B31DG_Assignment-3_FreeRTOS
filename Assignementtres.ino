#include<FreeRTOS.h>
#include<task.h>
#include<queue.h>
#include<timers.h>
#include<semphr.h>
#include<B31DGMonitor.h>
#include<Ticker.h>
#include<stdio.h>

int led_1 = 12;    //Output Pin for Signal LED in Task1
int led_2 = 14;    //Output Pin for Error LED
int p     = 4;     //Input Pin for Potentiometer input

int in    = 15;     //Pin for Frequency input for Task 2
int hi;            //integer for storing time for which wave is high for task 2
int lo;            //integer for storing time for which wave is low for task 2
int total;         //integer for storing total time of one cycle for task 2
int fr;            //integer for storing frequency of wave for task 2

int in1   = 2;    //Pin for Frequency input for Task 3
int hi1;           //integer for storing time for which wave is high for task 3
int lo1;           //integer for storing time for which wave is low for task 3
int total1;        //integer for storing total time of one cycle for task 3
int fr1;           //integer for storing frequency of wave for task 3

int fscale;        //integer for storing scaled frequency value for Task2
int feqmin = 333;  //integer for storing Minimum reference frequency for Task2 
int feqmax = 1000; //integer for storing Maximum reference frequency for Task2
int fscale1;       //integer for storing scaled frequency value for Task3 
int feqmin1= 500;  //integer for storing Minimum reference frequency for Task3
int feqmax1= 1000; //integer for storing Maximum reference frequency for Task3  
int a;             //integer for storing fscale value of Task2 in loop for Task5  
int a1;            //integer for storing fscale1 value of Task3 in loop for Task5 

int m0;            //integer for storing milestone (initial)
int m1;            //integer for storing milestone (final)
int m;             //Time taken for Task 1 to complete

int v0;            //integer for storing Voltage reading from Potentiometer
int v1;            //integer for storing Voltage reading from Potentiometer
int v2;            //integer for storing Voltage reading from Potentiometer
int v3;            //integer for storing Voltage reading from Potentiometer
unsigned long avg; //integer for storing average of last 4 voltage readings

int val;                             //Analog Value input from the Potentiometer
int volt;                            //integer value for analog value converted to voltage

B31DGCyclicExecutiveMonitor monitor; //initialise global variable for JobMonitoring with monitor as the variable name
Ticker Cycle;                        //initialise global variable for Ticker with Cycle as the variable name 
#define Frame_dura_ms  4             //define the duration of one frame as 4 Milliseconds
int fraCount = 0;                    //integer for storing Frame count, lets Micro-controller know which frame to execute. 
int section  = 0;                     //intege for storing case count, lets Micro-controller know which case to execute in frame.

/*void frame(){
  switch (section) {
        case 0:   t1();   t3();   t4();   t5();
    break;
        case 1:   t1();   t2();
    break;
        case 2:   t1();   t3();   
    break;
        case 3:   t1();
    break;
        case 4:   t1();   t3(); 
    break;
        case 5:   t1();   t4();
    break;
        case 6:   t1();   t3();
    break;
        case 7:   t1();   t2(); 
    break;
        case 8:   t1();   t3();
    break;
        case 9:   t1();
    break;
        case 10:  t1();   t3();   t4();
    break;
        case 11:  t1();   t2();  
    break;
        case 12:  t1();   t3();
    break;
        case 13:  t1();
    break;
        case 14:  t1();   t3();
    break;
        case 15:  t1();   t4();
    break;
        case 16:  t1();   t3();
    break;    
        case 17:  t1();   t2();
    break;
        case 18:  t1();   t3();
    break;
        case 19:  t1();   
    break;
        case 20:  t1();   t3();
    break;
        case 21:  t1();   t2();   t4();
    break;
        case 22:  t1();   t3();
    break;
        case 23:  t1();   
    break;
        case 24:  t1();   t3();
    break;
        case 25:  t1();   t4();   t5();
    break;
        case 26:  t1();   t3();
    break;
        case 27:  t1();   t2();
    break;
        case 28:  t1();   t3();
    break;
        case 29:  t1();
    break;
        case 30:  t1();   t3();   t4();
    break;
        case 31:  t1();   t2();
    break;
        case 32:  t1();   t3();
    break;
        case 33:  t1();
    break;
        case 34:  t1();   t3();   
    break;
        case 35:  t1();   t4();
    break;
        case 36:  t1();   t3();
    break;
        case 37:  t1();   t2();
    break;
        case 38:  t1();   t3();
    break;
        case 39:  t1();   
    break;
        case 40:  t1();   t3();   t4();
    break;
        case 41:  t1();   t2();
    break;
        case 42:  t1();   t3();
    break;
        case 43:  t1();   
    break;
        case 44:  t1();   t3();
    break;
        case 45:  t1();   t4();
    break;
        case 46:  t1();   t3();
    break;
        case 47:  t1();   t2();
    break;
        case 48:  t1();   t3();
    break;
        case 49:  t1();   
    break;
        //case 50:  t1();   t3();   t4();   t5();
    //break;
    
  }
  fraCount ++;                             //Increments Frame counter which is later used to determined remaining number of frames
  section = fraCount % 50;                 //Determines no. of remaining frames by taking the remainder, so microcontroller determines specific frame to execute in succession
}
*/

void setup(void) {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  digitalWrite(led_2,LOW);
  pinMode(in , INPUT);
  pinMode(in1, INPUT);
  monitor.startMonitoring();
  frame();
  Cycle.attach_ms(Frame_dura_ms,frame);
  xTaskCreate(&t1,"t1",1024,NULL,1,NULL,ARDDUINO_RUNNING CORE);
  xTaskCreate(&t2,"t2",1024,NULL,1,NULL,ARDDUINO_RUNNING CORE);
  xTaskCreate(&t3,"t3",1024,NULL,1,NULL,ARDDUINO_RUNNING CORE);
  xTaskCreate(&t4,"t4",1024,NULL,1,NULL,ARDDUINO_RUNNING CORE);
  xTaskCreate(&t5,"t5",1024,NULL,1,NULL,ARDDUINO_RUNNING CORE);
  

}

void loop(void) {
  // put your main code here, to run repeatedly:
  //uncomment this to find execution time of given tasks 
  /*unsigned long bT = micros();
  t1();
  unsigned long t = (micros()-bT);
  Serial.print("Duration JobTask1 Job = ");
  Serial.println(t);

  bT = micros();
  t2();
  t = (micros()-bT);
  Serial.print("Duration JobTask2 Job = ");
  Serial.println(t);

  bT = micros();
  t3();
  t = (micros()-bT);
  Serial.print("Duration JobTask3 Job = ");
  Serial.println(t);

  bT = micros();
  t4();
  t = (micros()-bT);
  Serial.print("Duration JobTask4 Job = ");
  Serial.println(t);

  bT = micros();
  t5();
  t = (micros()-bT);
  Serial.print("Duration JobTask5 Job = ");
  Serial.println(t);
  */
 
   
}

voidt0(void *pvParameters){
  
  }

void t1(void *pvParameters) {
   monitor.jobStarted(1);
   digitalWrite(led_1, HIGH);
   delayMicroseconds(200);
   digitalWrite(led_1,LOW);
   delayMicroseconds(50);
   digitalWrite(led_1, HIGH);
   delayMicroseconds(30);
   digitalWrite(led_1,LOW);
   monitor.jobEnded(1);
   vTaskDelay(50);
}

void t2(void *pvParameters) {
  monitor.jobStarted(2);
  digitalRead(in);
  hi     = 0;
  hi     = pulseIn(in,HIGH, 3000);
  if (hi > 0){
    fr   = 1000000/(2*hi);
  }
  else{
    fr   = 333;   
  }
  monitor.jobEnded(2); 
  vTaskDelay(50);
}

void t3(void *pvParameters) {
  monitor.jobStarted(3);
  digitalRead(in1);
  hi1     = 0;
  hi1     = pulseIn(in1,HIGH, 3000);
  if (hi1 > 0){
    fr1   = 1000000/(2*hi1);
  }
  else{
    fr1   = 500;   
  }
  monitor.jobEnded(3);
  vTaskDelay(50);

}

void t4(void *pvParameters) {
  monitor.jobStarted(4);
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
  monitor.jobEnded(4);
  vTaskDelay(50);
}

void t5(void *pvParameters) {
  
  monitor.jobStarted(5);
  fscale = (99*(fr - feqmin))/(feqmax - feqmin) ;
  a = fscale;
  if (fscale <= 0){
    a = 0;
  }

  if(fscale>99){
    a = 99;
  }
  //else {
    //a = 99;
  //}

  fscale1 = (99*(fr1 - feqmin1))/(feqmax1 - feqmin1) ;
  a1 = fscale1;
  if (fscale1 <= 0){
    a1= 0;
  }
  
  if(fscale1>99){
    a1 = 99;
  }
  //else{
    //a1= 99;
  //}

 //Serial.print(a);
 //Serial.print(',');
 //Serial.println(a1);
 Serial.printf(" %d , %d \n", a, a1);
 monitor.jobEnded(5);
 vTaskDelay(50);

}
