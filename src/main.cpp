#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

// 센서 핀 설정
const int sensorPin = A0;

// DHT 센서 설정
#define DHT_PIN A3
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

/** Enable Non-I2C LCD or I2C-LCD */
LiquidCrystal_I2C lcd(0x27, 16, 2);
// LiquidCrystal lcd(A1,A2, 5,4,3,2);

// 미세먼지 센서 설정
#define no_dust 0.6
short Vo = A0;   // 미세먼지 입력 핀
short V_LED = 6; // 적외선 센서 출력핀
float Vo_value = 0;
float Voltage = 0;
float dustDensity = 0;

// Temp/Humidity Variabes
float dht_temp = 0;
float dht_hum = 0;
String lvl = "";

// MAC 주소 설정
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// 서버 포트 설정
EthernetServer server(80);

void setup()
{
  // 시리얼 통신 시작
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  /** DHT Initialize */
  dht.begin(DHT_PIN);

  /** Dust-Module Pin Setup */
  pinMode(V_LED, OUTPUT);
  pinMode(Vo, INPUT);

  Serial.println("이더넷 초기화 중...");

  // 이더넷 링크 상태 확인
  if (Ethernet.linkStatus() == LinkOFF)
    Serial.println("이더넷 케이블이 연결되지 않았습니다.");

  // DHCP를 통해 IP 주소 요청
  // Serial.println("DHCP를 통해 IP 주소 요청 중...");
  // if (Ethernet.begin(mac) == 0)
  // {
  Serial.println("DHCP로 구성 실패. 정적 IP 사용 시도...");
  IPAddress ip(192, 168, 0, 26);
  // IPAddress dns(8, 8, 8, 8);
  // IPAddress gateway(192, 168, 0, 1);
  // IPAddress subnet(255, 255, 255, 0);
  Ethernet.begin(mac, ip);
  // }

  // 서버 IP 주소 출력
  Serial.print("서버 IP 주소: ");
  Serial.println(Ethernet.localIP());

  // 서버 시작
  server.begin();
  Serial.println("서버가 시작되었습니다.");
}

void ReadSensor()
{
  dht_temp = dht.readTemperature();
  dht_hum = dht.readHumidity();

  // short init_pos = 0;
  digitalWrite(V_LED, LOW);
  delayMicroseconds(280);
  Vo_value = analogRead(Vo);
  delayMicroseconds(40);
  digitalWrite(V_LED, HIGH);
  delayMicroseconds(9680);

  // convert sensor value to Voltage
  Voltage = Vo_value * 5.0 / 1024.0;

  // Calculate Fine-Dust value
  // dustDensity = 30 + 5 * ( (new_Voltage - 0.8) * 10 ); // old ver
  dustDensity = (Voltage - no_dust) / 0.005; // NEW
}

void LCDPrint()
{
  // Print Temperature
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  if (dht_temp < 10)
  {
    lcd.print(" ");
    lcd.setCursor(3, 0);
  }
  lcd.print(dht_temp);
  lcd.print("'C  ");

  // Proint Humidity
  lcd.setCursor(9, 0);
  lcd.print("H: ");
  lcd.print(dht_hum);
  lcd.print("% ");

  /** DUST
   * -------------
   */
  // Display to LCD
  lcd.setCursor(0, 1);
  lcd.print(Voltage);

  /** Dust Level Visualization
   * Perfect (0 ~ 15.0)
   * Good (15.0 ~ 30.0)
   * Soso (30.0 ~ 40.0)
   * Normal (40.0 ~ 50.0)
   * bad (50.0 ~ 75.0)
   * VBAD Very Bad (75.0 ~ 100.0)
   * Danger (100.0 ~ 150.0)
   * WORST(150.0 ~ )
   * ERROR (Cannot measure)
   */
  lvl = "";
  if (0.0 < dustDensity && dustDensity < 15.0)
    lvl = "Perfect";
  else if (15.0 <= dustDensity && dustDensity < 30.0)
    lvl = "Good";
  else if (30.0 <= dustDensity && dustDensity < 40.0)
    lvl = "soso";
  else if (40.0 <= dustDensity && dustDensity < 50.0)
    lvl = "Normal";
  else if (50.0 <= dustDensity && dustDensity < 75.0)
    lvl = "Bad !";
  else if (75.0 <= dustDensity && dustDensity < 100.0)
    lvl = "VeryBAD";
  else if (100.0 <= dustDensity && dustDensity < 150.0)
    lvl = "Danger";
  else if (150 <= dustDensity)
    lvl = "WORST";
  else
    lvl = "ERROR";

  // clear previous text
  for (short m = 5; m < 12; m++)
  {
    lcd.setCursor(m, 1);
    lcd.print(" ");
  }

  // display dust
  lcd.setCursor(5, 1);
  lcd.print(lvl); // dispaly fine-dust state
  lcd.setCursor(12, 1);
  lcd.print(dustDensity); // display fine-dust level (number)
}

void loop()
{
  // 클라이언트가 연결되었는지 확인
  EthernetClient client = server.available();
  if (client)
  {
    Serial.println("새 클라이언트 연결됨");
    boolean currentLineIsBlank = true;
    String httpRequest = "";

    // 클라이언트가 연결되어 있는 동안 데이터 읽기
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        httpRequest += c;

        // HTTP 요청 끝 (빈 줄 감지)
        if (c == '\n' && currentLineIsBlank)
        {
          // HTTP 요청을 분석하여 센서 값 요청 처리
          if (httpRequest.indexOf("GET /sensor") >= 0)
          {
            ReadSensor();

            // HTTP 응답 전송
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            client.print("{\"temperature\": ");
            client.print(dht_temp);
            client.print(", \"humidity\": ");
            client.print(dht_hum);
            client.print(", \"Voltage\": ");
            client.print(Voltage);
            client.print(", \"dustDensity\": ");
            client.print(dustDensity);
            client.println("}");
          }
          else
          {
            // 다른 요청 처리 (예: 404 Not Found)
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<body>");
            client.println("404 Not Found");
            client.println("</body>");
            client.println("</html>");
          }
          break;
        }

        // 줄 끝 확인
        if (c == '\n')
          currentLineIsBlank = true;
        else if (c != '\r')
          currentLineIsBlank = false;
      }
    }

    // 연결 종료
    delay(1);
    client.stop();
    Serial.println("클라이언트 연결 종료");
  }

  ReadSensor();
  LCDPrint();

  delay(1000);
}
