# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Enzo Maranho Tucilho 10436106 Jonas Fernando Nascimento de Melo 10443398
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

O coordenador calcula quantas senhas cada worker deve verificar e distribui essas senhas de forma proporcional, com base no total de combinações possíveis que é calculado a partir do conjunto de caracteres (charset) e o tamanho da senha (password_len)

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
 long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
```

---

## 2. Implementação das System Calls

fork() é usada para criar um processo filho para cada worker, o coordenador calcula o intervalo de senhas que ele deve verificar e então chama execl(), passando os argumentos necessários como o hash alvo, as senhas de início e fim etc. o coordenador utiliza wait() para aguardar a conclusão de cada processo filho, permitindo que o coordenador gerencie eficientemente a execução paralela dos workers e aguarde a conclusão de todas as tarefas.

**Código do fork/exec:**
```c
pid_t pid = fork();
-----------------------------------------------
char worker_id_str[10];
            char password_len_str[10];
            sprintf(worker_id_str, "%d", i);
            sprintf(password_len_str, "%d", password_len);
            execl("./worker", "worker", target_hash, start_password, end_password, charset, password_len_str, worker_id_str, (char *)NULL);
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**
[Explique como você implementou uma escrita atômica e como isso evita condições de corrida]

Foi feita utilizando a função open() com as flags O_CREAT | O_EXCL fazendo com que apenas o primeiro processo que tentar criar o arquivo seja bem-sucedido. Assim apenas um worker vai conseguir escrever no arquivo, evitando que haja conflito, porque se outros workers tentarem escrever depois que o arquivo já foi criado, eles simplesmente não vão conseguir abrir o arquivo.

Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**

[Explique como o coordinator lê o arquivo de resultado e faz o parse da informação]

O coordinator primeiro checa se o arquivo está lá. Se estiver, ele abre e faz a leitura da linha com o worker_id e a senha. Depois, usa o sscanf() para separar essas informações. Com isso, ele pode parar os outros workers e mostrar o resultado.
---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0.004.s | 0.005s | 0.006s | 0.67 |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | 3.099s | 3.228s | 0.433s | 7.15 |

**O speedup foi linear? Por quê?**
[Analise se dobrar workers realmente dobrou a velocidade e explique o overhead de criar processos]

O speedup não foi linear , pois ao aumentar o número de workers deveria diminuir o tempo, mas isso não aconteceu. No primeiro teste, o tempo até aumentou com mais workers, provavelmente por conta do overhead de gerenciar os processos. No segundo teste, o ganho foi maior, mas ainda assim não foi perfeito, já que o tempo não caiu de forma proporcional.

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
[Descreva um problema e como resolveu. Ex: "Tive dificuldade com o incremento de senha, mas resolvi tratando-o como um contador em base variável"]

O maior desafio que enfrentamos foi com o incremento de senha. Incrementar uma senha de forma lexicográfica, como se fosse um contador, parecia simples de primeira vista, mas a coisa ficou mais complicada quando tivemos que tratar o "overflow".

Para resolver isso, tratamos o incremento como um contador, onde começava pelo último caractere da senha e ia ajustando de trás pra frente, verificando se o caractere atual ultrapassava o limite. Se sim, ele voltava para o primeiro caractere e passava para o caractere à esquerda.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [ ] Código compila sem erros
- [ ] Todos os TODOs foram implementados
- [ ] Testes passam no `./tests/simple_test.sh`
- [ ] Relatório preenchido
