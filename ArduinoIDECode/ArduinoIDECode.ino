const int r = 11;
const int g = 10;
const int b = 9;

const int button1 = 2;
const int button2 = 7;
const int button3 = 8;
const int button4 = 12;

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;

#define enable 5
#define dirA 3
#define dirB 4

String incomingLine = "";

// score and motor PWM logic
int score = 0;
int motorPWM = 0;    // target PWM computed from score
int currentPWM = 0;  // actual PWM we send to the motor (for smoothing)
int targetPWM = 0;   // same as motorPWM, but separated for clarity

const int minPWM = 150;   // PWM min that reliably spins the motor
const int maxPWM = 255;   // PWM max
const int maxScore = 20;  // Number of "CORRECT" needed to reach top

void setup() {
  //LED
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);

  digitalWrite(r, LOW);
  digitalWrite(g, LOW);
  digitalWrite(b, LOW);

  //DC Motor
  pinMode(enable, OUTPUT);
  pinMode(dirA, OUTPUT);
  pinMode(dirB, OUTPUT);

  digitalWrite(enable, LOW);
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);

  //buttons
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);

  Serial.begin(9600);
}

// Clamps score, compute motorPWM and targetPWM from score
void updateMotorFromScore() {
  if (score < 0) score = 0;
  if (score > maxScore) score = maxScore;

  if (score == 0) {
    motorPWM = 0;  // motor off
  } else {
    motorPWM = map(score, 1, maxScore, minPWM, maxPWM);
  }
  targetPWM = motorPWM;

  Serial.print("PWM: ");
  Serial.println(motorPWM);
  Serial.print("Score: ");
  Serial.println(score);
}


void loop() {

  buttonState1 = digitalRead(button1);
  buttonState2 = digitalRead(button2);
  buttonState3 = digitalRead(button3);
  buttonState4 = digitalRead(button4);

  //Correct
  if (buttonState1 == HIGH) {
    score++;
    ledRight();
    updateMotorFromScore();
  }
  //Incorrect
  if (buttonState2 == HIGH) {
    score--;
    ledWrong();
    updateMotorFromScore();
  }
  //Start
  if (buttonState3 == HIGH) {
    start();
  }
  //Stop
  if (buttonState4 == HIGH) {
    stop();
  }

  // Read incoming characters
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\n') {
      
      incomingLine.trim();  // remove \r and spaces

      if (incomingLine == "CORRECT") {
        score++;
        ledRight();
      } else if (incomingLine == "WRONG") {
        score--;
        ledWrong();
      } else if (incomingLine == "START") {
        start();
      } else if (incomingLine == "STOP") {
        stop();
      }

      if (incomingLine == "CORRECT" || incomingLine == "WRONG") {
        updateMotorFromScore();
      }

      incomingLine = "";  // reset for next message
    } else {
      incomingLine += c;
    }
  }


  if (currentPWM < targetPWM) {
    currentPWM++;
  } else if (currentPWM > targetPWM) {
    currentPWM--;
  }

  fanOn(currentPWM);

  delay(10);
}

void ledWrong() {
  for (int i = 0; i <= 2; i++) {
    analogWrite(b, 15);
    analogWrite(r, 100);

    delay(150);

    digitalWrite(r, LOW);
    digitalWrite(g, LOW);
    digitalWrite(b, LOW);

    delay(150);
  }
}

void ledRight() {
  for (int i = 0; i <= 2; i++) {
    analogWrite(b, 15);
    analogWrite(g, 100);

    delay(150);

    digitalWrite(r, LOW);
    digitalWrite(g, LOW);
    digitalWrite(b, LOW);

    delay(150);
  }
}

void fanOn(int pwm) {
  if (pwm <= 0) {
    // motor off
    analogWrite(enable, 0);
    digitalWrite(dirA, LOW);
    digitalWrite(dirB, LOW);
  } else {
    digitalWrite(dirA, HIGH);
    digitalWrite(dirB, LOW);
    analogWrite(enable, pwm);
  }
}

void start() {
  if (score == 0) {
    score = 1;
  }
  updateMotorFromScore();
}

void stop() {
  score = 0;
  updateMotorFromScore();
}
