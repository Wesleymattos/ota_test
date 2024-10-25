#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "FAMÍLIA MATTOS"; // SSID da rede Wi-Fi
const char* password = "1498877676l"; // Senha da rede Wi-Fi
const char* firmware_url = "https://raw.githubusercontent.com/Wesleymattos/ota_test/refs/heads/main/ota_exemplo.ino.bin"; // URL do arquivo binário

bool wifiConnected = false; // Variável para monitorar a conexão Wi-Fi

void setup() {
  Serial.begin(115200); // Inicia a comunicação serial
}

void updateFirmware() {
  HTTPClient http;
  http.begin(firmware_url); // URL do firmware hospedado
  int httpCode = http.GET(); // Faz a requisição GET

  // Verifica se a requisição foi bem-sucedida
  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize(); // Obtém o tamanho do conteúdo
    if (contentLength > 0) {
      bool canBegin = Update.begin(contentLength); // Inicia a atualização
      if (canBegin) {
        WiFiClient *client = http.getStreamPtr(); // Obtém o stream do cliente
        size_t written = Update.writeStream(*client); // Escreve o stream na memória

        // Verifica se a escrita foi completa
        if (written == contentLength) {
          Serial.println("Atualização gravada com sucesso!");
        } else {
          Serial.printf("Falha ao gravar: apenas %d/%d bytes.\n", written, contentLength);
        }

        if (Update.end()) {
          if (Update.isFinished()) {
            Serial.println("Atualização concluída. Reiniciando...");
            ESP.restart(); // Reinicia o ESP32
          } else {
            Serial.println("Erro: atualização não foi finalizada.");
          }
        } else {
          Serial.printf("Erro durante a finalização: %s\n", Update.errorString());
        }
      } else {
        Serial.println("Falha ao iniciar a atualização.");
      }
    } else {
      Serial.println("Erro: Conteúdo inválido.");
    }
  } else {
    Serial.printf("Erro HTTP: %d\n", httpCode); // Exibe erro HTTP
  }
  
  http.end(); // Finaliza a requisição HTTP
}

void connectWiFi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password); // Inicia a conexão ao Wi-Fi

  // Aguarda a conexão e verifica o status
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 100000)) {
    delay(500); // Aguarda meio segundo
    Serial.println("Tentando conectar ao WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true; // Atualiza a variável de conexão
    Serial.println("Conectado ao WiFi!");
  } else {
    Serial.println("Falha ao conectar ao WiFi.");
  }
}

void loop() {
  // Verifica se há dados disponíveis na entrada serial
  if (Serial.available()) {
    char command = Serial.read(); // Lê o caractere recebido

    if (command == 'w') {
      connectWiFi(); // Conecta ao Wi-Fi
    } else if (command == 'f') {
      if (wifiConnected) {
        Serial.println("Iniciando atualização de firmware...");
        updateFirmware(); // Chama a função de atualização de firmware
      } else {
        Serial.println("Não conectado ao WiFi. Conectando...");
        connectWiFi(); // Conecta ao Wi-Fi se não estiver conectado
      }
    } else {
      Serial.println("Comando não reconhecido."); // Mensagem para comandos inválidos
    }
  }

  Serial.println("Estou vivo"); // Indica que o dispositivo está ativo
  delay(1000); // Aguarda um segundo antes de continuar o loop
}
