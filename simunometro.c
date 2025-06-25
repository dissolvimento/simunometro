#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // p/ sleep()
#include <time.h>       // p/ time()
#include <sys/ioctl.h>  // p/ pegar o tamanho do terminal e centralizar

#define RED "\033[1;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m" // reseta as cores do terminal

typedef struct{
    int hour;
    int min;
} current_time;

int main() {
    time_t start_time = time(NULL); // inicia contagem de tempo desde o início do programa
    int h, m;
    current_time table[2][6]; // cria tabela 2x6 p/ hora e minuto
    int row, column, i;
    char buffer[6]; // p/ armazenar hora (ex '13:30\0')
    int counter = 0; // controla qual item já foi "marcado"

    // executa o comando (no shell) "date +%H:%M" e abre um pipe p/ leitura
    FILE *fp = popen("date +%H:%M", "r"); 
    /*
        FILE -> ponteiro usado para entrada/saída de dados
        popen -> executa comando do sistema e abre um pipe (canal de comunicação)
        "r" -> read mode: ler saída do comando
    */
    if (fp == NULL) {
        perror("Erro ao executar comando de data"); // perror -> pega a mensagem de erro padrao do sistema
        return 1;
    }

    // passa fp para buffer em string
    fgets(buffer, sizeof(buffer), fp); // fgets(local p/ armazenar valor + \n\0, tamanho máximo, valor);
    pclose(fp);

    // passa buffer para endereço de h e m
    sscanf(buffer, "%d:%d", &h, &m);
    
    int total_m = m; // minutos totais
        
    // cria a tabela (armazena valores de hora e minutos em table.hour e table.minutes, respectivamente)
    for(int i = 0; i < 12; i++) {
        row = i / 6; // 1 / 6 = 0, 2 / 6 = 0, ..., 6 / 6 = 1, 7 / 6 = 1, ...
        column = i % 6; // 1 % 6 = 1, 2 % 6 = 2, ..., 6 / 6 = 0, 7 % 6 = 1, ...
        total_m = m+(i*30); // minutos atuais + 30min * i
        table[row][column].hour = h + (total_m / 60); // armazena em table.hour = h + total_m/60 horas
        table[row][column].min = (total_m % 60); // armazena em table.min = total_m%60 (minutos restantes)
    }

    // loop infinito: imprime e atualiza tabela a cada 30min
    while(1) {
        system("clear"); // limpa a tela a cada ciclo
        
        // pegar o tamanho do terminal pra posteriormente centralizar o simunometro
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        /*
            STDOUT_FILENO -> saída do terminal
            TIOCGWINSZ -> comando da ioctl que pega o tamanho da janela do terminal

            o &w vai ser preenchido com algo como:
                struct winsize {
                    unsigned short ws_row;    // número de linhas
                    unsigned short ws_col;    // número de colunas
                    ...
                };
        */
        
        // cada horário ocupa 6 colunas (ex: 14:00\0), total 6 · 6 = 36 colunas
        int table_width = 6 * 6;
        int horizontal_padding = (w.ws_col - table_width) / 2; // quantidade de colunas do tamanho do terminal - quantidade de caracteres / 2 (meio)
        int vertical_padding = (w.ws_row - 2) / 2; // pega o tamanho vertical do terminal, diminui por 2 (quantidade de linhas) e divide por dois (metade do padding pra cima e metade pra baixo), deixando centralizado

        // centralização vertical
        for (int s = 0; s < vertical_padding; s++) {
            printf("\n");
        }

        // centralização horizontal + impressão da table
        for(row = 0, i = 0; row < 2; row++) {
            for (int s = 0; s < horizontal_padding; s++) {
                printf(" ");
            }

            for(column = 0; column < 6; column++, i++){
                if(i <= counter) { // verifica se o i é menor que o verificador de 30min (counter)
                    // "marca" de vermelho quando o tempo tiver passado
                    printf(RED "%02d:%02d  " RESET, 
                           table[row][column].hour, table[row][column].min);
                } else {
                    // imprime o resto da tabela
                    printf(GREEN "%02d:%02d  ", 
                           table[row][column].hour, table[row][column].min);
                }
            }
            printf("\n");
        }

        sleep(1); // pede para o programa esperar 1 segundo antes de continuar (evita cpu cheia)
        
        time_t now = time(NULL); // pega o horário atual do sistema
        double elapsed =  difftime(now, start_time); // armazena em elapsed a diferenca entre now - start_time
        
        if (elapsed >= 1800) { // se a tiver passado 30min (1800 segundos)
            counter++;
            if (counter >= 12) counter = 12; // limita ao tamanho da tabela
            start_time = now; // reinicia o contador
        }
    }

    return 0;
}
