// Inclusão de bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"       // SDK do Raspberry Pi Pico
#include "hardware/i2c.h"      // Hardware I2C
#include "inc/ssd1306.h"       // Driver do display
#include "inc/font.h"          // Fontes para o display
#include "hardware/pio.h"      // Programmable I/O (PIO)
#include "hardware/clocks.h"   // Controle de clock

// Arquivo gerado pelo PIO assembler
#include "pio_matrix.pio.h"

// Definições de hardware
#define I2C_PORT i2c1          // Porta I2C utilizada
#define I2C_SDA 14             // Pino SDA
#define I2C_SCL 15             // Pino SCL
#define endereco 0x3C          // Endereço I2C do display
#define LED_R 13               // LED Vermelho
#define LED_G 11               // LED Verde
#define LED_B 12               // LED Azul
#define BOT_A 5                // Botão A
#define BOT_B 6                // Botão B
#define LED_COUNT 25           // Número de LEDs na matriz
#define LED_PIN 7              // Pino da matriz de LEDs

// Variáveis globais
static volatile uint32_t tempo_anterior = 0; // Timer para debounce
static PIO pio;                // Instância do PIO
static uint sm;                // State Machine do PIO
ssd1306_t ssd;                 // Estrutura do display
bool cor = true;               // Cor de fundo do display


