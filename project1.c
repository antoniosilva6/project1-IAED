/* iaed-23 - ist1102879 - project1
 * 
 * Ficheiro: project1.c
 * Autor: António Dias da Silva
 * Descição: Sistema de gestão de transportes públicos
*/

/* Bibliotecas */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Variáveis úteis */

#define BUFFER_SIZE 128         /* Tamanho do "buffer" */
#define ARR_SIZE 32             /* Tamanho de vetores */

#define MAX_CARREIRAS 200       /* Número máximo de carreiras */
#define MAX_PARAGENS 10000      /* Número máximo de paragens */  
#define MAX_LIGACOES 30000      /* Número máximo de ligações */

#define MAX_NOME_CARREIRA 21    /* Dimensão do nome da carreira */             
#define MAX_NOME_PARAGEM 51     /* Dimensão do nome da paragem */ 

#define DENTRO 1                /* Estado: dentro */
#define FORA 0                  /* Estado: fora */

#define NAO_EXISTE -1           /* Código de erro */

/* Tipos de Dados */

/* Carreira */
typedef struct {
    char nome[MAX_NOME_CARREIRA];    
    int indexLigacoes[MAX_LIGACOES];    /* Vetor de indices das paragens */
    int numLigacoes;                 
    int numParagens;
    float custo;
    float duracao;
} Carreira;

/* Paragem */
typedef struct {
    char nome[MAX_NOME_PARAGEM];
    int indexCarreiras[MAX_CARREIRAS];  /* Vetor de indices das carreiras */
    int numCarreiras;
    double latitude;
    double longitude;
} Paragem;

/* Variaveis Globais */

int _numParagens = 0;               /* Número total de Paragens */
Paragem _paragens[MAX_PARAGENS];    /* Paragens Guardadas */

int _numCarreiras = 0;              /* Número total de Carreiras*/
Carreira _carreiras[MAX_CARREIRAS]; /* Carreiras Guardadas */

/* Funcoes de Leitura */

/* Lê o input inteiro e guarda num vetor */
void lePalavraAteFimDeLinha(char str[]) {
	char c = getchar();
	int i = 0;
    while (c == ' ' || c == '\t')
	    c = getchar();
    while (c != '\n') {
	    str[i++] = c;
	    c = getchar();
	}

	str[i] = '\0';
}

/* Guarda num vetor a primeira palavra (argumento) lido e atualiza o buffer, 
retirando-o, mas mantendo o resto */
void leProximaPalavra(char str[], char buffer[]) {
    int i, k, j;
    /* Encontra o índice i do primeiro caractere não branco no buffer */
    for (i = 0; buffer[i] != '\0'; i++) {
        if (!isspace(buffer[i])) {
            break;
        }
    }

    /* Se o buffer está vazio ou contém apenas espaços em branco, define a 
    string de saída como vazia e retorna */
    if (buffer[i] == '\0') {
        str[0] = '\0';
        return;
    }

    /* Encontra o índice j do próximo caractere em branco no buffer após i */
    for (j = i; buffer[j] != '\0'; j++) {
        if (isspace(buffer[j])) {
            break;
        }
    }

    /* Copia a palavra do buffer para a string de saída */
    k = 0;
    for (; i < j; i++) {
        str[k++] = buffer[i];
    }
    str[k] = '\0';

    /* Encontra o próximo índice j do caractere em branco no buffer 
    após a palavra copiada */
    for (; buffer[j] != '\0'; j++) {
        if (!isspace(buffer[j])) {
            break;
        }
    }

    /* Move o restante do buffer para o início, excluindo a palavra copiada */
    k = 0;
    for (; buffer[j] != '\0'; j++) {
        buffer[k++] = buffer[j];
    }
    buffer[k] = '\0';
}

/* Guarda num vetor a palavra entre aspas e atualiza o buffer, retirando-a */
void tirarPalavraEntreAspas(char *buffer, char *nome) {
    /* Encontra o primeiro e último índice das aspas duplas no buffer */
    char *inicio = strchr(buffer, '"');
    char *fim = strchr(inicio + 1, '"');
    char *next_char = fim + 1;  /* próximo caractere após o último índice das aspas */
    
    if (inicio && fim) {
        int len = fim - inicio - 1;
        strncpy(nome, inicio + 1, len); /* Copia a string entre as aspas para a variável nome */
        nome[len] = '\0';
        
        /* Encontra o próximo caractere não branco após a última aspa */
        while (isspace(*next_char)) {
            ++next_char;
        }
        memmove(inicio, next_char, strlen(next_char) + 1);
    }
}

