Embarcatech    

Tarefa da aula síncrona do dia 03/02/2025  

Aluno: **Fábio Rocha Gomes Jardim**    

Matrícula: **TIC370100723**    

# Vídeo

[Link para o Drive](https://drive.google.com/file/d/1Bjwts_C0dZXV9ONO-ZNohPhk0hfaXDmf/view?usp=sharing)

# Introdução  

Esta tarefa foi desenvolvia para a placa **BitdogLab**, que utiliza o **Raspberry Pi Pico W**. O código demonstra o controle interativo da matriz de LEDs 5x5 e do display nativos da BitdogLab, utilizando tanto o **PIO** (Programmable I/O) para o controle dos LEDs quanto a interface **I2C** para a comunicação com o display. A interação com o usuário ocorre via terminal USB/serial e por meio dos botões na placa, permitindo a exibição de números e letras.
Os botões acionam e desacionam as cores verde e azul do led RGB da BitdogLab.

---

## Descrição e Funcionalidades do Projeto

- **Exibição de Números na Matriz de LEDs**  
  Utiliza uma matriz de padrões pré-definidos para números (0 a 9) e um estado "apagado". A função responsável converte os valores RGB para o formato adequado e desenha o padrão na matriz via PIO.

- **Display OLED via I2C**  
  O display OLED é inicializado e configurado via I2C. Ele exibe mensagens e caracteres informativos conforme a interação do usuário e o estado dos LEDs.

- **Controle Interativo com Botões**  
  Dois botões físicos (BOT_A e BOT_B) são configurados com interrupções. Ao serem pressionados, eles acionam a troca de estado dos LEDs (verde e azul) e atualizam o display e o terminal serial com mensagens correspondentes ("VERDE ON/OFF" ou "AZUL ON/OFF").

- **Entrada via Terminal USB/Serial**  
  O sistema aguarda a entrada de um caractere pelo terminal. Se um dígito (0-9) for digitado, o padrão correspondente é desenhado na matriz de LEDs; caso contrário, a matriz é apagada. O caractere também é exibido no display OLED.

- **Utilização do PIO**  
  A configuração do PIO possibilita o controle eficiente da matriz de LEDs, utilizando uma state machine e um programa específico carregado via PIO assembler.

---

## Funções que Compõem o Código

### `matrix_rgb(double r, double g, double b)`

- **Descrição**:  
  Converte os valores de intensidade de cores (RGB) para um formato de 32 bits na ordem **GRB**, que é o necessário para o funcionamento correto dos LEDs WS2812.

- **Funcionamento**:  
  Cada componente de cor é atenuado (multiplicado por 0.15) e, em seguida, os valores são empacotados em um `uint32_t` com a ordem de bits correta.

---

### `desenho_pio(int sprite, PIO pio, uint sm)`

- **Descrição**:  
  Desenha um padrão (sprite) na matriz de LEDs utilizando o PIO.

- **Funcionamento**:  
  - Percorre as linhas e colunas do sprite definido na matriz `matriz_numeros`.
  - Inverte a ordem de desenho em linhas pares para corrigir a visualização na BitdogLab.
  - Converte os valores RGB para o formato adequado usando `matrix_rgb` e envia os dados para a state machine do PIO.

---

### `call_back_dos_botoes(uint gpio, uint32_t events)`

- **Descrição**:  
  Função de callback para interrupções dos botões.

- **Funcionamento**:  
  - Implementa um mecanismo de debounce (250 ms) para evitar múltiplas leituras causadas pelo "bounce" dos botões.
  - Verifica qual botão foi pressionado (BOT_A ou BOT_B).
  - Inverte o estado do LED correspondente (verde para BOT_A, azul para BOT_B).
  - Atualiza o display OLED exibindo uma mensagem indicando se o LED foi ligado ou desligado.
  - Realiza também um *printf* para log via terminal.

---

### `inicializa_GPIOs(void)`

- **Descrição**:  
  Inicializa os pinos GPIO utilizados para os LEDs e botões.

- **Funcionamento**:  
  - Configura os pinos dos LEDs como saída e os dos botões como entrada.
  - Ativa resistores de pull-up para os botões, garantindo leituras estáveis.
  - Garante que os LEDs comecem desligados ao iniciar o sistema.

---

### `inicializa_PIO(void)`

- **Descrição**:  
  Configura o PIO para o controle da matriz de LEDs.

- **Funcionamento**:  
  - Seleciona o PIO (neste caso, `pio0`) e ajusta o clock do sistema para 128 MHz.
  - Carrega o programa PIO (definido em `pio_matrix.pio.h`) e inicializa uma state machine para gerenciar a comunicação com os LEDs.
  - Exibe mensagens via *printf* para indicar o sucesso da configuração.

---

### `main(void)`

- **Descrição**:  
  Função principal que integra todas as funcionalidades do projeto.

- **Fluxo de Execução**:
  1. **Inicializações**:
     - Inicializa a comunicação padrão (USB, UART, etc) com `stdio_init_all()`.
     - Chama `inicializa_GPIOs()` para configurar os pinos.
     - Chama `inicializa_PIO()` para configurar o controle da matriz de LEDs.
  2. **Configuração Inicial da Matriz**:
     - Desenha o sprite "apagado" (índice 10 na matriz de números) para garantir que a matriz de LEDs inicie sem exibir nenhum número.
  3. **Configuração de Interrupções**:
     - Define callbacks para os botões, permitindo a atualização do estado dos LEDs e do display conforme a interação.
  4. **Inicialização do Display OLED**:
     - Aguarda a conexão USB.
     - Configura a comunicação I2C e inicializa o display OLED (usando o driver `ssd1306` e fontes definidas).
     - Limpa o display para iniciar com todos os pixels apagados.
  5. **Loop Principal**:
     - Solicita que o usuário digite um caractere via terminal.
     - Aguarda a entrada do caractere utilizando `getchar_timeout_us()`.
     - Se o caractere for um dígito (0-9), chama `desenho_pio()` para desenhar o padrão correspondente na matriz de LEDs; caso contrário, apaga o display da matriz.
     - Atualiza o display OLED exibindo o caractere digitado.
  6. **Encerramento**:
     - O loop é infinito, garantindo a operação contínua do sistema.

---

## Instruções de Compilação e Execução

1. **Pré-requisitos**:
   - Ambiente de desenvolvimento configurado para o **Raspberry Pi Pico** (SDK do Pico, CMake, etc).
   - Drivers e bibliotecas para o display OLED (`ssd1306`) e para o controle via PIO.

2. **Compilação**:
   - Configure o projeto de acordo com o SDK do Pico.
   - Compile utilizando `cmake` e `make` (ou o método equivalente do seu ambiente de desenvolvimento).

3. **Carregamento na Placa**:
   - Após a compilação, faça o upload do binário gerado para a placa BitdogLab (por exemplo, via USB).

4. **Utilização**:
   - Conecte a placa via USB e abra um terminal serial.
   - Siga as mensagens exibidas no terminal para interagir com o sistema, digitando caracteres e pressionando os botões.

