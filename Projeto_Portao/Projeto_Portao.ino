#include <UIPEthernet.h>
#include <utility/logging.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Servo.h>
Servo myservo;
int pos;    // variable to store the servo position
int pos2;
const int LedLuz = 9;
const int btn = 13;  // esse pino 13 usa na interface internet vou mudar depois mas ele me ajuda testar melhor com o led da placa
int ligado;
int btnState = 0;
int guardaState = 0;
int guardaState2 = 0;
long previousMillis = 0;        // Variável de controle do tempo
long redLedInterval = 0;     // Tempo de espera para o próximo comando do botão
// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x10 };
//the IP address is dependent on your network
IPAddress ip(192, 168, 1, 14);
// the dns server ip
IPAddress dnServer(189, 40, 198, 80);
// the router's gateway address:
IPAddress gateway(192, 168, 1, 1);
// the subnet:
IPAddress subnet(255, 255, 255, 0);
const char* mqtt_server = "m13.cloudmqtt.com";
void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  Serial.print("Message arrived [");
  Serial.print(topicStr);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if (topicStr == "Portao") {
    if (payload[0] == guardaState2) {
      // digitalWrite(LedLuz, LOW);   // Turn the LED on (Note that LOW is the voltage level
      guardaState2 = guardaState2 + 1;
      acionaPortao(guardaState2);
      Serial.println("Abrir Portao");
      // but actually the LED is on; this is because
      // it is acive low on the ESP-01)
    } else {
      //  digitalWrite(LedLuz, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.println("Fechar Portao");
      guardaState2 = guardaState2 + 1;
      acionaPortao(guardaState2);
    }
  }
  else {
    // Se o topico nao for Portao ele vira primeiro pra cá mas nao deve fazer nada.. para direto e ver o próximo if abaixo.
    Serial.println("Não funcionou nao leu o topico");
    acionaPortao(2);
    // client.loop();
  }
  if (topicStr == "Luz") {
    // Switch on the LED if an 1 was received as first character
    if (payload[0] == '0') {
      digitalWrite(LedLuz, HIGH);   // Turn the LED on (Note that LOW is the voltage level
      Serial.println("Desligado");
    } else {
      digitalWrite(LedLuz, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.println("Ligado");
    }
  }
  else {
    Serial.println("Nao acionou o topico Luz");
  }
}
EthernetClient ethClient;
PubSubClient client(mqtt_server, 19927, callback, ethClient);
void setup()
{
  Serial.begin(9600);
  // initialize the ethernet device
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
  myservo.attach(6);
  while (!Serial) {}
  pinMode(LedLuz, OUTPUT);
  pinMode(btn, INPUT);
  // Initialize the BUILTIN_LED pin as an output
  // if (Ethernet.begin(mac, ip, dnServer, gateway, subnet)) {
  //   Serial.println(Ethernet.localIP());
  // } else {
  //  Serial.println("Falha no DHCP");
  // }
  if (client.connect("alsdkjflakj", "teste", "123")) {
    Serial.println("Cheguei aqui");
    Serial.flush();
    //client.publish("Luz", "1111");
    client.subscribe("Luz");
    client.subscribe("Portao");
    client.setCallback(callback);
  } else {
    Serial.println("Não Cheguei aqui");
  }
}
void loop()
{
  unsigned long currentMillis = millis();    //Tempo atual em ms
  btnState = digitalRead(btn);
  if (btnState == 1) {
    if (currentMillis - previousMillis > redLedInterval) {
      previousMillis = currentMillis;    // Salva o tempo atual
      Led();
      guardaState = guardaState + 1; // aqui começa o guardaState
      guardaState2 = guardaState2 + 1;
      acionaPortao(guardaState2);
      // abrir();
    }
    else {
      Serial.println("Aguarde");
    }
  }
  else
  {
    // alguma mensagem ? acho que nao precisa deste else
  }
  Serial.println(btnState);
  Serial.flush();
  client.loop();
}
void acionaPortao(int direcao) {
  // Abrir Portao : direcao = 1
  if (direcao == 1) {
    //  pos = 120; // tira isso daqui depois
    for (pos = 0; pos <= 120; pos += 1) { // goes from 0 degrees to 180 degrees
      myservo.write(pos);
      // delay(100);
      // waits Xms for the servo to reach the position
      Serial.println("Portao Abrindo");
      Serial.flush();
    }
    Serial.println("Portao Aberto");
  }
  // Fechar Portao : direcao = 2
  else if (direcao == 2)  {
    // pos = 60; // tira isso daqui depois
    for (pos2 = pos; pos2 >= 0; pos2--) { // goes from 180 degrees to 0 degrees
      // tell servo to go to position in variable 'pos'
      // delay(100);
      myservo.write(pos2);  // waits 15ms for the servo to reach the position
      Serial.println("Portao Fechando");
      Serial.flush();
    }
    guardaState2 = guardaState2 - 2;
    Serial.println("Portao Fechado");
  }
}
void Led() {
  Serial.print("btnStatus: ");
  Serial.println(guardaState);
  if (guardaState == 1) {
    //ligado = 1 - ligado;
    ligarLampada(1);
  }
  else {
    desligarLampada(0);
  }
}
void ligarLampada(int sim) {
  digitalWrite(LedLuz, HIGH);
  guardaState = guardaState - 2;
  redLedInterval = 3000; // define o tempo de intervalo que a lampada  ficara acesa até o proximo comando
  // reportar o estado do led via MQTT
}
void desligarLampada(int nao) {
  digitalWrite(LedLuz, nao);
  // reportar o estado do led via MQTT
}