/* Funcoes auxiliares */

/* Retorna o numero de palavras num vetor */
int contaPalavras(char buffer[]) {
    int i = 0, contador = 0, estado = FORA;
    while (buffer[i] != '\0') {
        if (buffer[i] == '\n' || buffer[i] == '\t' || buffer[i] == ' ')
            estado = FORA;
        else if (estado == FORA) {
            estado = DENTRO;
            ++contador;
        }
        i++;
    }
    return contador;
}

/* Retira apenas a primeira letra e o espaço */
void removerComandoDoBuffer(char buffer[]) {
    int i;

    for (i = 0; buffer[i] != ' ' && buffer[i] != '\0'; i++);
    
    if (buffer[i] == ' ') {
        memmove(buffer, buffer + i + 1, strlen(buffer) - i);
    }
}

/* Verifica se existem aspas */
int temAspas(char buffer[]){
    if (buffer[0] == '"')
        return 1;
    return 0;
}

/* Chega todos os indices uma posição para a direira e insere na primeira
posição a nova paragem de origem */
void mudarParagemOrigem(int paragens[], int numParagens, int novaOrigem) {
    int i;
    for (i = numParagens-1; i >= 0; i--) {
        paragens[i+1] = paragens[i];    /* move os indíces todas para a direitao */
    }
    paragens[0] = novaOrigem;
}

/* Algoritmo de ordenação BubbleSort */

void bubbleSort(int indexes[], int size, int (*cmpFunc)(int a, int b)) {
	int i, j, done;

	for (i = 0; i < size - 1; i++) {
		done = 1;
		for (j = size - 1; j > i; j--)
			if ((*cmpFunc) (indexes[j - 1], indexes[j])) {
				int aux = indexes[j];
				indexes[j] = indexes[j - 1];
				indexes[j - 1] = aux;
				done = 0;
			}
		if (done)
			break;
	}
}

/* Funções de Paragem */

/* Retorna o indice da Paragem, caso esta exista */
int encontraParagem(char nome[]) {
    int i;
    for (i = 0; i < _numParagens; i++) {
        if (!strcmp(nome, _paragens[i].nome)) {
            return i;
        }       
    }
    return NAO_EXISTE;
}

/* Imprime a informação relativa à paragem do argumento */
void infoParagem(char buffer[]) {
    int indexParagem;

    removerComandoDoBuffer (buffer);
    indexParagem = encontraParagem(buffer);
    if (indexParagem == NAO_EXISTE){
        printf ("%s: no such stop.\n", buffer);
    }

    else {
        printf ("%16.12f %16.12f\n", _paragens[indexParagem].latitude,
                _paragens[indexParagem].longitude);
    }
}

/* Imprime a paragens consoante o indice dado */
void mostraParagens(int index) {
    printf ("%s: %16.12f %16.12f %d\n",
    _paragens[index].nome, _paragens[index].latitude,
    _paragens[index].longitude, _paragens[index].numCarreiras);
}

/* Lista as paragens existentes */
void listaParagens() {
    int i;
    for (i = 0; i < _numParagens; i++){
        mostraParagens(i);
    }
}

/* Recebe um nome, latitude e longitude e cria a Paragem */
void criaParagem (char nome[], double latitude, double longitude) {
    strcpy(_paragens[_numParagens].nome, nome);
    /* Inicialização das variáveis */
    _paragens[_numParagens].latitude = latitude;
    _paragens[_numParagens].longitude = longitude;
    _paragens[_numParagens].numCarreiras = 0;

    _numParagens++; /* Incrementa número de Paragens */
}

