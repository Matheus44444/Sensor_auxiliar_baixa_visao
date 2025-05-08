// Código para o ESP32 da Bandana (Receptor ESP-NOW) - MODIFICADO para Sensor IR
#include <esp_now.h>
#include <WiFi.h>

// --- Definições de Pinos (GPIO Indiretos) ---
#define MOTOR_FRENTE_PIN 13
#define MOTOR_ESQUERDA_PIN 12
#define MOTOR_DIREITA_PIN 14
#define MOTOR_INFERIOR_IR_PIN 27

#define FREQ_MIN_ALERT 500
#define FREQ_MAX_ALERT 2000

#define DIST_MAX_PROPORTIONAL_ALERT 150
#define DIST_MIN_PROPORTIONAL_ALERT 5

// --- Estrutura de Dados para ESP-NOW ---
typedef struct struct_message
{
  float distancia_frente;
  float distancia_esquerda;
  float distancia_direita;

  bool obstaculo_frente;
  bool obstaculo_esquerda;
  bool obstaculo_direita;
  bool obstaculo_inferior_ir;
} struct_message;

struct_message dadosRecebidos;

// --- Função de Callback para ESP-NOW ---
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
  Serial.print("Dados recebidos de: ");
  for (int i = 0; i < 6; i++)
  {
    Serial.print(info->src_addr[i], HEX);
    if (i < 5)
      Serial.print(":");
  }
  Serial.println();

  memcpy(&dadosRecebidos, incomingData, sizeof(dadosRecebidos));
  Serial.println("Conteúdo dos dados:");
  Serial.print("Frente: ");
  Serial.print(dadosRecebidos.distancia_frente);
  Serial.print(" cm, Obstáculo: ");
  Serial.println(dadosRecebidos.obstaculo_frente);
  Serial.print("Esquerda: ");
  Serial.print(dadosRecebidos.distancia_esquerda);
  Serial.print(" cm, Obstáculo: ");
  Serial.println(dadosRecebidos.obstaculo_esquerda);
  Serial.print("Direita: ");
  Serial.print(dadosRecebidos.distancia_direita);
  Serial.print(" cm, Obstáculo: ");
  Serial.println(dadosRecebidos.obstaculo_direita);
  // Serial.print("Trás/Baixo: "); Serial.print(dadosRecebidos.distancia_tras); // Removido
  Serial.print("IR Inferior Obstáculo: ");
  Serial.println(dadosRecebidos.obstaculo_inferior_ir);
  Serial.println("-------------------------------------");

  if (dadosRecebidos.obstaculo_frente)
  {
    float current_dist_cm = dadosRecebidos.distancia_frente;
    long frequency;

    frequency = map((long)(current_dist_cm * 10),
                    (long)(DIST_MAX_PROPORTIONAL_ALERT * 10),
                    (long)(DIST_MIN_PROPORTIONAL_ALERT * 10),
                    FREQ_MIN_ALERT,
                    FREQ_MAX_ALERT);

    frequency = constrain(frequency, FREQ_MIN_ALERT, FREQ_MAX_ALERT);

    tone(MOTOR_FRENTE_PIN, frequency);
    Serial.print("ALERTA FRONTAL PROPORCIONAL! Dist: ");
    Serial.print(current_dist_cm);
    Serial.print(" cm, Freq: ");
    Serial.println(frequency);
  }
  else
  {
    noTone(MOTOR_FRENTE_PIN);
  }

  if (dadosRecebidos.obstaculo_esquerda)
  {
    float current_dist_cm = dadosRecebidos.distancia_esquerda;
    long frequency;

    frequency = map((long)(current_dist_cm * 10),
                    (long)(DIST_MAX_PROPORTIONAL_ALERT * 10),
                    (long)(DIST_MIN_PROPORTIONAL_ALERT * 10),
                    FREQ_MIN_ALERT,
                    FREQ_MAX_ALERT);

    frequency = constrain(frequency, FREQ_MIN_ALERT, FREQ_MAX_ALERT);

    tone(MOTOR_ESQUERDA_PIN, frequency);
    Serial.print("ALERTA ESQUERDO PROPORCIONAL! Dist: ");
    Serial.print(current_dist_cm);
    Serial.print(" cm, Freq: ");
    Serial.println(frequency);
  }
  else
  {
    noTone(MOTOR_ESQUERDA_PIN);
  }

  if (dadosRecebidos.obstaculo_direita)
  {
    float current_dist_cm = dadosRecebidos.distancia_direita;
    long frequency;

    frequency = map((long)(current_dist_cm * 10),
                    (long)(DIST_MAX_PROPORTIONAL_ALERT * 10),
                    (long)(DIST_MIN_PROPORTIONAL_ALERT * 10),
                    FREQ_MIN_ALERT,
                    FREQ_MAX_ALERT);
    frequency = constrain(frequency, FREQ_MIN_ALERT, FREQ_MAX_ALERT);

    tone(MOTOR_DIREITA_PIN, frequency);
    Serial.print("ALERTA DIREITA PROPORCIONAL! Dist: ");
    Serial.print(current_dist_cm);
    Serial.print(" cm, Freq: ");
    Serial.println(frequency);
  }
  else
  {
    noTone(MOTOR_DIREITA_PIN);
  }

  if (dadosRecebidos.obstaculo_inferior_ir)
  {
    digitalWrite(MOTOR_INFERIOR_IR_PIN, HIGH);
    Serial.println("ALERTA: Obstáculo INFERIOR (IR)!");
  }
  else
  {
    digitalWrite(MOTOR_INFERIOR_IR_PIN, LOW);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(MOTOR_FRENTE_PIN, OUTPUT);
  pinMode(MOTOR_ESQUERDA_PIN, OUTPUT);
  pinMode(MOTOR_DIREITA_PIN, OUTPUT);
  pinMode(MOTOR_INFERIOR_IR_PIN, OUTPUT);

  digitalWrite(MOTOR_FRENTE_PIN, LOW);
  digitalWrite(MOTOR_ESQUERDA_PIN, LOW);
  digitalWrite(MOTOR_DIREITA_PIN, LOW);
  digitalWrite(MOTOR_INFERIOR_IR_PIN, LOW);

  WiFi.mode(WIFI_STA);
  Serial.print("Endereço MAC deste ESP32 (Receptor): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP32 Bandana (Receptor com IR) configurado e aguardando dados...");
}

void loop()
{
  delay(100);
}
