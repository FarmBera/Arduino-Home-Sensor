#include <LiquidCrystal.h> // 일반 LCD Library
#include <LiquidCrystal_I2C.h> // I2C LCD Library
#include <DS1302.h> // RTC Library
// #include <RtcDS1302.h> // RTC Library 2
#include <DHT.h> // 온도/습도 센서
#include <SPI.h> // SD Card Library
#include <SD.h> // SD Card Library


/** 파일 관련 변수 */
String FILE_PATH = "MyRoom.txt";
File myFile; // 열린 파일 저장


/** 온습도 센서 사용 설정 */
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/** RTC (Real Tie CLock) 사용 설정 */
// DS1302 myrtc(11, 9, 8); // Output: RST, DAT, CLK
DS1302 myrtc(9, 8, 7); // Output: RST, DAT, CLK

/** LCD 사용 설정 */
LiquidCrystal_I2C lcdi2c(0x27, 16, 2);
LiquidCrystal lcd(A1,A2, 5,4,3,2); 

/** 미세먼지 측정 관련 변수 */
#define no_dust 0.6
short Vo = A0; // 미세먼지 입력 핀
short V_LED = 6; // 적외선 센서 출력핀
float Vo_value = 0;
float Voltage = 0; // 측정된 전압값
float dustDensity = 0; // 최종 미세먼지 계산

// 날짜 변수
String c_DATES = "";
short c_year = 0;
short c_month = 0;
short c_date = 0;
// 시계 변수
String c_CLOCK = "";
short c_hour = 0;
short c_min = 0;
short c_sec = 0;
// 온도/습도 변수
short dht_temp = 0;
short dht_hum = 0;
String lvl = "";


/** 모든 LCD 화면 지우기 */
void ClearLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR");
    lcdi2c.clear();
    lcdi2c.setCursor(0, 0);
    lcdi2c.print("ERROR");
}


/** 시계 출력하기 */
void DisplayClock(short pos=0, char TypeLCD='d') {
    String TIME = myrtc.getTimeStr();
    TIME = TIME.substring(0, 5); // .toInt()

    if (TypeLCD == 'd') {
        lcd.setCursor(0, pos);
        // lcd.print(myrtc.getTimeStr());
        lcd.print(TIME);
        lcd.print(" ");
        lcd.print(myrtc.getDateStr());
    }
    else if (TypeLCD == 'i') {
        lcdi2c.setCursor(0, pos);
        // lcdi2c.print(myrtc.getTimeStr());
        lcdi2c.print(TIME);
        lcdi2c.print(" ");
        lcdi2c.print(myrtc.getDateStr());
    }
    else {
        ClearLCD();
    }
}


/** 전체 화면 시계 출력하기 */
void DisplayClockFull(short pos = 0, char TypeLCD = 'd') {
    String TIME = myrtc.getTimeStr();
    String DATE = myrtc.getDateStr();


    if (TIME == c_CLOCK) return;
    c_CLOCK = TIME;


    if (TypeLCD == 'd') {
        c_CLOCK = TIME;
        lcd.setCursor(4, 0);
        lcd.print(c_CLOCK);
    }
    else if (TypeLCD == 'i') {
        lcdi2c.setCursor(4, 0);
        lcdi2c.print(TIME);
    }
    else {
        ClearLCD();
        return;
    }


    if (DATE == c_DATES) return;
    c_DATES = DATE;


    if (TypeLCD == 'd') {
        lcd.setCursor(3, 1);
        lcd.print(DATE);
    }
    else if (TypeLCD == 'i') {
        lcdi2c.setCursor(3, 1);
        lcdi2c.print(DATE);
    }
    else {
        ClearLCD();
        return;
    }
}


String GetClockSecTime() {
    String TIME = myrtc.getTimeStr();
    String TEMP = "";
    TEMP += TIME.substring(6, 8);
    // Serial.print(TEMP);
    return TEMP;
}


/** 미세먼지 출력 화면 초기설정 */
void DisplayInitTemp(char TypeLCD='d') {
    if (TypeLCD == 'd') {
        lcd.setCursor(0, 0);
        lcd.print("T: ");
        // lcd.print(t);
        lcd.setCursor(5, 0);
        lcd.print("'C");
        lcd.setCursor(9, 0);
        lcd.print("H: ");

        lcd.setCursor(14, 0);
        lcd.print("%");
    }
    else if (TypeLCD == 'i') {
        lcdi2c.setCursor(0, 0);
        lcdi2c.print("T: ");
        // lcdi2c.print(t);
        lcdi2c.setCursor(5, 0);
        lcdi2c.print("'C");
        lcdi2c.setCursor(9, 0);
        lcdi2c.print("H: ");

        lcdi2c.setCursor(14, 0);
        lcdi2c.print("%");
    }
    else {
        ClearLCD();
    }
}


