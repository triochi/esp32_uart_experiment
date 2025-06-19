#include <AccelStepper.h>
#include <Preferences.h>


#define MOTOR_IN1 2  // Update with your actual GPIOs
#define MOTOR_IN2 3
#define MOTOR_IN3 4
#define MOTOR_IN4 5

#define ON_OF_Pin 10      // Adjust to match your wiring
#define ADC_O2_PIN 1      // Replace A2 with GPIO1
#define ADC_VALVE_PIN 0   // Replace A3 with GPIO0

#define MAX_STEPS 280
#define OK 750
#define RANGE_TOP 50
#define RANGE_BOTTOM 100
#define ADDRESS 0

#define MIDDLE 245

#define FORWARD  1
#define BACKWARD -1

Preferences prefs;
AccelStepper motor(AccelStepper::FULL4WIRE, MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4);

bool on = false;
bool wasOn = false;
bool changed = false;
int steps = MAX_STEPS;
int takenSteps = 0;
long lastWrite = 0;
long lastUpdate = 0;
int lastValve = 0;


// Task handle for motor control task
TaskHandle_t motorTaskHandle = NULL;

// Shared target position (use volatile for safe access between tasks)
volatile long targetPosition = 0;
volatile bool newPositionAvailable = false;

// WiFi AP credentials
const char* ssid = "MyESP32_AP";
const char* password = "12345678";  // must be at least 8 characters

void motorTask(void * parameter) {
  while (true) {
    // Check if a new position has been set
    if (newPositionAvailable) {
      // Locking mechanism is minimal here; consider mutex if you expect race conditions
      long pos = targetPosition;
      newPositionAvailable = false;

      motor.moveTo(pos);
      motor.runToPosition();  // blocking call, safe here because WiFi runs separately
    }
    // Let other tasks run
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Setup ESP32 as WiFi Access Point
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP address: ");
  Serial.println(IP);
  pinMode(ON_OF_Pin, INPUT);

  motor.setMaxSpeed(600);
  motor.setAcceleration(300);

  takenSteps = readSteps();
  lastWrite = millis();
  lastUpdate = lastWrite;
   targetPosition = 0;
  newPositionAvailable = true;

  xTaskCreate(
    motorTask,
    "MotorControl",
    4096,
    NULL,
    1,
    &motorTaskHandle
  );
}

void reset() {
  motor.moveTo(-MAX_STEPS);
  while (motor.distanceToGo() != 0) {
    motor.run();
  }
  writeSteps(0);
  while (takenSteps <= MAX_STEPS && takenSteps != MIDDLE) {
    if (takenSteps < MIDDLE) {
      motor.move(1);
      motor.runToPosition();
      takenSteps++;
    } else {
      motor.move(-1);
      motor.runToPosition();
      takenSteps--;
    }
  }
  changed = true;
}

void close() {
  while (takenSteps < MAX_STEPS) {
    motor.move(1);
    motor.runToPosition();
    takenSteps++;
  }
  changed = true;
}

int readSteps() {
  prefs.begin("motor", true); // Read-only mode
  int val = prefs.getInt("steps", 0);
  prefs.end();
  return val;
}

void writeSteps(int number) {
  prefs.begin("motor", false); // Read/write mode
  prefs.putInt("steps", number);
  prefs.end();
  takenSteps = number;
}

void move(int8_t direction, int16_t stepsToTake = 1) {
  while (stepsToTake > 0) {
    if (direction == FORWARD && takenSteps < MAX_STEPS) {
      motor.move(1);
      motor.runToPosition();
      takenSteps++;
      changed = true;
    } else if (direction == BACKWARD && takenSteps > 0) {
      motor.move(-1);
      motor.runToPosition();
      takenSteps--;
      changed = true;
    }
    stepsToTake--;
  }
}

void stay_middle() {
  if (takenSteps < MIDDLE) {
    motor.move(1);
    motor.runToPosition();
    takenSteps++;
    changed = true;
  }
}

void loop() {
  int o2 = analogRead(ADC_O2_PIN);
  int valve = analogRead(ADC_VALVE_PIN);
  long now = millis();
  on = digitalRead(ON_OF_Pin);

  Serial.print("O2: ");
  Serial.print(o2);
  Serial.print(" Valve: ");
  Serial.print(valve);
  Serial.print(" On: ");
  Serial.print(on);
  Serial.print(" Steps: ");
  Serial.println(takenSteps);

  if (on) {
    wasOn = true;

    if (valve - lastValve > 30) {
      move(BACKWARD, 10);
    } else if (lastValve - valve > 50) {
      move(FORWARD, 10);
    } else if (valve <= 60) {
      stay_middle();
    } else if (now - lastUpdate > 50 && (o2 > 400 || o2 < 200)) {
      lastUpdate = now;
      if (o2 > (OK + RANGE_TOP)) {
        move(FORWARD);
      } else if (o2 < (OK - RANGE_BOTTOM)) {
        move(BACKWARD);
      }
    }
  }

  if (!on && wasOn) {
    reset();
    wasOn = false;
  }

  if (Serial.available()) {
    int d = Serial.readString().toInt();
    if (d == 1) {
      motor.move(-10);
      motor.runToPosition();
    } else if (d == 2) {
      motor.move(10);
      motor.runToPosition();
    } else if (d == 3) {
      motor.move(-MAX_STEPS);
      motor.runToPosition();
    } else if (d == 4) {
      motor.move(MAX_STEPS);
      motor.runToPosition();
    } else if (d == 5) {
      on = !on;
    } else if (d == 6) {
      reset();
    } else if (d == 7) {
      Serial.println(digitalRead(ON_OF_Pin));
    } else if (d == 8) {
      Serial.println(takenSteps);
    } else if (d == 9) {
      writeSteps(0);
    } else if (d == 10) {
      close();
    }
  }

  motor.run();
  lastValve = valve;

  if (now - lastWrite > 3000 && changed) {
    writeSteps(takenSteps);
    changed = false;
    lastWrite = now;
  }
}
