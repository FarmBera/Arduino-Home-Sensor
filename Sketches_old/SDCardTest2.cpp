#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
    Serial.begin(9600);
    Serial.print("Initializing SD card...");

    if (!SD.begin(4)) {// SD카드 모듈을 초기화합니다.
        Serial.println("initialization failed!"); // SD카드 모듈 초기화에 실패하면 에러를 출력합니다.
        while (1);
    }
    Serial.println("initialization done.");

    // 파일을 열어 쓸 준비를 합니다. 한 번에 하나의 파일만 열 수 있습니다.
    myFile = SD.open("test.txt", FILE_WRITE); // 두 번째 인자가 있으면 쓰기모드입니다.

    if (myFile) { // 파일이 정상적으로 열리면 파일에 문자를 작성(추가)합니다.
        String one = "1One";
        String two = "2Two";
        Serial.print("Writing to test.txt...");
        myFile.println("Hello, mechasolution!");
        myFile.println(one + " middle " + two);
        myFile.close(); // 파일을 닫습니다.
        Serial.println("done.");
    }
    else {
        // 파일이 열리지 않으면 에러를 출력합니다.
        Serial.println("ERROR with Opening test.txt");
    }

    // 파일을 읽기 위해 다시 엽니다. 두 번째 인자가 없으면 읽기모드입니다.
    myFile = SD.open("test.txt");

    if (myFile) {
        Serial.println("test.txt:");

        // while문을 통해 파일을 EOF(End-Of-File)까지 읽습니다.
        while (myFile.available()) {
            Serial.write(myFile.read()); // 읽을 파일이 있다면 시리얼로 출력합니다.
        }
        myFile.close(); // 파일을 닫습니다.
    }
    else {
        // 파일이 열리지 않으면 에러를 출력합니다.
        Serial.println("error opening test.txt");
    }
}

void loop() {}