/* Cria a Paragem com nome entre aspas */
void paragemComAspas(char buffer[], Paragem novaParagem) {
    char latitude[ARR_SIZE];

    tirarPalavraEntreAspas(buffer, novaParagem.nome);
    leProximaPalavra(latitude, buffer);
    novaParagem.latitude = atof(latitude);
    novaParagem.longitude = atof(buffer);

    if (encontraParagem(novaParagem.nome) != NAO_EXISTE){
        printf ("%s: stop already exists.\n", novaParagem.nome);
    }

    else {
        criaParagem (novaParagem.nome, novaParagem.latitude, 
                    novaParagem.longitude);
    }
}

/* Cria uma nova Paragem e adiciona no vetor global */
void adicionaParagem(char buffer[]) {
    Paragem novaParagem;
    char latitude[ARR_SIZE];

    removerComandoDoBuffer(buffer);

    if (temAspas(buffer) == 1){
        paragemComAspas(buffer, novaParagem);
    }
    
    else {
        leProximaPalavra(novaParagem.nome, buffer);
        
        leProximaPalavra(latitude, buffer);
        novaParagem.latitude = atof(latitude);
        novaParagem.longitude = atof(buffer);

        if (encontraParagem(novaParagem.nome) != NAO_EXISTE){
            printf ("%s: stop already exists.\n", novaParagem.nome);
        }

        else {
            criaParagem (novaParagem.nome, novaParagem.latitude, 
                        novaParagem.longitude);
        }     
    }
}

/* Consoante o número de argumentos, executa os comandos */
void comandoParagens(char buffer[]) {

    int n = contaPalavras(buffer);

    if (n == 1){
        listaParagens();
    }

    else if (n == 2){
        infoParagem(buffer);
    }

    else if (n >= 4){
        adicionaParagem(buffer);
    }

    else {
        printf ("Invalid comand: %s\n", buffer);
    }
}

/* Funções de Carreiras */

/* Retorna o indice da Carreira, caso esta exista */
int encontraCarreira(char nome[]) {
    int i;
    for (i = 0; i < _numCarreiras; i++) {
        if(!strcmp(nome, _carreiras[i].nome)) {
            return i;
        }
    }
    return NAO_EXISTE;
}

/* Imprime as informações relativas à carreira, dada como argumento */
void mostraCarreiras(int index) {
    int numParagens;

    numParagens = _carreiras[index].numLigacoes;

    if (numParagens == 0) {
        printf ("%s %d %.2f %.2f\n",
        _carreiras[index].nome, _carreiras[index].numParagens,
        _carreiras[index].custo, _carreiras[index]. duracao);
    }

    else {
        printf ("%s %s %s %d %.2f %.2f\n",
        _carreiras[index].nome, _paragens[_carreiras[index].indexLigacoes[0]].nome,
        _paragens[_carreiras[index].indexLigacoes[numParagens - 1]].nome, 
        _carreiras[index].numParagens, _carreiras[index].custo, 
        _carreiras[index]. duracao);
    }
}

/* Lista as Carreiras existentes */
void listaCarreiras() {
    int i;
    for (i = 0; i < _numCarreiras; i++) {
        mostraCarreiras(i);
    }
}

/* Imprime todas as Paragens pela ordem onde passa a carreira */
void infoCarreira(int index) {
    int i, numParagens;
    numParagens = _carreiras[index].numLigacoes;

    for (i = 0; i < numParagens; i++) {
        if (i == numParagens - 1) {
            printf ("%s\n", _paragens[_carreiras[index].indexLigacoes[i]].nome);
        }

        else {
            printf ("%s, ", _paragens[_carreiras[index].indexLigacoes[i]].nome);
        }
    }
}

/* Cria uma nova Carreira e adiciona no vetor global de Carreiras*/
void adicionaCarreira(char buffer[]) {
    Carreira novaCarreira;
    
    strcpy(novaCarreira.nome, buffer);

    strcpy(_carreiras[_numCarreiras].nome, novaCarreira.nome);
    /* Inicialização das variáveis */
    _carreiras[_numCarreiras].numLigacoes = 0;
    _carreiras[_numCarreiras].duracao = 0;
    _carreiras[_numCarreiras].custo = 0;
    _carreiras[_numCarreiras]. numParagens = 0;

    _numCarreiras++; /* Incrementa o número de carreiras */
}

