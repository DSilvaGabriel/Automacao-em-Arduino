// 🧠 Estas são chamadas de "bibliotecas". 
// Imagine que são caixinhas prontas com funções especiais.
// Essas caixinhas ajudam a ESP32 (nosso cérebro do robô) a entender como usar o controle do PS4, os servos e a memória interna.
#include <PS4Controller.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// 📌 Aqui estamos dando nomes aos "pinos" da placa ESP32.
// Esses pinos são como "tomadas" onde ligamos fios dos motores.
// Um "pino" é uma entrada ou saída de energia ou sinais na placa.
// Quando ligamos fios nos pinos, podemos controlar motores, LEDs, sensores etc.
#define motor_direita_frente 26  // Pino que faz a roda direita girar para frente
#define motor_direita_tras 27    // Pino que faz a roda direita girar para frente
#define motor_esquerda_frente 25 // Pino que faz a roda esquerda girar para frente
#define motor_esquerda_tras 14   // Pino que faz a roda esquerda girar para frente

#define canal_servo_direita 33
#define canal_servo_esquerda 32

#define canal_servo_lapis 18



#define canal_servo_garraD 13
#define canal_servo_garraE 12

Servo servo_direita;
Servo servo_esquerda;

Servo servo_lapis;



Servo servo_garraD;
Servo servo_garraE;

int valorD = 0;
int valorE = 180;

int valorGD = 0;
int valorGE = 180;



enum Movimento { PARADO, FRENTE, TRAS, ESQUERDA, DIREITA };

struct Comando {
  Movimento movimento;
  unsigned long duracao;
};

Comando comandosTemp[100];
Comando comandosPermanente[100];

int indexTemp = 0;
int indexPermanente = 0;

bool gravando = false;
bool gravandoToggleAnterior = false;
unsigned long tempoAnterior = 0;
Movimento ultimoMovimento = PARADO;

Preferences prefs;

void frente(), tras(), direita(), esquerda(), parada();
void gravarMovimento(Movimento mov);
void salvarRota();
void carregarRota();
void apagarRota();
void executarComandos(Comando comandos[], int quantidade);  // ADICIONADA A DECLARAÇÃO

void setup() {
  Serial.begin(115200);
  PS4.begin("f8:da:0c:d2:8c:aa");
  Serial.println("Pronto");

  pinMode(motor_direita_frente, OUTPUT);
  pinMode(motor_direita_tras, OUTPUT);
  pinMode(motor_esquerda_frente, OUTPUT);
  pinMode(motor_esquerda_tras, OUTPUT);

    servo_direita.setPeriodHertz(50);
  servo_direita.attach(canal_servo_direita, 500, 2400);
  servo_esquerda.setPeriodHertz(50);
  servo_esquerda.attach(canal_servo_esquerda, 500, 2400);

  servo_lapis.setPeriodHertz(50);
  servo_lapis.attach(canal_servo_lapis, 500, 2400);
  
  servo_garraD.setPeriodHertz(50);
  servo_garraD.attach(canal_servo_garraD, 500, 2400);
  servo_garraE.setPeriodHertz(50);
  servo_garraE.attach(canal_servo_garraE, 500, 2400);

  // 🆕 Envia os servos automaticamente para a posição 0° ao iniciar
  servo_direita.write(0);
  servo_esquerda.write(180);
  servo_garraD.write(0);
  servo_garraE.write(180);
  servo_lapis.write(90);
  Serial.println("Servos posicionados em 0 graus");

  prefs.begin("robo", false);
  carregarRota();
}

