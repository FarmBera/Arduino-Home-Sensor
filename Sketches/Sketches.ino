// #include <LiquidCrystal.h> // Non-I2C LCD Library
#include <LiquidCrystal_I2C.h> // I2C LCD Library
#include <DS1302.h> // RTC (Real Time Clock) Library
#include <DHT.h> // Temp/Hum Library
#include <SPI.h> // SD Card Library
#include <SD.h> // SD Card Library


String FILE_PATH = "roomtemp.txt";
File myFile;
short logState = 0;

/** Enable Temp/Hum Sensor */
#define DHTPIN A3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/** Enable RTC (Real Tie CLock)
 * Output: RST, DAT, CLK  */
DS1302 myrtc(9, 8, 7); // 

/** Enable Non-I2C LCD, I2C-LCD */
LiquidCrystal_I2C lcd(0x27, 16, 2);
// LiquidCrystal lcd(A1,A2, 5,4,3,2); 

/** Fine Dust Variables */
#define no_dust 0.6
short Vo = A0; // 미세먼지 입력 핀
short V_LED = 6; // 적외선 센서 출력핀
float Vo_value = 0;
float Voltage = 0; // 측정된 전압값
float dustDensity = 0; // 최종 미세먼지 계산

// Date Variables
String DATE = "";
// Clock Variables
String TIME = "";
// Temp/Humidity Variabes
int dht_temp = 0;
int dht_hum = 0;
String lvl = "";

// is SD Card Prepared?
short init_res_sd = 0;



/** Get Time & Date from RTC Module (for Syncing program times) */
void ClockRequest() {
    TIME = myrtc.getTimeStr();
    DATE = myrtc.getDateStr();
}


/** Print clock at LCD */
void DisplayClock(short pos=0) {
    String Temp_TIME = TIME.substring(0, 5); // .toInt()

    lcd.setCursor(0, pos);
    lcd.print(Temp_TIME + " " + DATE);
    // lcd.print(Temp_TIME);
    // lcd.print(" ");
    // lcd.print(DATE);
}


void DisplayClockSecond(short pos=0) {
    String Temp_TIME = TIME.substring(0, 5); // .toInt()

    lcd.setCursor(0, pos);
    lcd.print(TIME);
    lcd.print(" ");
    lcd.print(DATE);
}


/** Print Full-Screen clock */
void DisplayClockFull() {
    lcd.setCursor(4, 0);
    lcd.print(TIME);
    lcd.setCursor(3, 1);
    lcd.print(DATE);
}


/** Get second time for condition determination */
String GetClockSecTime() {
    String TIME_SEC = myrtc.getTimeStr();
    // TIME_SEC = TIME_SEC.substring(6, 8);
    // Serial.print(TIME_SEC);
    return TIME_SEC.substring(6, 8);
}


/** Get second time for condition determination */
String GetClockMiliTime() {
    String TIME_SEC = myrtc.getTimeStr();
    // TIME_SEC = TIME_SEC.substring(6, 8);
    // Serial.print(TIME_SEC);
    return TIME_SEC.substring(7, 8);
}


