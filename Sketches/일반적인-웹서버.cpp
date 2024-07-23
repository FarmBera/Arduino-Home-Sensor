// 일반적인 웹서버

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(80); // 80번 포트로 서버 생성

const int sensorPin = A0; // 센서를 연결할 아날로그 핀

void setup()
{
    Serial.begin(9600);

    Serial.println("이더넷 초기화 중...");

    if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("이더넷 케이블이 연결되지 않았습니다.");
    }

    // DHCP 시도
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

    // IP 주소 출력
    Serial.print("서버 IP 주소: ");
    Serial.println(Ethernet.localIP());

    // 서버 시작
    server.begin();
    Serial.println("서버가 시작되었습니다.");
}

void loop()
{
    EthernetClient client = server.available();
    if (client)
    {
        Serial.println("새 클라이언트");
        boolean currentLineIsBlank = true;
        String httpRequest = "";

        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                httpRequest += c;

                if (c == '\n' && currentLineIsBlank)
                {
                    int sensorValue = analogRead(sensorPin);

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");
                    client.println("<body>");
                    client.print("Sensor Value: ");
                    client.print(sensorValue);
                    client.println("</body>");
                    client.println("</html>");
                    break;
                }

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

        delay(1);
        client.stop();
        Serial.println("클라이언트 연결 종료");
    }
}