void loop() {
  if (PS4.isConnected()) {
    if (PS4.Triangle() && !gravandoToggleAnterior) {
      gravando = !gravando;
      if (gravando) {
        Serial.println("Iniciando gravação...");
        indexTemp = 0;
        tempoAnterior = millis();
        ultimoMovimento = PARADO;
      } else {
        Serial.println("Finalizando gravação.");
        if (ultimoMovimento != PARADO && indexTemp < 100) {
          comandosTemp[indexTemp].movimento = ultimoMovimento;
          comandosTemp[indexTemp].duracao = millis() - tempoAnterior;
          indexTemp++;
        }
        ultimoMovimento = PARADO;
      }
      delay(500);
    }
    gravandoToggleAnterior = PS4.Triangle();

    if (PS4.Options()) {
      Serial.println("Executando comandos temporários...");
      executarComandos(comandosTemp, indexTemp);
      delay(1000);
    }

    if (PS4.Circle()) {
      Serial.println("Salvando como rota permanente.");
      salvarRota();
      delay(1000);
    }

    if (PS4.Square()) {
      Serial.println("Executando rota permanente...");
      executarComandos(comandosPermanente, indexPermanente);
      delay(1000);
    }

    if (PS4.Share()) {
      Serial.println("Apagando gravação permanente...");
      apagarRota();
      delay(1000);
    }

    Movimento movAtual = PARADO;
    if (PS4.LStickY() > 20) {
      frente();
      movAtual = FRENTE;
    } else if (PS4.LStickY() < -20) {
      tras();
      movAtual = TRAS;
    } else if (PS4.RStickX() > 20) {
      esquerda();
      movAtual = ESQUERDA;
    } else if (PS4.RStickX() < -20) {
      direita();
      movAtual = DIREITA;
    } else {
      parada();
      movAtual = PARADO;
    }

    if (gravando) {
      gravarMovimento(movAtual);
    }
  if (PS4.R2()) {
        if(valorD < 100){
        valorD = valorD + 1;
        valorE = valorE - 1;
        }
      servo_direita.write(valorD);
      servo_esquerda.write(valorE);
      Serial.println("Servos movidos para 0 graus");
      delay (10);
    }
    if (PS4.L2()) {
      if(valorD > 0){
        valorD = valorD - 1;
        valorE = valorE + 1;
      }
      servo_direita.write(valorD);
      servo_esquerda.write(valorE);
      Serial.println("Servos movidos para 0 graus");
      delay (10);
    }
    if (PS4.R1()) {
      if(valorGD < 100){
        valorGD = valorGD + 1;
        valorGE = valorGE - 1;
      }
      servo_garraD.write(valorGD);
      servo_garraE.write(valorGE);
      Serial.println("Servos movidos para 0 graus");
      delay (10);
    }
    if (PS4.L1()) {
      if(valorGD > 0){
        valorGD = valorGD - 1;
        valorGE = valorGE + 1;
      }
      servo_garraD.write(valorGD);
      servo_garraE.write(valorGE);
      Serial.println("Servos movidos para 0 graus");
      delay (10);
    }
  if (PS4.Right()) {
        servo_lapis.write(90);
      }
      if (PS4.Down()) {
        servo_lapis.write(180);
      }
  
  }
}

void gravarMovimento(Movimento movAtual) {
  if (movAtual != ultimoMovimento) {
    if (ultimoMovimento != PARADO && indexTemp < 100) {
      comandosTemp[indexTemp].movimento = ultimoMovimento;
      comandosTemp[indexTemp].duracao = millis() - tempoAnterior;
      indexTemp++;
    }
    tempoAnterior = millis();
    ultimoMovimento = movAtual;
  }
}

void executarComandos(Comando comandos[], int tamanho) {
  for (int i = 0; i < tamanho; i++) {
    if (PS4.Cross()) {
      Serial.println("Execução interrompida pelo botão X");
      parada();
      break;
    }

    switch (comandos[i].movimento) {
      case FRENTE: frente(); break;
      case TRAS: tras(); break;
      case DIREITA: direita(); break;
      case ESQUERDA: esquerda(); break;
      default: parada(); break;
    }

    unsigned long inicio = millis();
    while (millis() - inicio < comandos[i].duracao) {
      if (PS4.Cross()) {
        Serial.println("Execução interrompida pelo botão X");
        parada();
        return;
      }
      delay(10);
    }

    parada();
    delay(100);
  }
  parada();
}

void salvarRota() {
  prefs.putInt("qtd", indexTemp);
  for (int i = 0; i < indexTemp; i++) {
    prefs.putUChar(("m" + String(i)).c_str(), comandosTemp[i].movimento);
    prefs.putULong(("t" + String(i)).c_str(), comandosTemp[i].duracao);
  }
  indexPermanente = indexTemp;
  for (int i = 0; i < indexTemp; i++) {
    comandosPermanente[i] = comandosTemp[i];
  }
}

void carregarRota() {
  indexPermanente = prefs.getInt("qtd", 0);
  for (int i = 0; i < indexPermanente; i++) {
    comandosPermanente[i].movimento = (Movimento)prefs.getUChar(("m" + String(i)).c_str(), 0);
    comandosPermanente[i].duracao = prefs.getULong(("t" + String(i)).c_str(), 0);
  }
}

void apagarRota() {
  prefs.clear();
  indexPermanente = 0;
}

// Funções de movimento
void frente() {
  digitalWrite(motor_direita_frente, LOW);
  digitalWrite(motor_direita_tras, HIGH);
  digitalWrite(motor_esquerda_frente, LOW);
  digitalWrite(motor_esquerda_tras, HIGH);
}

void tras() {
  digitalWrite(motor_direita_frente, HIGH);
  digitalWrite(motor_direita_tras, LOW);
  digitalWrite(motor_esquerda_frente, HIGH);
  digitalWrite(motor_esquerda_tras, LOW);
}

void esquerda() {
  digitalWrite(motor_direita_frente, LOW);
  digitalWrite(motor_direita_tras, HIGH);
  digitalWrite(motor_esquerda_frente, HIGH);
  digitalWrite(motor_esquerda_tras, LOW);
}

void direita() {
  digitalWrite(motor_direita_frente, HIGH);
  digitalWrite(motor_direita_tras, LOW);
  digitalWrite(motor_esquerda_frente, LOW);
  digitalWrite(motor_esquerda_tras, HIGH);
}

void parada() {
  digitalWrite(motor_direita_frente, LOW);
  digitalWrite(motor_direita_tras, LOW);
  digitalWrite(motor_esquerda_frente, LOW);
  digitalWrite(motor_esquerda_tras, LOW);
}
