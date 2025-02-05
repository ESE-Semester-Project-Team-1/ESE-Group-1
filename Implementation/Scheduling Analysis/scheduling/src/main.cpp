#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define MAX 255
#define LED_PIN 6

#define TEST 1

// Declare a mutex handle
SemaphoreHandle_t xMutexSensorMem;
SemaphoreHandle_t xMutexActuatorMem;

const int sensorPin = A0;
int sensorMem = 0;
int actuatorMem = 0;
int testing = 1;

void actuator_job();

void sensorJob();

void controlJob();

void testWCETActuatorJob();

void testWCETSensorJob();

void testWCETControlJob();


void sensorHandler(void *pvParameters){
  while(true){
    sensorJob();
    vTaskDelay(92);
  }
}

void actuatorHandler(void *pvParameters){
  for(;;){
    actuator_job();
    vTaskDelay(187);
  }

}

void controlUnit(void *pvParameters){
  while(true){
      controlJob();
      vTaskDelay(163);
  }
}

void setup() {
  // Initialize the Serial Monitor at 9600 baud
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Create the mutex
  xMutexSensorMem = xSemaphoreCreateMutex();
  xMutexActuatorMem = xSemaphoreCreateMutex();


  // Create tasks


  if(testing ==0){
    testWCETActuatorJob();
    testWCETSensorJob();
    testWCETControlJob();
    Serial.print("test finished");

  }
  else{
    xTaskCreate(sensorHandler, "Task 1", 1000, NULL, 1, NULL);
    xTaskCreate(controlUnit, "Task 2", 1000, NULL, 1, NULL);
    xTaskCreate(actuatorHandler, "Task 3", 1000, NULL, 1, NULL);
  }

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();
  

}

void loop() {

}

void actuator_job(){
      //get value from memory
      xSemaphoreTake(xMutexActuatorMem, portMAX_DELAY);
      int actuatorValue = actuatorMem;
      xSemaphoreGive(xMutexActuatorMem);

      //Serial.print("from actuatorHandler, actuatorValue: ");
      //Serial.println(actuatorValue);

      //Serial.print("from actuatorHandler, actuatorMem: ");
      //Serial.println(actuatorMem);

      //perform action
      analogWrite(LED_PIN, actuatorValue);
}

void testWCETActuatorJob() {
  unsigned long startTime, endTime;
  unsigned long totalTime = 0;
  const int iterations = 1000;

  // Run actuator_job 1000 times and measure execution time
  for (int i = 0; i < iterations; i++) {
    startTime = micros();  // Start time in microseconds
    actuator_job();        // Call the function
    endTime = micros();    // End time in microseconds

    totalTime += (endTime - startTime);  // Accumulate time taken for each iteration
  }

  // Print the total time taken for 1000 iterations
  Serial.print("Total time for 1000 iterations for Actuator Job: ");
  Serial.print(totalTime);
  Serial.println(" microseconds");

  // Optionally, print the average execution time per iteration
  Serial.print("Average time per iteration for actuator job: ");
  Serial.print(totalTime / iterations);
  Serial.println(" microseconds");
}

void testWCETSensorJob(){
  unsigned long startTime, endTime;
  unsigned long totalTime = 0;
  const int iterations = 1000;

  // Run sensor_job 1000 times and measure execution time
  for (int i = 0; i < iterations; i++) {
    startTime = micros();  // Start time in microseconds
    sensorJob();        // Call the function
    endTime = micros();    // End time in microseconds

    totalTime += (endTime - startTime);  // Accumulate time taken for each iteration
  }

  // Print the total time taken for 1000 iterations
  Serial.print("Total time for 1000 iterations for sensor job: ");
  Serial.print(totalTime);
  Serial.println(" microseconds");

  // Optionally, print the average execution time per iteration
  Serial.print("Average time per iteration sensor job: ");
  Serial.print(totalTime / iterations);
  Serial.println(" microseconds");
}

void sensorJob(){
   //read sensor value
    int sensorValue = analogRead(sensorPin);
    
    //Serial.print("from sesnsorHandler, sensor value: ");
    //Serial.println(sensorValue);

    //store in process memory
    xSemaphoreTake(xMutexSensorMem, portMAX_DELAY);
    sensorMem = sensorValue;
    xSemaphoreGive(xMutexSensorMem);

    //Serial.print("from sensorHandler, sensorMem value: ");
    //Serial.println(sensorMem);
   
}

void controlJob(){
  //get value from memory
      xSemaphoreTake(xMutexSensorMem, portMAX_DELAY);
      int sensorValue = sensorMem;
      xSemaphoreGive(xMutexSensorMem);

      //Serial.print("from controlUnit, sensorValue: ");
      //Serial.println(sensorValue);

      float differential = 0.5;

      //perform computation
      int y = sensorValue+differential;
      

      //Serial.print("from controlUnit, y: ");
      //Serial.println(y);

      //store to actuator memory
      xSemaphoreTake(xMutexActuatorMem, portMAX_DELAY);
      actuatorMem = y;
      xSemaphoreGive(xMutexActuatorMem);
}

void testWCETControlJob() {
  unsigned long startTime, endTime;
  unsigned long totalTime = 0;
  const int iterations = 1000;

  // Run control job 1000 times and measure execution time
  for (int i = 0; i < iterations; i++) {
    startTime = micros();  // Start time in microseconds
    controlJob();        // Call the function
    endTime = micros();    // End time in microseconds

    totalTime += (endTime - startTime);  // Accumulate time taken for each iteration
  }

  // Print the total time taken for 1000 iterations
  Serial.print("Total time for 1000 iterations for control job: ");
  Serial.print(totalTime);
  Serial.println(" microseconds");

  // Optionally, print the average execution time per iteration
  Serial.print("Average time per iteration for control job: ");
  Serial.print(totalTime / iterations);
  Serial.println(" microseconds");
}