#include <SPI.h>
#include <Ethernet.h>

// MAC 주소 설정
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// 서버 포트 설정
EthernetServer server(80);

// 센서 핀 설정
const int sensorPin = A0;

void setup()
{
  // 시리얼 통신 시작
  Serial.begin(9600);
  Serial.println("이더넷 초기화 중...");

  // 이더넷 링크 상태 확인
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("이더넷 케이블이 연결되지 않았습니다.");
  }

  // DHCP를 통해 IP 주소 요청
  Serial.println("DHCP를 통해 IP 주소 요청 중...");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP로 구성 실패. 정적 IP 사용 시도...");
    IPAddress ip(192, 168, 0, 177);
    IPAddress dns(8, 8, 8, 8);
    IPAddress gateway(192, 168, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, dns, gateway, subnet);
  }

  // 서버 IP 주소 출력
  Serial.print("서버 IP 주소: ");
  Serial.println(Ethernet.localIP());

  // 서버 시작
  server.begin();
  Serial.println("서버가 시작되었습니다.");
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
            int sensorValue = analogRead(sensorPin);

            // HTTP 응답 전송
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            client.print("{\"sensorValue\": ");
            client.print(sensorValue);
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
        {
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          currentLineIsBlank = false;
        }
      }
    }

    // 연결 종료
    delay(1);
    client.stop();
    Serial.println("클라이언트 연결 종료");
  }
}
