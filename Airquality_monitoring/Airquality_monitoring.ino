#include <Wire.h> 
#include <DHT11.h>
#include <pm2008_i2c.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
PM2008_I2C pm2008_i2c;

int redPin = 11;
int greenPin = 10;
int bluePin = 9;
int buzzerPin = 8;
int dhtPin = 7;
int irReceiverPin = 6;
int buzzerFlag = 0; // 특정 조건에 Buzzer 동작을 위한 플래그 변수
int lcdMode = 0; // LCD 출력모드 컨트롤을 위한 플래그 변수
DHT11 dht11(dhtPin);

// IR receive Setting
IRrecv irrecv(irReceiverPin);
decode_results decodedSignal;
unsigned long last = millis();

void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    irrecv.enableIRIn(); // Start IR receiver
    Serial.begin(9600);
    pm2008_i2c.begin();
    pm2008_i2c.command();
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.print("Turning on...");
    Serial.println("\n\nTurning on...\n");
    delay(4000);
}

void loop() {
    int err;
    float temp, humi;
    int intTemp, intHumi;
    unsigned char pm2, pm10;
    uint8_t ret = pm2008_i2c.read();

    // DHT11
    if ((err = dht11.read(humi, temp)) == 0) {
        intTemp = (int)temp;
        intHumi = (int)humi;
    }
    else {
        Serial.print("DHT11 Error");
    }

    // PM2008
    if (ret == 0) {
        pm2 = pm2008_i2c.pm2p5_grimm;
        pm10 = pm2008_i2c.pm10_grimm;
    }
    else {
        Serial.print("PM2008 Error");
    }

    // IR receive
    if (irrecv.decode(&decodedSignal) == true) { // if (IR 데이터 수신)
      Serial.println(decodedSignal.value);
        if (millis() - last > 250/*250*/) {
            if (decodedSignal.value == 16718055) { // 리모컨 2번 눌릴시
                lcdMode = 1;
            }
            else if (decodedSignal.value == 16724175) { // 리모컨 1번 눌릴시
                lcdMode = 0;
            }
            Serial.print("LCD Mode: ");
            Serial.println(lcdMode);
        }
        last = millis();
        irrecv.resume(); // watch out for another message
    }

    // Serial 모니터 출력
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print("\tHumid: ");
    Serial.print(humi);
    Serial.println();
    Serial.print("PM 2.5 (GRIMM) : ");
    Serial.println(pm2);
    Serial.print("PM 10 (GRIMM) : ");
    Serial.println(pm10);
    Serial.println();

    // LCD 출력
    lcd.clear();
    if (lcdMode == 0) {
        lcd.print("Temp: ");
        lcd.print(intTemp);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humi: ");
        lcd.print(intHumi);
        lcd.print("%");
    }
    else if (lcdMode == 1) {
        lcd.print("PM2.5: ");
        lcd.print(pm2); // grimm방식 표기
        lcd.print("ug/m^3");
        lcd.setCursor(0, 1);
        lcd.print("PM10.: ");
        lcd.print(pm10); // grimm방식 표기
        lcd.print("ug/m^3");
    }

    // PM2.5 수치에 따른 RGB LED 색 변환 및 Buzzer 동작
    if (pm2 < 15) { // 좋음
        setColor(80, 188, 223);
        buzzerFlag = 0;
    }
    else if (pm2 >= 15 && pm2 < 35) { // 보통
        setColor(0, 255, 0);
        buzzerFlag = 0;
    }
    else if (pm2 >= 35 && pm2 < 75) { // 나쁨
        setColor(255, 100, 0);
        if (buzzerFlag == 0) {
            // buzzer 2번 울림
            for (int j = 0; j < 2; j++) {
                digitalWrite(buzzerPin, HIGH);
                delay(400);
                digitalWrite(buzzerPin, LOW);
                delay(400);
            }
            buzzerFlag = 1;
        }
    }
    else if (pm2 >= 75) { // 매우 나쁨
        setColor(255, 0, 0);
        if (buzzerFlag == 0 || buzzerFlag == 1) {
            // buzzer 3번 울림
            for (int j = 0; j < 3; j++) {
                digitalWrite(buzzerPin, HIGH);
                delay(300);
                digitalWrite(buzzerPin, LOW);
                delay(300);
            }
            buzzerFlag = 2;
        }
    }

    delay(2000); // 새로고침 주기 2초

} // End of loop()

// RGB LED 제어를 위한 함수
void setColor(int red, int green, int blue) {
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}
