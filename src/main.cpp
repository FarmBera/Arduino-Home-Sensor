// #include <LiquidCrystal.h> // Non-I2C LCD Library
#include <LiquidCrystal_I2C.h> // I2C LCD Library
#include <DS1302.h>            // RTC (Real Time Clock) Library
#include <DHT.h>               // Temp/Hum Library
#include <SPI.h>               // SD Card Library
#include <SD.h>                // SD Card Library
#include <Adafruit_Sensor.h>

String FILE_PATH = "roomtemp.txt";
File myFile;
short logState = 0;

/** pin list
 * A3: DHT (temp/hum sensor)
 *
 * << RTC >>
 * 9 - RST
 * 8 - DAT
 * 7 - CLK
 *
 * << Dust Sensor >>
 * A0 - input sensor
 * 6 - IR Sensor output
 *
 * << SD pin >>
 * 4
 */

/** Enable Temp/Hum Sensor */
#define DHT_PIN A3
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

/** Enable RTC (Real Tie CLock)
 * Output: RST, DAT, CLK  */
DS1302 myrtc(9, 8, 7); //

/** Enable Non-I2C LCD or I2C-LCD */
LiquidCrystal_I2C lcd(0x27, 16, 2);
// LiquidCrystal lcd(A1,A2, 5,4,3,2);

/** Fine Dust Variables */
#define no_dust 0.6
short Vo = A0;   // 미세먼지 입력 핀
short V_LED = 6; // 적외선 센서 출력핀
float Vo_value = 0;
float Voltage = 0;     // 측정된 전압값
float dustDensity = 0; // 최종 미세먼지 계산

String DATE = ""; // date var
String TIME = ""; // clock var

// Temp/Humidity Variabes
int dht_temp = 0;
int dht_hum = 0;
String lvl = "";

// is SD Card Prepared?
short SD_PIN = 4;
short init_res_sd = 0;

/** Get Time & Date from RTC Module */
void ClockRequest()
{
  TIME = myrtc.getTimeStr();
  DATE = myrtc.getDateStr();
}

/** Print clock at LCD */
void DisplayClock(short pos = 0)
{
  String Temp_TIME = TIME.substring(0, 5); // .toInt()

  lcd.setCursor(0, pos);
  lcd.print(Temp_TIME + " " + DATE);
}

void DisplayClockSecond(short pos = 0)
{
  String Temp_TIME = TIME.substring(0, 5); // .toInt()

  lcd.setCursor(0, pos);
  lcd.print(TIME);
  lcd.print(" ");
  lcd.print(DATE);
}

/** Get second time for condition determination */
String GetClockSecTime()
{
  String TIME_SEC = myrtc.getTimeStr();
  return TIME_SEC.substring(6, 8);
}

/** Get second time for condition determination */
String GetClockMiliTime()
{
  String TIME_SEC = myrtc.getTimeStr();
  return TIME_SEC.substring(7, 8);
}

/** Display Temp/Hum Level */
void DisplayTemp(short pos = 0)
{
  // Load Sensor Value
  dht_temp = dht.readTemperature();
  dht_hum = dht.readHumidity();

  // Print Temperature
  lcd.setCursor(0, pos);
  lcd.print("T: ");
  if (dht_temp < 10)
  {
    lcd.print(" ");
    lcd.setCursor(3, pos);
  }
  lcd.print(dht_temp);
  lcd.print("'C  ");

  // Proint Humidity
  lcd.setCursor(9, pos);
  lcd.print("H: ");
  lcd.print(dht_hum);
  lcd.print("% ");
}

/** Display Fine-Dust Level */
void DisplayDust(short pos = 1)
{
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

  // Display to LCD
  lcd.setCursor(0, pos);
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
  for (short m = 7; m < 12; m++)
  {
    lcd.setCursor(m, pos);
    lcd.print(" ");
  }

  // Display the write operation to the SD card has been completed normally
  lcd.setCursor(4, pos);
  if (logState == 1)
    lcd.print("V");
  else if (logState == 0)
    lcd.print("X");
  else
    lcd.print("E");

  // display current minute
  lcd.setCursor(5, pos);
  lcd.print(TIME.substring(3, 5));

  // display dust
  lcd.setCursor(7, pos);
  lcd.print(lvl); // dispaly fine-dust state
  lcd.setCursor(12, pos);
  lcd.print(dustDensity); // display fine-dust level (number)
}