// Matriz multidimensional com padrões de LEDs para cada número (0-9 + apagado)
// Formato: [número][linha][coluna][canal RGB]
int matriz_numeros[11][5][5][3] = {
    {//0
    {{0, 0, 0}, {42, 0, 0}, {42, 0, 0}, {42, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 0, 0}, {0, 0, 0}, {42, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {42, 0, 0}, {0, 0, 0}, {42, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {42, 0, 0}, {0, 0, 0}, {42, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {42, 0, 0}, {42, 0, 0}, {42, 0, 0}, {0, 0, 0}} 
},

{//1
    {{0, 0, 0}, {0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}}, 
    {{0, 0, 0}, {0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}}  
},

{//2
    {{0, 0, 0}, {0, 2, 42}, {0, 2, 42}, {0, 2, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 2, 42}, {0, 0, 0}},  
    {{0, 0, 0}, {0, 2, 42}, {0, 2, 42}, {0, 2, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 2, 42}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},  
    {{0, 0, 0}, {0, 2, 42}, {0, 2, 42}, {0, 2, 42}, {0, 0, 0}}
},

{//3
    {{0, 0, 0}, {33, 0, 42}, {33, 0, 42}, {33, 0, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {33, 0, 42}, {0, 0, 0}},
    {{0, 0, 0}, {33, 0, 42}, {33, 0, 42}, {33, 0, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {33, 0, 42}, {0, 0, 0}},
    {{0, 0, 0}, {33, 0, 42}, {33, 0, 42}, {33, 0, 42}, {0, 0, 0}}
},

{//4
    {{0, 0, 0}, {0, 19, 42}, {0, 0, 0}, {0, 19, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 19, 42}, {0, 0, 0}, {0, 19, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 19, 42}, {0, 19, 42}, {0, 19, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 19, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 19, 42}, {0, 0, 0}}
},

{//5
    {{0, 0, 0}, {42, 40, 0}, {42, 40, 0}, {42, 40, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 40, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 40, 0}, {42, 40, 0}, {42, 40, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {42, 40, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 40, 0}, {42, 40, 0}, {42, 40, 0}, {0, 0, 0}}
},

{//6
    {{0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {11, 42, 0}, {11, 42, 0}, {11, 42, 0}, {0, 0, 0}},
    {{0, 0, 0}, {11, 42, 0}, {0, 0, 0}, {11, 42, 0}, {0, 0, 0}},
    {{0, 0, 0}, {11, 42, 0}, {11, 42, 0}, {11, 42, 0}, {0, 0, 0}}
},

{//7
    {{0, 0, 0}, {42, 0, 40}, {42, 0, 40}, {42, 0, 40}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {42, 0, 40}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {42, 0, 40}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {42, 0, 40}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {42, 0, 40}, {0, 0, 0}}
},

{//8
    {{0, 0, 0}, {42, 14, 0}, {42, 14, 0}, {42, 14, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 14, 0}, {0, 0, 0}, {42, 14, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 14, 0}, {42, 14, 0}, {42, 14, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 14, 0}, {0, 0, 0}, {42, 14, 0}, {0, 0, 0}},
    {{0, 0, 0}, {42, 14, 0}, {42, 14, 0}, {42, 14, 0}, {0, 0, 0}}
},

{//9
    {{0, 0, 0}, {0, 32, 42}, {0, 32, 42}, {0, 32, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 32, 42}, {0, 0, 0}, {0, 32, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 32, 42}, {0, 32, 42}, {0, 32, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 32, 42}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 32, 42}, {0, 0, 0}}
},

{//10 - Apaga o display
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
}
};

// Função para definir a intensidade de cores do LED  
// A função recebe os canais na ordem rgb, mas retorna na ordem GRB, necessárias para o correto funcionamentos dos LEDs ws2812.
static uint32_t matrix_rgb(double r, double g, double b){
    // Multiplique pelos fatores desejados para atenuar o brilho.
      // Os valores resultantes são convertidos para unsigned char.
      unsigned char R = (unsigned char)(r * 0.15);
      unsigned char G = (unsigned char)(g * 0.15);
      unsigned char B = (unsigned char)(b * 0.15);
      return ((uint32_t)G << 24) | ((uint32_t)R << 16) | ((uint32_t)B << 8);
  }

  // Rotina que desenha um número na matriz de LEDs utilizando PIO
void desenho_pio(int sprite, PIO pio, uint sm){   
    uint32_t valor_led;
    // Percorre cada linha e coluna de cada número
    for (int linha = 4; linha >= 0; linha--) // Percorre as linhas de baixo para cima. A lógica foi invertida somente para inverter a visualização na bitdoglab
    {
        for (int coluna = 0; coluna < 5; coluna++)
        {
            if(linha%2!=0){ // Se a linha for par, desenha da direita para a esquerda. A lógica foi invertida somente para inverter a visualização na bitdoglab
                valor_led = matrix_rgb(
                matriz_numeros[sprite][linha][coluna][0],
                matriz_numeros[sprite][linha][coluna][1],
                matriz_numeros[sprite][linha][coluna][2]);
                pio_sm_put_blocking(pio, sm, valor_led);
            }
            else{ // Se a linha for ímpar, desenha da direita para a esquerda
                valor_led = matrix_rgb(
                matriz_numeros[sprite][linha][4-coluna][0],
                matriz_numeros[sprite][linha][4-coluna][1],
                matriz_numeros[sprite][linha][4-coluna][2]);
                pio_sm_put_blocking(pio, sm, valor_led);
            }
        }
    }
}

void call_back_dos_botoes(uint gpio, uint32_t events){
    uint32_t tempo_agora = to_ms_since_boot(get_absolute_time()); // Pega o tempo atual em milissegundos para debounce
    if (tempo_agora - tempo_anterior > 250){ // Verifica se o botão foi pressionado por mais de 250ms para evitar bounce
        tempo_anterior = tempo_agora; // Atualiza o tempo anterior
        if (gpio == BOT_A){
            gpio_put(LED_G, !gpio_get(LED_G)); // Inverte o estado do LED verde
            if(gpio_get(LED_G)==1){
                printf("O LED verde foi LIGADO\n");
                ssd1306_fill(&ssd, !cor); // Limpa o display
                ssd1306_draw_string(&ssd, "VERDE ON", 30, 30); // Desenha uma string
                ssd1306_send_data(&ssd); // Atualiza o display
            }else{
                printf("O LED verde foi DESLIGADO\n");
                ssd1306_fill(&ssd, !cor); // Limpa o display
                ssd1306_draw_string(&ssd, "VERDE OFF", 30, 30); // Desenha uma string
                ssd1306_send_data(&ssd); // Atualiza o display

            }
        }else{
            gpio_put(LED_B, !gpio_get(LED_B)); // Inverte o estado do LED vermelho
            if(gpio_get(LED_B)==1){
                printf("O LED azul foi LIGADO\n");
                ssd1306_fill(&ssd, !cor); // Limpa o display
                ssd1306_draw_string(&ssd, "AZUL ON", 30, 30); // Desenha uma string
                ssd1306_send_data(&ssd); // Atualiza o display
            }else{
                printf("O LED azul foi DESLIGADO\n");
                ssd1306_fill(&ssd, !cor); // Limpa o display
                ssd1306_draw_string(&ssd, "AZUL OFF", 30, 30); // Desenha uma string
                ssd1306_send_data(&ssd); // Atualiza o display

            }
        }
    }
}

void inicializa_GPIOs(void) {
    // Inicializa os GPIOs para os LEDs e botões
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_init(BOT_A);
    gpio_init(BOT_B);
    
    // Seta os GPIO's como saída para os LEDs
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
    
    // Seta os GPIO's como entrada para os botões
    gpio_set_dir(BOT_A, GPIO_IN);
    gpio_set_dir(BOT_B, GPIO_IN);
    
    // Seta os GPIO's com pull-up para os botões
    gpio_pull_up(BOT_A);
    gpio_pull_up(BOT_B);
    
    // Coloca os LEDs como desligados
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);
}

void inicializa_PIO(void){
    // Configura variaveis para o PIO
        pio = pio0; 
        bool ok;
        
        // Configura o clock para 128 MHz
        ok = set_sys_clock_khz(128000, false);
    
        printf("Iniciando a transmissão PIO\n");
        if (ok) printf("Clock set to %ld\n", clock_get_hz(clk_sys));
    
        // Configuração do PIO
        uint offset = pio_add_program(pio, &pio_matrix_program);
        sm = pio_claim_unused_sm(pio, true);
        pio_matrix_program_init(pio, sm, offset, LED_PIN);
    
    }

int main(void) {
    // Inicializa todas as stdio (USB, UART, etc)
    stdio_init_all();
    inicializa_GPIOs();
    inicializa_PIO();

    // Inicia a matriz de LEDs apagada (10 corresponde ao display apagado)
    desenho_pio(10, pio, sm);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BOT_A, GPIO_IRQ_EDGE_FALL, true, &call_back_dos_botoes);
    gpio_set_irq_enabled_with_callback(BOT_B, GPIO_IRQ_EDGE_FALL, true, &call_back_dos_botoes);

    // (Opcional) Espera até que a conexão USB esteja ativa.
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
  
  char c = 0;

    while (true) {
        // Solicita que o usuário digite um caractere
        printf("Digite um caractere e pressione Enter: \n");
        fflush(stdout); // Garante que a mensagem seja exibida imediatamente

        // Loop secundário aguardando entrada
        char c = 0;
        while (true) {
            int input = getchar_timeout_us(100000); // Aguarda até que um caractere seja recebido
            if (input != PICO_ERROR_TIMEOUT) {
                c = (char)input;
                break;
            }
            tight_loop_contents(); // Mantém o processador ativo sem desperdiçar energia
        }

        if(c == '0'||c == '1'||c == '2'||c == '3'||c == '4'||c == '5'||c == '6'||c == '7'||c == '8'||c == '9'){
            desenho_pio(c -'0', pio, sm);
        }else{
            desenho_pio(10, pio, sm);
        }

        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_draw_char(&ssd, c, 60, 30); // Desenha um caractere
        ssd1306_send_data(&ssd); // Atualiza o display

    }

    return 0;
}