/** 미세먼지 출력 화면 초기설정 */
void DisplayInitDust(char TypeLCD='d') {
    // LCD에 출력하기
    if (TypeLCD == 'd') {
        lcd.setCursor(0, 1);
        lcd.print("V");
    }
    else if (TypeLCD == 'i') {
        lcdi2c.setCursor(0, 1);
        // lcdi2c.print("V: "); // 기존
        lcdi2c.print("V");
        // lcdi2c.setCursor(9, 1);
        // lcdi2c.print("D: ");
        // lcdi2c.print("D");
    }
    else {
        ClearLCD();
    }
}


/** 온도/습도 측정 및 표시 */
void DisplayTemperature(char TypeLCD='d') {
    // 측정값 불러오기
    int t = dht.readTemperature();
    int h = dht.readHumidity();

    // Serial.print("ERROR");
    // Serial.print(t);
    // Serial.print(" ");
    // Serial.println(h);

    /** 조건 판별
     * 저장된 값이랑 맞지 않는다면
     * LCD 값 새로고침
    */

    // 온도 확인
    if (TypeLCD == 'd') {
        if (dht_temp != t) {
            dht_temp = t;
            lcd.setCursor(3, 0);
            if (dht_temp < 10) {
                lcd.print("  ");
                lcd.setCursor(3, 0);
            }
            lcd.print(dht_temp);
        }
    }
    else if (TypeLCD == 'i') {
        dht_temp = t;
        lcdi2c.setCursor(3, 0);
        if (dht_temp < 10) {
            lcdi2c.print("  ");
            lcdi2c.setCursor(3, 0);
        }
        lcdi2c.print(dht_temp);
    }
    else {
        ClearLCD();
        Serial.println("ERROR Temp");
        return;
    }


    // 습도 확인
    if (TypeLCD == 'd') {
        if (dht_hum != h) {
            dht_hum = h;
            if (TypeLCD == 'd') {
                lcd.setCursor(12, 0);
                lcd.print(dht_hum);
            }
        }
    }
    else if (TypeLCD == 'i') {
        dht_hum = h;
        lcdi2c.setCursor(12, 0);
        lcdi2c.print(dht_hum);
    }
    else {
        ClearLCD();
        Serial.println("ERROR Humidity");
        return;
    }
}