/** logging information at SD Card (temperature, humidity, dust-level etc) */
void SDCardLogging()
{
  myFile = SD.open(FILE_PATH, FILE_WRITE); // open file with writing mode

  String RESULTStr = "";

  if (myFile) // File opened normally
  {
    // Write information
    myFile.println(DATE + " " + TIME + " >> " + dht_temp + " " + dht_hum + " " + Voltage + " " + dustDensity);
    myFile.close();

    // Print complete message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Writing Done at");
    lcd.setCursor(1, 0);
    lcd.print(FILE_PATH);
    delay(500);
    DisplayClock(1);

    logState = 1;
  }
  else
  { // When file not opened
    logState = 0;
    // Serial.println("Error with Opening File >> " + FILE_PATH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Writing Failed");
    DisplayClock(1);
    delay(2000);
  }
}

void setup()
{
  // Serial.begin(9600);

  // LCD Initialization
  // lcd.begin(16, 2); // for Non-I2C LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  /** DHT Initialize */
  dht.begin(DHT_PIN);

  /** Dust-Module Pin Setup */
  pinMode(V_LED, OUTPUT);
  pinMode(Vo, INPUT);

  /** set RTC (Real Time Clock) module's time */
  // myrtc.halt(false);           // 동작 모드로 설정
  // myrtc.writeProtect(false);   // 시간 변경을 가능하게 설정
  // myrtc.setDOW(TUESDAY);       // 요일 설정
  // myrtc.setTime(15, 2, 0);     // 시간 설정 ( 시간, 분, 초 )
  // myrtc.setDate(24, 12, 2024); // 날짜 설정 ( 일, 월, 년도 )
  // myrtc.writeProtect(true);    // 시간 변경을 불가능하게 설정
  // // myrtc.halt(true);

  /** Start SD Card Module Initialization */
  logState = 0;
  Serial.print("Initializing SD card...");
  lcd.setCursor(0, 0);
  lcd.print("Initializing");
  lcd.setCursor(0, 1);
  lcd.print("SD card...");

  if (!SD.begin(SD_PIN))
  { // Initialize SD Card Module
    Serial.println("Initialization Failed!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Initialization");
    lcd.setCursor(0, 1);
    lcd.print("Failed!");

    logState = 0;

    // while (1);
  }
  else
  { // When Initialization Success,
    logState = 1;
    Serial.println("Initialization Done.");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Initialization");
    lcd.setCursor(0, 1);
    lcd.print("SD Card DONE !");

    init_res_sd = 1;

    delay(1 * 1000);
  }

  // Display log start message
  if (init_res_sd == 1)
  {
    logState = 1;
    lcd.clear();
    lcd.print("Logging Start!");
    DisplayClock(1);
    DisplayClock(1);
  }
  delay(1 * 1000);

  /** Logfile Initialization */
  ClockRequest();

  myFile = SD.open(FILE_PATH, FILE_WRITE);
  String RESULTStr = "";

  if (myFile) // file open
  {
    // Write Files
    myFile.println("");
    myFile.println("");
    myFile.print("Logfile at ");
    myFile.print(DATE);
    myFile.print(" ");
    myFile.println(TIME);
    myFile.close();

    init_res_sd = 1;

    logState = 1;
    lcd.setCursor(0, 0);
    lcd.print("LogfileInit Done");
    DisplayClock(1);
  }
  else // error
  {
    logState = 0;
    Serial.print("ERROR with Opening >> ");
    Serial.println(FILE_PATH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR with");
    lcd.print("Opening");
    lcd.print(FILE_PATH);

    init_res_sd = 0;
  }
  delay(1 * 1000);
  lcd.clear();
}

void loop()
{
  ClockRequest();

  if (init_res_sd == 1 &&
      GetClockSecTime().toInt() == 0)
  {
    SDCardLogging();
  }

  /** divide
   * 0 1 2
   * 3 4
   * 5 6 7
   * 8 9
   */
  int sec = GetClockMiliTime().toInt();
  if (
      (0 <= sec && sec <= 2) ||
      (5 <= sec && sec <= 7))
  {
    DisplayTemp();
  }
  else if (
      (3 <= sec && sec <= 4) ||
      (8 <= sec && sec <= 9))
  {
    DisplayClockSecond();
  }
  DisplayDust();

  delay(900);
}