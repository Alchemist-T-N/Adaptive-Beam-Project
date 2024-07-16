#define BLYNK_TEMPLATE_ID "TMPL3zJdFCC9T"
#define BLYNK_TEMPLATE_NAME "ADB"
#define BLYNK_AUTH_TOKEN "OJU7YlPrVbE8qGx7T9-cb1urT4gxlk_O"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial

#define ir_count 2               // infrared sensors count
#define ldr_count 2              // luminous sensor count
#define led_count 2              // led light count
#define us_count 1               // ultrasonic sensors count
int dist_range[2] = { 10, 20 };  // the distance range where the operation should take place.

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

const char* ssid = "Alchemist";          // wifi id
const char* password = "Hlobipin@1234";  // wifi password
int automatic = 1;                       // it is connected with touch. it can be either 0 or 1
int i = 0;                               // for looping statements

//all io ports:
/*
  d0 - 16
  d1 - 5
  d2 - 4
  d3 - 0
  d4 - 2;.
  d5 - 14
  d6 - 12
  d7 - 13
  d8 - 15
*/

int irs[2] = { 16, 4 };    //[d0,d2]
int ldrs[2] = { 5, 0 };    //[d1,d3]
int uss[2] = { 2, 14 };    //[d4, d5] [output, input] only one sensor
int leds[2] = { 12, 13 };  //[d6, d7]
int touch = 15;            // d8



void connect_wifi_blink() {
  // connect to wifi and connect ot blynk
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Wifi Connecting....");
  }
  Serial.println("WiFi connected");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Blynk.run();
}


void light_handler(int led, int of_on) {
  if (led == 0) {
    Blynk.virtualWrite(V0, of_on);
    digitalWrite(leds[0], of_on);
    Serial.print("led-0 is ");
  } else {
    Blynk.virtualWrite(V1, of_on);
    digitalWrite(leds[1], of_on);
    Serial.print("led-1 is ");
  }
  if (of_on == 1) {
    Serial.println("on");
  } else {
    Serial.println("off");
  }
}



void all_led(int off_on) {
  Blynk.virtualWrite(V0, off_on);
  Blynk.virtualWrite(V1, off_on);
  int i = 0;
  for (i = 0; i < led_count; i++) {
    digitalWrite(leds[i], off_on);
  }
}


int* ir_or_ldr_reader() {
  int i, ir_input, ldr_input;
  static int is_on[2] = { 0, 0 };
  for (i = 0; i < ir_count; i++) {
    ir_input = !digitalRead(irs[i]);              //reading ir sensors default one -> get negetion of the value
    ldr_input = !digitalRead(ldrs[i]);            //reading ldr sensors default one -> get negetion of the value
    if (ir_input == HIGH || ldr_input == HIGH) {  //check if ir detects car or ldr detects high beam
      is_on[i] = 1;
    } else {
      is_on[i] = 0;
    }
  }
  return is_on;
}




int get_car_dist() {
  long distance;
  long duration;
  digitalWrite(uss[0], LOW);
  delayMicroseconds(2);
  digitalWrite(uss[0], HIGH);
  delayMicroseconds(10);
  digitalWrite(uss[0], LOW);
  duration = pulseIn(uss[1], HIGH);
  distance = duration * 0.034 / 2;
  if (dist_range[0] < distance && distance < dist_range[1]) {
    Serial.print("is in range : ");
    Serial.print(distance);
    Serial.println(" cm.");
    return 1;
  } else {
    Serial.print("not in range : ");
    Serial.print(distance);
    Serial.println(" cm.");
    return 0;
  }
}


int is_automatic() {
  if (digitalRead(touch) == HIGH) {
    automatic = !automatic;
  }
  return automatic;
}

void setup() {
  Serial.begin(115200);
  int i;
  connect_wifi_blink();  // connect to wifi else it will not work.
  //infrared
  for (i = 0; i < ir_count; i++) {
    pinMode(irs[i], INPUT);
  }
  //luminous
  for (i = 0; i < ir_count; i++) {
    pinMode(ldrs[i], INPUT);
  }
  //ultrasonic
  pinMode(uss[0], OUTPUT);
  pinMode(uss[1], INPUT);
  //leds
  for (i = 0; i < led_count; i++) {
    pinMode(leds[i], OUTPUT);
  }
  //touch
  pinMode(touch, INPUT);

  all_led(HIGH);
}

void loop() {
  /*
    check if automatic or manual mode.
    get if car or light detected.
    if car or light detected, check if the car is in range.
    turn off respected led if in range
    else turn on respected led
  */
  if (is_automatic()) {
    Serial.println(" ");
    Serial.println(" ");
    Serial.println("automatic mode ...... ");
    int* is_detected = ir_or_ldr_reader();  //is an array
    // Serial.print("is_detected: ");
    // Serial.print(is_detected[0]);
    // Serial.print("  --  ");
    // Serial.println(is_detected[1]);

    int is_in_range = get_car_dist();

    for (i = 0; i < led_count; i++) {
      if (is_detected[i] == 1 && is_in_range == 1) {  //if any of the ir or ldr is detected
        light_handler(i, LOW);
      } else {
        light_handler(i, HIGH);
      }
    }

  } else {
    Serial.println("manual mode.....");
  }
  delay(100);
}