/** Display Temp/Hum Level */
void DisplayTemp(short pos = 0) {
    // Load Sensor Value
    dht_temp = dht.readTemperature();
    dht_hum = dht.readHumidity();

    // Print Temperature
    lcd.setCursor(0, pos);
    lcd.print("T: ");
    if (dht_temp < 10) {
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
void DisplayDust(short pos = 1) {
    // short init_pos = 0;
    digitalWrite(V_LED, LOW);
    delayMicroseconds(280);
    Vo_value = analogRead(Vo);
    delayMicroseconds(40);
    digitalWrite(V_LED, HIGH);
    delayMicroseconds(9680);

    // 센서 전압값 계산
    Voltage = Vo_value * 5.0 / 1024.0; // 아날로그 값을 전압값으로 Change

    // 미세먼지 계산하기
    // dustDensity = 30 + 5 * ( (new_Voltage - 0.8) * 10 ); // SOSO
    dustDensity = (Voltage - no_dust) / 0.005; // NEW

    // LCD에 출력하기
    lcd.setCursor(0, pos);
    lcd.print(Voltage);


    /** 미세먼지 수치 별 시각화
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


    for (short m=7; m<12; m++) {
        lcd.setCursor(m, pos);
        lcd.print(" ");
    }


    lcd.setCursor(4, pos);
    if (logState == 1) lcd.print("O");
    else if (logState == 0) lcd.print("X");
    else lcd.print("E");

    lcd.setCursor(5, pos);
    // Serial.println(TIME.substring(3, 5));
    lcd.print(TIME.substring(3, 5));


    lcd.setCursor(7, pos);
    lcd.print(lvl);
    lcd.setCursor(12, pos); // 미세먼지 값
    lcd.print(dustDensity);
}


/** Write custom message at file */
void SDCardLogging(String CustomMsg) {
    myFile = SD.open(FILE_PATH, FILE_WRITE); // Open file with writing mode

    if (myFile) { // File opened normally
        // File Open Message
        Serial.println("Writing Custom Message...");

        myFile.println(DATE + " " + TIME + " Message >> " + CustomMsg);
        myFile.close();

        // Complete Message
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Writed CusMsg at");
        DisplayClock(1);
    }
    else { // File not opened
        Serial.println("Error with Opening File >> " + FILE_PATH);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Writing Msg Failed");
        DisplayClock(1);
    }
}


/** logging at SD Card */
void SDCardLogging() {
    myFile = SD.open(FILE_PATH, FILE_WRITE); // open file with writing mode

    String RESULTStr = "";

    if (myFile) { // File opened normally
        // File Open Message
        Serial.print("Writing");

        // Write information
        Serial.print("/");
        myFile.println(DATE + " " + TIME + " >> " + dht_temp + " " + dht_hum + " " + Voltage + " " + dustDensity);
        Serial.print(".");
        myFile.close();
        Serial.print(".");

        // Print complete message
        Serial.print("/");
        lcd.clear();
        Serial.print(".");
        lcd.setCursor(0, 0);
        Serial.print(".");
        lcd.print("Writing Done at");
        Serial.print(".");
        DisplayClock(1);
        Serial.print(".");

        logState = 1;
        Serial.println("Done");
    }
    else { // When file not opened
        logState = 0;
        Serial.println("Error with Opening File >> " + FILE_PATH);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Writing Failed");
        DisplayClock(1);
    }
}


void setup() {
    Serial.begin(9600);

    // lcd.begin(16, 2);

    lcd.init();
    lcd.backlight();
    // lcd.clear();

    /** Dust-Module Pin Setup */
    pinMode(V_LED, OUTPUT);
    pinMode(Vo, INPUT);

    /** Sync Time at RTC (Real Time Clock) 
     * ONLY FOR FIRST TIME! */
    // myrtc.halt(false); // 동작 모드로 설정
    // myrtc.writeProtect(false); // 시간 변경을 가능하게 설정
    // myrtc.setDOW(MONDAY); // 요일 설정
    // myrtc.setTime(11, 22, 0); // 시간 설정 ( 시간, 분, 초 )
    // myrtc.setDate(6, 1, 2024); // 날짜 설정 ( 일, 월, 년도 )
    // myrtc.writeProtect(true); // 시간 변경을 가능하게 설정


    /** Start SD Card Module Initialization */
    logState = 0;
    Serial.print("Initializing SD card...");
    lcd.setCursor(0, 0);
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("SD card...");

    if (!SD.begin(4)) { // Initialize SD Card Module
        Serial.println("Initialization Failed!");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Initialization");
        lcd.setCursor(0, 1);
        lcd.print("Failed!");

        logState = 0;

        while (1);
    }
    else { // When Initialization Success, 
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
    if (init_res_sd == 1) {
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

    if (myFile) {
        // File Open Message
        Serial.print("Writing to >>");
        Serial.print(FILE_PATH);
        Serial.println("<<");

        // Write Files
        Serial.println("UPDATES");

        myFile.println(" ");
        myFile.println(" ");
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
    else { // When File not opened normally
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


void loop() {
    ClockRequest();

    if (init_res_sd == 1 && 
        GetClockSecTime().toInt() == 0
    ) {
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
        (5 <= sec && sec <= 7)
    ) {
        DisplayTemp();
    }
    else if (
        (3 <= sec && sec <= 4) ||
        (8 <= sec && sec <= 9)
    ) {
        DisplayClockSecond();
    }
    DisplayDust();

    delay(900);
}