/* Verifica se a Carreira existe, se não existir cria uma nova, caso
contrário imprime as suas paragens */
void verificaCarreira(char buffer[]) {

    int index;
    removerComandoDoBuffer(buffer);
    index = encontraCarreira(buffer);

    if (index == NAO_EXISTE) {
        adicionaCarreira(buffer);
    }

    else {
        infoCarreira(index);
    }
}

/* Verifica um potencial erro na abreviatura da palavra inverso */
int verificaErroInverso (char buffer[]) {
    int tamanho, condicao;
    char inverso[] = "inverso";

    tamanho = strlen(buffer);
    if (tamanho < 3 || tamanho > 7) {
        return 1;
    }

    /* Verifica se os caracteres até ao tamanho, são iguais */
    condicao = strncmp(inverso, buffer, tamanho);
    if (condicao != 0) {
        return 1;
    }
    
    return 0;
}

/* Lista as Paragens da Carreira por ordem inversa */
void listaParagensInverso(int index) {
    int i, numParagens;
    numParagens = _carreiras[index].numLigacoes;

    for (i = numParagens - 1; i >= 0; i--) {
        if (i == 0) {
            printf ("%s\n", _paragens[_carreiras[index].indexLigacoes[i]].nome);
        }

        else {
            printf ("%s, ", _paragens[_carreiras[index].indexLigacoes[i]].nome);
        }
    }
}

/* Imprime as Paragens da Carreira por ordem inversa */
void inversoInfoCarreira(char buffer[]) {
    int index, erro;
    char nome[MAX_NOME_CARREIRA];

    removerComandoDoBuffer(buffer);

    leProximaPalavra(nome, buffer);
    index = encontraCarreira(nome);
    
    if (index == NAO_EXISTE) {
        return;
    }

    else {
        erro = verificaErroInverso(buffer);
        if (erro) {
            printf ("incorrect sort option.\n");
        }

        else {
            listaParagensInverso(index);
        }
    } 
}

/* Consoante o número de argumentos, executa os comandos */
void comandoCarreiras(char buffer[]) {

    int n = contaPalavras(buffer);

    if (n == 1) {
        listaCarreiras();
    }

    else if (n == 2) {
        verificaCarreira(buffer);
    }

    else if (n == 3) {
        inversoInfoCarreira(buffer);
    }

    else {
        printf ("Invalid comand: %s\n", buffer);
    }
}

/* Funcoes de Ligacoes */ 

/* Verifica se a ligacao é circular ou não */
int ligacaoCircular(int indexCarreira, int numLigacoes) {
    if (_carreiras[indexCarreira].indexLigacoes[numLigacoes-1] ==
        _carreiras[indexCarreira].indexLigacoes[0]) {
            return 1;
        }
    return 0;
}

/* Retorna se o indice da Carreira já está no vetor de indices de Carreiras 
da Paragem ou se o indice da Paragem já está no vetor de indices de
Paragens da Carreira */
int jaPassaACarreiraOuParagem(int indexes[], int num, int indice) {
    int i;
    for (i = 0; i < num-1; i++){
        if (indexes[i] == indice)
            return 1;
    }
    return 0;
}

/* Cria a primeira Ligação */
void primeiraLigacao (int indexParagem1, int indexParagem2, int indexCarreira) {
    int numCarreiras;
    /* Adição das Paragens ao vetor de indíces de Paragens da Carreira */
    _carreiras[indexCarreira].indexLigacoes[0] = indexParagem1; 
    _carreiras[indexCarreira].indexLigacoes[1] = indexParagem2;
    _carreiras[indexCarreira].numLigacoes += 2;
    _carreiras[indexCarreira].numParagens += 2;
    /* Adição da Carreira ao vetor de indices de Carreiras de cada Paragem */
    numCarreiras = _paragens[indexParagem1].numCarreiras;
    _paragens[indexParagem1].indexCarreiras[numCarreiras] = indexCarreira;
    _paragens[indexParagem1].numCarreiras += 1;

    numCarreiras = _paragens[indexParagem2].numCarreiras;
    _paragens[indexParagem2].indexCarreiras[numCarreiras] = indexCarreira;
    _paragens[indexParagem2].numCarreiras += 1;
}

