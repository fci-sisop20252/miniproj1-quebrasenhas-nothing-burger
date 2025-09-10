#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <hash_md5> <tamanho> <charset> <num_workers>\n", argv[0]);
        return 1;
    }

    
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio
    if (password_len < 1 || password_len > 10) {
    fprintf(stderr, "Erro: tamanho da senha deve estar entre 1 e 10.\n");
    return 1;
    }
    if (num_workers < 1 || num_workers > MAX_WORKERS) {
        fprintf(stderr, "Erro: número de workers deve estar entre 1 e %d.\n", MAX_WORKERS);
        return 1;
    }
    if (charset_len == 0) {
        fprintf(stderr, "Erro: charset não pode ser vazio.\n");
        return 1;
    }

    
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    
    // IMPLEMENTE AQUI:
    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
        // TODO: Calcular intervalo de senhas para este worker
        long long start_index = i * passwords_per_worker + (i < remaining ? i : remaining);
        long long count = passwords_per_worker + (i < remaining ? 1 : 0);
        long long end_index = start_index + count - 1;
        // TODO: Converter indices para senhas de inicio e fim
        char start_password[password_len + 1];
        char end_password[password_len + 1];
        index_to_password(start_index, charset, charset_len, password_len, start_password);
        index_to_password(end_index, charset, charset_len, password_len, end_password);
        // TODO 4: Usar fork() para criar processo filho
        pid_t pid = fork();
        // TODO 7: Tratar erros de fork()
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid > 0) {
            // TODO 5: No processo pai: armazenar PID
            workers[i] = pid;
            printf("[Coordinator] Worker %d criado com PID %d, intervalo: %s - %s\n", i, pid, start_password, end_password);
        } else {
            // TODO 6: No processo filho: usar execl() para executar worker
            char worker_id_str[10];
            char password_len_str[10];
            sprintf(worker_id_str, "%d", i);
            sprintf(password_len_str, "%d", password_len);
            execl("./worker", "worker", target_hash, start_password, end_password, charset, password_len_str, worker_id_str, (char *)NULL);
            // TODO 7: Tratar erros de execl()
            perror("execl");
            exit(1);
        }
    }

    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
   // TODO 8: Aguardar todos os workers terminarem usando wait()
// IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis

int workers_finished = 0;
int workers_with_success = 0;

for (int i = 0; i < num_workers; i++) {
    int status;
    pid_t finished_pid = wait(&status);
    
    if (finished_pid == -1) {
        perror("wait");
        continue;
    }
    
    // Identificar qual worker terminou
    int worker_index = -1;
    for (int j = 0; j < num_workers; j++) {
        if (workers[j] == finished_pid) {
            worker_index = j;
            break;
        }
    }
    
    workers_finished++;
    
    // Verificar se terminou normalmente
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        
        if (worker_index != -1) {
            printf("[Coordinator] Worker %d (PID %d) terminou normalmente com código %d\n", 
                   worker_index, finished_pid, exit_code);
        } else {
            printf("[Coordinator] Worker desconhecido (PID %d) terminou com código %d\n", 
                   finished_pid, exit_code);
        }
        
        // Código 0 indica sucesso (senha encontrada)
        if (exit_code == 0) {
            workers_with_success++;
            printf("[Coordinator] Worker %d encontrou a senha!\n", worker_index);
        }
    } else if (WIFSIGNALED(status)) {
        int signal_num = WTERMSIG(status);
        if (worker_index != -1) {
            printf("[Coordinator] Worker %d (PID %d) foi terminado pelo sinal %d\n", 
                   worker_index, finished_pid, signal_num);
        } else {
            printf("[Coordinator] Worker desconhecido (PID %d) foi terminado pelo sinal %d\n", 
                   finished_pid, signal_num);
        }
    } else {
        if (worker_index != -1) {
            printf("[Coordinator] Worker %d (PID %d) terminou de forma anômala\n", 
                   worker_index, finished_pid);
        } else {
            printf("[Coordinator] Worker desconhecido (PID %d) terminou de forma anômala\n", 
                   finished_pid);
        }
    }
}

printf("\n[Coordinator] Todos os %d workers terminaram.\n", workers_finished);
if (workers_with_success > 0) {
    printf("[Coordinator] %d worker(s) reportaram sucesso.\n", workers_with_success);
}
    
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    
    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado
    if (password_found) {
        FILE *result_file = fopen(RESULT_FILE, "r");
        if (result_file != NULL) {
            char line[512];
            if (fgets(line, sizeof(line), result_file) != NULL) {
                line[strcspn(line, "\n")] = '\0';
                
                char *colon = strchr(line, ':');
                if (colon != NULL) {
                    *colon = '\0';
                    int worker_id = atoi(line);
                    char *found_password = colon + 1;
                    char computed_hash[33];
                    md5_string(found_password, computed_hash);
                    
                    printf("senha achada\n");
                    printf("worker: %d\n", worker_id);
                    printf("senha: %s\n", found_password);
                    printf("hash md5: %s\n", computed_hash);
                    
                    if (strcmp(computed_hash, target_hash) == 0) {
                        printf("hash equivalente\n");
                    } else {
                        printf("hash não equivalente\n");
                    }
                } 
            }
            fclose(result_file);
        } else {
            printf("falha em abrir o arquivo.\n");
        }
    } else {
        printf("senha não encontrada.\n");
    }

    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    
    return 0;
}