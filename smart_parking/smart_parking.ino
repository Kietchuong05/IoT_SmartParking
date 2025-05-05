#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Thông tin WiFi
const char* ssid = "Fablab 2.4G";
const char* wifi_password = "Fira@2024";

// Thông tin MQTT Broker
const char* mqtt_server = "192.168.69.177";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* client_id = "ESP_Parking_System";

WiFiClient espClient;
PubSubClient client(espClient);

// Định nghĩa chân cảm biến IR và servo
#define IR_SENSOR_PIN_1 D5
#define SERVO_PIN_1 D7
#define IR_SENSOR_PIN_2 D6  
#define SERVO_PIN_2 D8
#define IR_SENSOR_PIN_3 D3

// Cấu hình logic cảm biến IR (true: HIGH khi phát hiện; false: LOW khi phát hiện)
const bool IR_ACTIVE_HIGH = true;

// Khởi tạo LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Khởi tạo đối tượng Servo
Servo myServo1;
Servo myServo2;

// Biến lưu trạng thái trước đó
int lastIrState1 = -1;
int lastIrState2 = -1;
int lastIrState3 = -1;
int lastServoAngle1 = -1;
int lastServoAngle2 = -1;

// Thời gian cập nhật
unsigned long lastUpdate = 0;
const long updateInterval = 500; // Tăng lên 500ms để giảm tần suất cập nhật