/* Adiciona a paragem da nova ligação no fim do vetor de indíces */
void adicionaNoFim (int indexParagem2, int indexCarreira, 
                    int numCarreiras, int numLigacoes) {
    /* Se a carreira ou paragem já tiver sido adicionada, não atualiza nem adiciona de novo */
    if (!jaPassaACarreiraOuParagem(_carreiras[indexCarreira].indexLigacoes,
                                        _carreiras[indexCarreira].numLigacoes,
                                        indexParagem2)) {

            _carreiras[indexCarreira].numParagens += 1;
            _paragens[indexParagem2].indexCarreiras[numCarreiras] = indexCarreira;
            _paragens[indexParagem2].numCarreiras += 1;
    }

    _carreiras[indexCarreira].indexLigacoes[numLigacoes] = indexParagem2;
    _carreiras[indexCarreira].numLigacoes += 1;

    /* Se a ligação for circular, tem mais uma paragem */
    if (ligacaoCircular(indexCarreira, 
    _carreiras[indexCarreira].numLigacoes)) {
        _carreiras[indexCarreira].numParagens += 1;
    }
}

/* Adiciona a paragem da nova ligação no inicio do vetor de indíces */
void adicionaNoInicio (int indexParagem1, int indexCarreira,
                        int numCarreiras, int numLigacoes ) {
    /* Se a carreira ou paragem já tiver sido adicionada, não atualiza nem adiciona de novo */
    if (!jaPassaACarreiraOuParagem(_carreiras[indexCarreira].indexLigacoes,
                                        _carreiras[indexCarreira].numLigacoes,
                                        indexParagem1)) {

        _carreiras[indexCarreira].numParagens += 1;
        _paragens[indexParagem1].indexCarreiras[numCarreiras] = indexCarreira;
        _paragens[indexParagem1].numCarreiras += 1;
    }

    mudarParagemOrigem(_carreiras[indexCarreira].indexLigacoes, 
                        numLigacoes, indexParagem1);
    _carreiras[indexCarreira].numLigacoes += 1;

    /* Se a ligação for circular, tem mais uma paragem */
    if (ligacaoCircular(indexCarreira, _carreiras[indexCarreira].numLigacoes)) {
        _carreiras[indexCarreira].numParagens += 1;
    }
}

/* Cria as Ligacoes */
void criaLigacao(int indexParagem1, int indexParagem2, int indexCarreira, 
                float custo, float duracao) {
    int numLigacoes, numCarreiras;
    numLigacoes = _carreiras[indexCarreira].numLigacoes;

    if (numLigacoes == 0) {
        primeiraLigacao(indexParagem1, indexParagem2, indexCarreira);
    }
    /* Verifica qual dos dois possíveis inputs foi colocado */
    else if (_carreiras[indexCarreira].indexLigacoes[numLigacoes-1] == 
            indexParagem1) {
        numCarreiras = _paragens[indexParagem2].numCarreiras;

        adicionaNoFim(indexParagem2, indexCarreira, numCarreiras, numLigacoes);
    }

    else {
        numCarreiras = _paragens[indexParagem1].numCarreiras;
        
        adicionaNoInicio(indexParagem1, indexCarreira, numCarreiras, 
                        numLigacoes);
        }

    /* Incremento do custo e duração */
    _carreiras[indexCarreira].custo += custo;
    _carreiras[indexCarreira].duracao += duracao;
}

/* Verifica um potencial erro na criação de uma ligação */
int erroLigacao(int indexCarreira, int indexParagem1, int indexParagem2) {
    int numLigacoes;
    numLigacoes = _carreiras[indexCarreira].numLigacoes;
    /* As duas únicas possiblidades de input para se formar uma ligação */
    if (indexParagem1 == _carreiras[indexCarreira].indexLigacoes[numLigacoes-1] ||
        indexParagem2 == _carreiras[indexCarreira].indexLigacoes[0]) {

        return 0;
    }
    return 1;    
}

/* Verifica um potencial erro nos valores do custo e duração */
int erroValoresNegativos(float custo, float duracao) {
    if (custo < 0 || duracao < 0) {
        return 1;
    }
    return 0;
}

