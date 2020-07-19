/*
  Exemplo de uso RadioengeLoraMesh lib
  Autor: Richard C. W.
  Versão: 0.1
*/

#include <RadioengeLoraMesh.h> //Inclusão da biblioteca


/* 
    Aqui é apresentado a inicialização da RadioengeLib,
    o comando de leitura local, que retorna as informações do módulo
    e liga/desliga o led por 1 segundo.

    Pinos de comunicação UART da porta de comandos
    
    No exemplo a seguir:
    - ligar o pino RX (pino 2) do módulo na porta digital 6 do arduino
    - ligar o pino TX (pino 2) do módulo na porta digital 7 do arduino

    Conectar um led no pino GPIO 0 do módulo.

    
 */
#define RADIO_RX_PIN 6
#define RADIO_TX_PIN 7


/* Instância do objeto */
RadioengeLoraMesh radio(RADIO_RX_PIN, RADIO_TX_PIN, 9600);

device_t dispositivo;   //Struct com as informações do Rádio
mod_param_t modulation; //Struct com as informações de modulação

/*
    Inicialização do Arduino
*/
void setup()
{
    Serial.begin(115200);
    Serial.println("Hello World - Radioenge LoraMesh");
    radio.begin(); //Inicialização da comunicação com o Rádio

    /*
       Realiza a leitura local do Rádio e verifica 
       se a leitura local foi realizada com sucesso
    */
    if(radio.localRead(&dispositivo) != RADIO_OK)
    {
        Serial.println("Falha ao comunicar com o Radio LoraMesh");
    }
    else
    {
        /* Imprime as informações do módulo Radioenge LoraMesh */
        Serial.println("==================");
        Serial.println("   Local Read");
        Serial.println("------------------");
        Serial.print("UID: ");
        Serial.print(dispositivo.uniqueId[0]);
        Serial.print(".");
        Serial.print(dispositivo.uniqueId[1]);
        Serial.print(".");
        Serial.print(dispositivo.uniqueId[2]);
        Serial.print(".");
        Serial.println(dispositivo.uniqueId[3]);

        Serial.print("ID: ");
        Serial.println(dispositivo.id);

        Serial.print ("Versao de HW: ");
        Serial.println(dispositivo.hwVersion);

        Serial.print("Versao de FW: ");
        Serial.print(dispositivo.fwVersion);
        Serial.print(".");
        Serial.println(dispositivo.fwRevision);

        Serial.print("Banco ");
        Serial.println(dispositivo.bank);

        Serial.print("Modulacao: ");
        if(dispositivo.modulation == MODULATION_LORA)
        {
            Serial.println("Lora");
        }
        else if(dispositivo.modulation == MODULATION_FSK)
        {
            Serial.println("FSK");
        }
        else
        {
            Serial.println("Erro");
        }
    }

    /* 
        Consulta a modulação configurada e 
        imprime as informações de modulação
    */
    if(radio.getModulation(dispositivo.id, &modulation) == RADIO_OK)
    {
        Serial.println("==================");
        Serial.println("   Modulacao");
        Serial.println("------------------");
        Serial.print("Potencia: ");
        Serial.print(modulation.power);
        Serial.println(" dBm");
        Serial.print("Banda: ");
        switch(modulation.bandwidth)
        {
            case BANDWIDTH_125_kHz:
                Serial.println("125 kHz");
                break;
            case BANDWIDTH_250_kHz:
                Serial.println("250 kHz");
                break;
            case BANDWIDTH_500_kHz:
                Serial.println("500 kHz");
                break;
            default:
                Serial.println("?");
                break;
        }
        
        Serial.print("Spread Factor: ");
        if(modulation.spreadFactor == SPREAD_FACTOR_FSK)
        {
            Serial.println("FSK");
        }
        else if(modulation.spreadFactor >= SPREAD_FACTOR_7 && modulation.spreadFactor <= SPREAD_FACTOR_12)
        {
            Serial.println(modulation.spreadFactor);
            Serial.print("Coding Rate: ");
            switch(modulation.codingRate)
            {
                case CODING_RATE_4_5:
                    Serial.println("4/5");
                    break;
                case CODING_RATE_4_6:
                    Serial.println("4/6");
                    break;
                case CODING_RATE_4_7:
                    Serial.println("4/7");
                    break;
                case CODING_RATE_4_8:
                    Serial.println("4/8");
                    break;				
                default:
                    Serial.println("?");
                    break;
            }
        }
        else
        {
            Serial.println("?");
        }
    }
    else
    {
        Serial.println("Falha ao obter os parametros de modulacao");
    }

    Serial.println("==========================================");
    Serial.println("   Definindo pino GPIO 0 como saida...");
    Serial.println("------------------------------------------");

    /* Define o pino GPIO zero como pino de saída */
    if(radio.pinMode(dispositivo.id, RADIO_GPIO0, RADIO_OUTPUT, RADIO_PULL_OFF) == RADIO_OK)
    {
        Serial.println("GPIO 0 definido como pino de saída");
    }
    else
    {
        Serial.println("Falha ao configurar o pino");
    }
}

/*
    Alterna o valor de saída do pino GPIO 0 a cada 1 segundo
*/
void loop() 
{

    radio.digitalWrite(dispositivo.id, RADIO_GPIO0, HIGH);
    delay(1000);
    radio.digitalWrite(dispositivo.id, RADIO_GPIO0, LOW);
    delay(1000);
}