void setup() {
  Serial.begin(115200);
  Serial.println("starting.");

  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting.");
  delay(1000);
  lcd.clear();

  // Kết nối WiFi
  setup_wifi();
  
  // Cấu hình MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  // Cấu hình chân cảm biến IR
  pinMode(IR_SENSOR_PIN_1, INPUT);
  pinMode(IR_SENSOR_PIN_2, INPUT);
  pinMode(IR_SENSOR_PIN_3, INPUT);

  // Gắn servo
  if (!myServo1.attach(SERVO_PIN_1)) {
    Serial.println("Servo 1 attach failed");
    lcd.clear();
    lcd.print("Servo 1 Error");
    while (true);
  }
  myServo1.write(0);
  Serial.println("Servo 1 initialized at 0 degrees");

  if (!myServo2.attach(SERVO_PIN_2)) {
    Serial.println("Servo 2 attach failed");
    lcd.clear();
    lcd.print("Servo 2 Error");
    while (true);
  }
  myServo2.write(0);
  Serial.println("Servo 2 initialized at 0 degrees");

  // Cập nhật trạng thái ban đầu
  int irState1 = readStableIrState(IR_SENSOR_PIN_1);
  int irState2 = readStableIrState(IR_SENSOR_PIN_2);
  int irState3 = readStableIrState(IR_SENSOR_PIN_3);
  
  updateLcdAndServos(irState1, irState2, irState3);
  publishSensorStates(irState1, irState2, irState3);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to:");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Xử lý lệnh điều khiển servo 1
  if (String(topic) == "parking/barrier1/control") {
    int angle = message.toInt();
    angle = constrain(angle, 0, 90);
    myServo1.write(angle);
    Serial.print("Servo 1 set to: ");
    Serial.println(angle);
    
    // Cập nhật LCD nếu cần
    lcd.setCursor(0, 0);
    lcd.print(angle == 0 ? "Mo     " : "Dong   ");
    
    // Gửi lại trạng thái
    client.publish("parking/barrier1/state", String(angle).c_str());
  }

  // Xử lý lệnh điều khiển servo 2
  if (String(topic) == "parking/barrier2/control") {
    int angle = message.toInt();
    angle = constrain(angle, 0, 90);
    myServo2.write(angle);
    Serial.print("Servo 2 set to: ");
    Serial.println(angle);
    
    // Cập nhật LCD nếu cần
    lcd.setCursor(0, 1);
    lcd.print(angle == 0 ? "Mo     " : "Dong   ");
    
    // Gửi lại trạng thái
    client.publish("parking/barrier2/state", String(angle).c_str());
  }
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(client_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      
      // Đăng ký các topic cần nhận
      client.subscribe("parking/barrier1/control");
      client.subscribe("parking/barrier2/control");
      
      // Gửi thông báo online
      client.publish("parking/status", "online");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishSensorStates(int ir1, int ir2, int ir3) {
  if (!client.connected()) {
    return;
  }
  
  client.publish("parking/sensor/entry1", ir1 == LOW ? "free" : "occupied");
  client.publish("parking/sensor/entry2", ir2 == LOW ? "free" : "occupied");
  client.publish("parking/sensor/slot1", ir3 == HIGH ? "empty" : "occupied");
  
  Serial.println("Published sensor states to MQTT");
}

int readStableIrState(int pin) {
  int readings = 0;
  for (int i = 0; i < 10; i++) {
    readings += digitalRead(pin);
    delay(5);
  }
  int state = (readings >= 6) ? HIGH : LOW;
  return IR_ACTIVE_HIGH ? state : !state;
}

void updateLcdAndServos(int irState1, int irState2, int irState3) {
  // Kiểm tra trạng thái IR3
  bool isOccupied = (irState3 == LOW);

  // Cập nhật LCD cho IR1 (bên trái dòng 1)
  if (isOccupied && lastIrState1 != HIGH) { 
    lcd.setCursor(0, 0);
    lcd.print("Dong   ");
    lastIrState1 = HIGH;
    Serial.println("IR1 forced to: Dong (IR3 occupied)");
  } else if (!isOccupied && irState1 != lastIrState1) {
    lcd.setCursor(0, 0);
    lcd.print(irState1 == LOW ? "Mo     " : "Dong   ");
    lastIrState1 = irState1;
    delay(20);
    Serial.print("IR1 changed to: ");
    Serial.println(irState1 == LOW ? "Mo" : "Dong");
  }

  // Cập nhật LCD cho IR3 (bên phải dòng 1)
  if (irState3 != lastIrState3) {
    lcd.setCursor(9, 0);
    lcd.print(irState3 == HIGH ? "KhongXe" : "CoXe   ");
    lastIrState3 = irState3;
    delay(20);
    Serial.print("IR3 changed to: ");
    Serial.println(irState3 == HIGH ? "Khong Xe" : "Co Xe");
  }

  // Cập nhật LCD cho IR2 (bên trái dòng 2)
  if (isOccupied && lastIrState2 != HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("Dong   ");
    lastIrState2 = HIGH;
    Serial.println("IR2 forced to: Dong (IR3 occupied)");
  } else if (!isOccupied && irState2 != lastIrState2) {
    lcd.setCursor(0, 1);
    lcd.print(irState2 == LOW ? "Mo     " : "Dong   ");
    lastIrState2 = irState2;
    delay(20);
    Serial.print("IR2 changed to: ");
    Serial.println(irState2 == LOW ? "Mo" : "Dong");
  }

  // Điều khiển servo 1
  int targetAngle1 = isOccupied ? 90 : (irState1 == LOW ? 0 : 90);
  if (targetAngle1 != lastServoAngle1) {
    myServo1.write(targetAngle1);
    Serial.print("Servo 1 commanded to ");
    Serial.println(targetAngle1);
    lastServoAngle1 = targetAngle1;
    
    // Gửi trạng thái servo qua MQTT
    if (client.connected()) {
      client.publish("parking/barrier1/state", String(targetAngle1).c_str());
    }
  }

  // Điều khiển servo 2
  int targetAngle2 = isOccupied ? 90 : (irState2 == LOW ? 0 : 90);
  if (targetAngle2 != lastServoAngle2) {
    myServo2.write(targetAngle2);
    Serial.print("Servo 2 commanded to ");
    Serial.println(targetAngle2);
    lastServoAngle2 = targetAngle2;
    
    // Gửi trạng thái servo qua MQTT
    if (client.connected()) {
      client.publish("parking/barrier2/state", String(targetAngle2).c_str());
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= updateInterval) {
    int irState1 = readStableIrState(IR_SENSOR_PIN_1);
    int irState2 = readStableIrState(IR_SENSOR_PIN_2);
    int irState3 = readStableIrState(IR_SENSOR_PIN_3);

    updateLcdAndServos(irState1, irState2, irState3);
    publishSensorStates(irState1, irState2, irState3);
    
    lastUpdate = currentMillis;
  }
}