/** 미세먼지 측정하기 */
void DisplayDust(char TypeLCD='d') {
    digitalWrite(V_LED, LOW);
    delayMicroseconds(280);
    Vo_value = analogRead(Vo);
    delayMicroseconds(40);
    digitalWrite(V_LED, HIGH);
    delayMicroseconds(9680);

    // 센서 전압값 계산
    float new_Voltage = Vo_value * 5.0 / 1024.0; // 아날로그 값을 전압값으로 Change

    // 미세먼지 계산하기
    // dustDensity = 30 + 5 * ( (new_Voltage - 0.8) * 10 ); // SOSO
    float new_DS = (new_Voltage - no_dust) / 0.005; // NEW

    // 전압값 표시
    if (Voltage != new_Voltage) {
        // Serial.println("New Voltage!");
        Voltage = new_Voltage;
        // lcdi2c.setCursor(3, 1); 

        if (TypeLCD == 'd') {
            lcd.setCursor(1, 1);
            lcd.print(Voltage);
        }
        else if (TypeLCD == 'i') {
            lcdi2c.setCursor(1, 1);
            lcdi2c.print(Voltage);
        }
        else {
            ClearLCD();
            return;
        }
    }

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
    String new_lvl = "";
    if (0.0 < new_DS && new_DS < 15.0)
        new_lvl = "Perf";
    else if (15.0 <= new_DS && new_DS < 30.0)
        new_lvl = "Good";
    else if (30.0 <= new_DS && new_DS < 40.0)
        new_lvl = "soso";
    else if (40.0 <= new_DS && new_DS < 50.0)
        new_lvl = "Norml";
    else if (50.0 <= new_DS && new_DS < 75.0)
        new_lvl = "bad.";
    else if (75.0 <= new_DS && new_DS < 100.0)
        new_lvl = "VBAD";
    else if (100.0 <= new_DS && new_DS < 150.0)
        new_lvl = "Danger";
    else if (150 <= new_DS)
        new_lvl = "WORST";
    else
        new_lvl = "ERROR";
    
    // lcdi2c.setCursor(10, 1); // 레벨
    if (TypeLCD == 'd') {
        lcd.setCursor(6, 1);
    }
    else if (TypeLCD == 'i') {
        lcdi2c.setCursor(6, 1);
    }
    else {
        ClearLCD();
        return;
    }

    if (lvl != new_lvl) {
        // Serial.println("New Level !");
        lvl = "      ";

        if (TypeLCD == 'd') lcd.print(lvl);
        else if (TypeLCD == 'i') lcdi2c.print(lvl);
        lvl = new_lvl;

        if (TypeLCD == 'd') {
            lcd.setCursor(6, 1);
            lcd.print(lvl);
        }
        else if (TypeLCD == 'i') {
            lcdi2c.setCursor(6, 1);
            lcdi2c.print(lvl);
        }
        else {
            ClearLCD();
            return;
        }
    }

    if (dustDensity != new_DS) {
        // Serial.println("New DustDensity !");
        dustDensity = new_DS;

        if (TypeLCD == 'd') {
            lcd.setCursor(12, 1); // 미세먼지 값
            lcd.print(dustDensity);
        }
        else if (TypeLCD == 'i') {
            lcdi2c.setCursor(12, 1); // 미세먼지 값
            lcdi2c.print(dustDensity);
        }
        else {
            ClearLCD();
            return;
        }
    }
}


/** 디스플레이 초기화 */
void DisplayInit(char TypeLCD='d') {
    DisplayInitTemp(TypeLCD);
    DisplayInitDust(TypeLCD);
}


/** 화면에 표시하기 */
void Display(char TypeLCD='d') {
    DisplayTemperature(TypeLCD);
    DisplayDust(TypeLCD);
}


/** SD 카드에 내용 기록하기 */
void SDCardLogging() {
    myFile = SD.open(FILE_PATH, FILE_WRITE); // 두 번째 인자가 있으면 쓰기모드입니다.

    String RESULTStr = "";

    if (myFile) { // 파일이 정상적으로 열리면 // 파일 작성
        // File Open Message
        Serial.print("Writing to >>");
        Serial.print(FILE_PATH);
        Serial.println("<<");

        // Write Files
        RESULTStr += myrtc.getDateStr();
        RESULTStr += " ";
        RESULTStr += myrtc.getTimeStr();
        RESULTStr += " >> ";
        RESULTStr += dht_temp;
        RESULTStr += " ";
        RESULTStr += dht_hum;
        RESULTStr += " ";
        RESULTStr += Voltage;
        RESULTStr += " ";
        RESULTStr += dustDensity;
        // RESULTStr += "";
        Serial.println(RESULTStr);
        
        // Serial.print(myrtc.getDateStr());
        // Serial.print(" ");
        // Serial.print(myrtc.getTimeStr());
        // Serial.print(" >> ");
        // Serial.print(dht_temp);
        // Serial.print(" ");
        // Serial.print(dht_hum);
        // Serial.print(" ");
        // Serial.print(Voltage);
        // Serial.print(" ");
        // Serial.print(dustDensity);
        // Serial.println("");

        // myFile.println(RESULTStr);
        myFile.println(RESULTStr);
        myFile.close();

        // Complete Message
        Serial.println("Writing Done.");
    }
    else { // 파일 안열리면 오류 출력
        Serial.print("Error with Opening >> ");
        Serial.println(FILE_PATH);
    }
}


/** 시리얼 모니터로 입력 받기 */
void CheckSerialInput() {
    
}


