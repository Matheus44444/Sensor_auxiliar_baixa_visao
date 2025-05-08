#include <esp_now.h>
#include <WiFi.h>
#include <Ultrasonic.h>

// --- Definições de Pinos (GPIO Indiretos) ---

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// Sensores Ultrassônicos
#define US_FRENTE_TRIGGER_PIN 13
#define US_FRENTE_ECHO_PIN 12
#define US_ESQUERDA_TRIGGER_PIN 14
#define US_ESQUERDA_ECHO_PIN 27
#define US_DIREITA_TRIGGER_PIN 26
#define US_DIREITA_ECHO_PIN 25

#define IR_INFERIOR_ANALOG_PIN 32

// --- Instâncias dos Sensores ---
Ultrasonic ultrasonic_frente(US_FRENTE_TRIGGER_PIN, US_FRENTE_ECHO_PIN);
Ultrasonic ultrasonic_esquerda(US_ESQUERDA_TRIGGER_PIN, US_ESQUERDA_ECHO_PIN);
Ultrasonic ultrasonic_direita(US_DIREITA_TRIGGER_PIN, US_DIREITA_ECHO_PIN);

// --- Estrutura de Dados para ESP-NOW ---
typedef struct struct_message
{
  float distancia_frente;
  l float distancia_esquerda;
  float distancia_direita;

  bool obstaculo_frente;
  bool obstaculo_esquerda;
  bool obstaculo_direita;
  bool obstaculo_inferior_ir;
} struct_message;

struct_message dadosSensores;

// --- Configurações ESP-NOW ---
esp_now_peer_info_t peerInfo;

// --- Limiares de Detecção ---
#define LIMIAR_OBSTACULO_ULTRASONICO 50.0
#define LIMIAR_SENSOR_IR_ANALOGICO 3000

// Callback executado quando os dados são enviados
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nStatus do último pacote enviado:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entregue com Sucesso" : "Falha na Entrega");
}

void setup()
{
  Serial.begin(115200);

  // Configura o dispositivo como uma estação Wi-Fi
  WiFi.mode(WIFI_STA);

  // Inicializa o ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }

  // Registra o callback de envio
  esp_now_register_send_cb(OnDataSent);

  // Registra o peer (receptor)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Falha ao adicionar peer");
    return;
  }
  pinMode(IR_INFERIOR_ANALOG_PIN, INPUT);

  Serial.println("ESP32 Bengala (Transmissor com IR) configurado.");
}

void loop()
{
  // --- Leitura dos Sensores Ultrassônicos ---
  dadosSensores.distancia_frente = ultrasonic_frente.read();
  dadosSensores.distancia_esquerda = ultrasonic_esquerda.read();
  dadosSensores.distancia_direita = ultrasonic_direita.read();

  dadosSensores.obstaculo_frente = dadosSensores.distancia_frente < LIMIAR_OBSTACULO_ULTRASONICO && dadosSensores.distancia_frente > 0;
  dadosSensores.obstaculo_esquerda = dadosSensores.distancia_esquerda < LIMIAR_OBSTACULO_ULTRASONICO && dadosSensores.distancia_esquerda > 0;
  dadosSensores.obstaculo_direita = dadosSensores.distancia_direita < LIMIAR_OBSTACULO_ULTRASONICO && dadosSensores.distancia_direita > 0;

  // --- Leitura do Sensor Infravermelho (Analógico) ---
  int valor_analogico_ir = analogRead(IR_INFERIOR_ANALOG_PIN);
  dadosSensores.obstaculo_inferior_ir = valor_analogico_ir < LIMIAR_SENSOR_IR_ANALOGICO;

  // --- Envio dos Dados via ESP-NOW ---
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&dadosSensores, sizeof(dadosSensores));

  if (result == ESP_OK)
  {
    // Serial.println("Dados enviados com sucesso");
  }
  else
  {
    Serial.println("Erro ao enviar os dados");
  }

  // --- Impressão no Serial Monitor para Debug ---
  Serial.print("Frente: ");
  Serial.print(dadosSensores.distancia_frente);
  Serial.print(" cm, Obstáculo: ");
  Serial.println(dadosSensores.obstaculo_frente);
  Serial.print("Esquerda: ");
  Serial.print(dadosSensores.distancia_esquerda);
  Serial.print(" cm, Obstáculo: ");
  Serial.println(dadosSensores.obstaculo_esquerda);
  Serial.print("Direita: ");
  Serial.print(dadosSensores.distancia_direita);
  Serial.print(" cm, Obstáculo: ");
  Serial.println(dadosSensores.obstaculo_direita);
  Serial.print("IR Inferior (Valor Analógico): ");
  Serial.print(valor_analogico_ir);
  Serial.print(", Obstáculo Detectado: ");
  Serial.println(dadosSensores.obstaculo_inferior_ir);
  Serial.println("-------------------------------------");

  delay(200);
}