/* Retira os argumentos necessários para criar as ligações e verifica
potenciais erros na criação destas */
void comandoLigacoes(char buffer[]) {
    char nome[MAX_NOME_PARAGEM];
    char inteiros[ARR_SIZE];
    int indexCarreira, indexParagem1, indexParagem2, numLigacoes;
    float custo, duracao;

    removerComandoDoBuffer(buffer);
    leProximaPalavra(nome, buffer);

    indexCarreira = encontraCarreira(nome);
    if (indexCarreira == NAO_EXISTE) {
        printf ("%s: no such line.\n", nome);
    }

    /* Verifica se o nome da 1ªparagem está entre aspas */
    else {
        memset(nome, 0, sizeof(nome));
        if (temAspas(buffer)) {
            tirarPalavraEntreAspas(buffer, nome);
        }

        else {
            leProximaPalavra(nome, buffer);
        }

        indexParagem1 = encontraParagem(nome);
        if(indexParagem1 == NAO_EXISTE) {
            printf ("%s: no such stop.\n", nome);
        }

        else {
            /* Verifica se o nome da 2ªparagem está entre aspas */
            memset(nome, 0, sizeof(nome));
            if (temAspas(buffer)) {
                tirarPalavraEntreAspas(buffer, nome);
            }

            else {
                leProximaPalavra(nome, buffer);
            }

            indexParagem2 = encontraParagem(nome);
            if (indexParagem2 == NAO_EXISTE) {
                printf ("%s: no such stop.\n", nome);
            }

            else {
                leProximaPalavra(inteiros, buffer);

                custo = atof(inteiros);
                duracao = atof(buffer);
                /* Verifica se os valores são negativos */
                if (erroValoresNegativos(custo, duracao)) {
                    printf ("negative cost or duration.\n");  
                }

                else {
                     numLigacoes = _carreiras[indexCarreira].numLigacoes;
                    /* Verfica se a ligação pode ser feita */
                    if (numLigacoes != 0 && 
                        erroLigacao(indexCarreira, indexParagem1, indexParagem2)) {                        
                        printf ("link cannot be associated with bus line.\n");
                    }

                    else {
                        criaLigacao(indexParagem1, indexParagem2, 
                                    indexCarreira, custo, duracao);
                    }
                }   
            }   
        }   
    }
}

/* Comando Interseções */

/* Compara os nomes das duas carreiras */
int comparaCarreiras(int a, int b) {
    return (strcmp(_carreiras[a].nome, _carreiras[b].nome) > 0);
}

/* Imprime as Interseções */
void mostrarIntersecoes (int index) {
    int numCarreiras, i;

    numCarreiras = _paragens[index].numCarreiras;
    /* Ordenção dos indices consoante o nome da carreira */
    bubbleSort(_paragens[index].indexCarreiras, numCarreiras, comparaCarreiras);

    printf ("%s %d:",_paragens[index].nome, _paragens[index].numCarreiras);

    for (i = 0; i < numCarreiras; i++) {
        printf(" %s", _carreiras[_paragens[index].indexCarreiras[i]].nome);
    }

    printf("\n");
}

/* Lista todas as interseções */
void comandoIntersecoes() {
    int i;
    for (i = 0; i < _numParagens; i++) {
        if (_paragens[i].numCarreiras > 1) {
            mostrarIntersecoes(i);
        }
    }
}

/* Main */

int main() {  
    char buffer[BUFFER_SIZE];

    while(buffer[0] != EOF){
        lePalavraAteFimDeLinha(buffer);
        
        /* Possíveis casos de "inputs" */
        switch (buffer[0]) {
        case 'q': return 0;

        case 'c': comandoCarreiras(buffer);
            memset(buffer, 0, sizeof(buffer));
            break;
        
        case 'p': comandoParagens(buffer);
            memset(buffer, 0, sizeof(buffer));
            break; 

        case 'l': comandoLigacoes(buffer);
            memset(buffer, 0, sizeof(buffer));
            break; 

        case 'i': comandoIntersecoes();
            memset(buffer, 0, sizeof(buffer));
            break;

        default: 
            printf ("Invalid comand: %s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        } 
    }
    
    return 0;
}