void setup() {
    Serial.begin(9600);

    lcd.begin(16, 2);

    lcdi2c.init();
    lcdi2c.clear();
    lcdi2c.backlight();

    /** 미세먼지 핀 입력/출력 설정 */
    pinMode(V_LED, OUTPUT);
    pinMode(Vo, INPUT);

    /** RTC (Real Time Clock) 시간 동기화
     * 최초 설정할 때만 실행함
    */
    // myrtc.halt(false); // 동작 모드로 설정
    // myrtc.writeProtect(false); // 시간 변경을 가능하게 설정
    // myrtc.setDOW(MONDAY); // 요일 설정
    // myrtc.setTime(13, 42, 0); // 시간 설정 ( 시간, 분, 초 )
    // myrtc.setDate(26, 12, 2023); // 날짜 설정 ( 일, 월, 년도 )
    // myrtc.writeProtect(true); // 시간 변경을 가능하게 설정


    /** SD 카드 라이브러리 초기화 */
    Serial.print("Initializing SD card...");

    lcd.setCursor(0, 0);
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("SD card...");

    lcdi2c.setCursor(0, 0);
    lcdi2c.print("Initializing");
    lcdi2c.setCursor(0, 1);
    lcdi2c.print("SD card...");
    
    // SD카드 모듈 초기화
    if (!SD.begin(4)) {
        Serial.println("initialization Failed!"); // SD카드 모듈 초기화 실패하면 에러 출력
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("initialization"); // SD카드 모듈 초기화 실패하면 에러 출력
        lcd.setCursor(0, 1);
        lcd.print("Failed!");

        lcdi2c.clear();
        lcdi2c.setCursor(0, 0);
        lcdi2c.print("initialization"); // SD카드 모듈 초기화 실패하면 에러 출력
        lcdi2c.setCursor(0, 1);
        lcdi2c.print("Failed!");
        while (1);
    }
    Serial.println("initialization done.");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("initialization");
    lcd.setCursor(0, 1);
    lcd.print("SD Card DONE !");

    lcdi2c.clear();
    lcdi2c.setCursor(0, 0);
    lcdi2c.print("initialization");
    lcdi2c.setCursor(0, 1);
    lcdi2c.print("SD Card DONE !");
    delay(2 * 1000);
    lcd.clear();
    lcdi2c.clear();


    /** 로그 시작 메시지 띄우기 */
    lcd.print("Logging Start!");
    lcdi2c.print("Logging Start!");
    DisplayClock(1, 'd');
    DisplayClock(1, 'i');
    delay(2 * 1000);

    myFile = SD.open(FILE_PATH, FILE_WRITE); // 두 번째 인자가 있으면 쓰기모드입니다.

    String RESULTStr = "";

    if (myFile) { // 파일이 정상적으로 열리면 // 파일 작성
        // File Open Message
        Serial.print("Writing to >>");
        Serial.print(FILE_PATH);
        Serial.println("<<");

        // Write Files
        Serial.println("\n\nUPDATES");
        myFile.println(RESULTStr);
        myFile.close();
    }
    else { // 파일 안열리면 오류 출력
        Serial.print("ERROR with Opening >> ");
        Serial.println(FILE_PATH);
        lcd.setCursor(0, 0);
        lcd.print("ERROR with");
        lcd.print("Opening");
        lcd.print(FILE_PATH);
        lcdi2c.setCursor(0, 0);
        lcdi2c.print("ERROR with");
        lcdi2c.print("Opening");
        lcdi2c.print(FILE_PATH);
    }
    lcd.clear();
    lcdi2c.clear();

    
    // lcd.setCursor(0, 0);
    // lcd.print("asdf");

    
    /** 기본 뼈대 화면 만들기 */
    // DisplayInitTemp();
    // DisplayInitDust();
    DisplayInitTemp('i');
    DisplayInitDust('i');
}

void loop() {
    // Display('i');
    // DisplayClock();
    // DisplayClockFull();

    // lcdi2c.clear();
    for (int x=0; x<3; x++) {
        lcdi2c.setCursor(0, 0);
        for (int i=0; i<16; i++) {
            lcdi2c.print(" ");
        }
        DisplayInitTemp('i');

        for (int i=0; i<4; i++) {
            Display('i');
            delay(1 * 1000);
        }
        for (int i=0; i<2; i++) {
            DisplayClock(0, 'i');
            DisplayDust('i');
            delay(1 * 1000);
        }
    }
    
    // lcdi2c.clear();
    SDCardLogging();

    // if (GetClockSecTime() == "00") {
    //     SDCardLogging();
    // }


    // delay(500);
    // delay( 1 * 1000 ); // 1 sec
    // delay( 3 * 1000 ); // 3 sec
    // delay( 5 * 1000 ); // 5 sec
    // delay( 10 * 1000 ); // 10 sec
    // delay( 30 * 1000 ); // 30 sec
}