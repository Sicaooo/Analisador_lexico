#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
	PALAVRA_CHAVE,
	SIMBOLO,
	NUMERO,
	STRING,
	IDENTIFICADOR
} NomeTokens;

typedef struct {
	char elemento[20];
	NomeTokens tipo;
} Token;

typedef struct no {
	Token token;
	struct no* prox;
} no;

void liberar_nos(no* atual) {
	no* prox;
	while ((prox = atual->prox)) {
		free(atual);
		atual = prox;
	}
}

bool push(no** no_atual, char* elemento, int tam_elemento, NomeTokens tipo) {
	if (!*no_atual) {
		*no_atual = malloc(sizeof(**no_atual));
		if (!*no_atual) return false;
	}
	
	memcpy((*no_atual)->token.elemento, elemento, tam_elemento);
	(*no_atual)->token.elemento[tam_elemento] = '\0';

	(*no_atual)->token.tipo = tipo;
	(*no_atual)->prox = NULL;
	
	return true;
}

bool in(const char* agulha, const char* palheiro, int tam_palheiro, int tam_elem_palheiro) {
	for (int i = 0; i < tam_palheiro; ++i) {
		if (strncmp(agulha, palheiro + i * tam_elem_palheiro, tam_elem_palheiro) == 0) return true;
	}
	return false;
}

NomeTokens tokenizador(const char* elemento, const char* PALAVRAS_CHAVE, int tam_palavras_chave) {
	switch (*elemento) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': return NUMERO; break;
		case '"': return STRING; break;
	}

	if (in(elemento, PALAVRAS_CHAVE, tam_palavras_chave, 12)) return PALAVRA_CHAVE;

	return IDENTIFICADOR;
}

int main(void) {
	const char PALAVRAS_CHAVE[][12] = { 
		"class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return" 
	};
	const int QNT_PALAVRAS_CHAVE = sizeof(PALAVRAS_CHAVE) / sizeof(*PALAVRAS_CHAVE);
	const char SIMBOLOS[] = "{}()[].,;+-*/&|<>=~";
	const int QNT_SIMBOLOS = sizeof(SIMBOLOS);

	FILE* entrada_arquivo = fopen("teste.txt", "r");
	if (!entrada_arquivo) {
		printf("Arquivo nao encontrado\n");
		return 1;
	}

	char buffer[256];
	if (!fgets(buffer, sizeof(buffer), entrada_arquivo)) {
		printf("Falha ao ler arquivo\n");
		fclose(entrada_arquivo);
		return 1;
	}

	no* inicio = malloc(sizeof(*inicio));
	if (!inicio) return 1;

	memset(inicio, 0, sizeof(*inicio));

	no** no_atual = &inicio;

	buffer[strcspn(buffer, "\n")] = '\0';

	char* char_atual = buffer;
	while (*char_atual) {
		char elem_buffer[100];
		if (in(char_atual, SIMBOLOS, QNT_SIMBOLOS, 1)) {
			if (!push(no_atual, char_atual, 1, SIMBOLO)) {
				printf("Nao foi possivel adicionar a pilha\n");
				liberar_nos((*inicio).prox);
				return 1;
			}
			do { ++char_atual; } while (*char_atual == ' ');

			no_atual = &(*no_atual)->prox;

			continue;
		}
		if (*char_atual >= '0' && *char_atual <= '9') {
			int i;
			for (i = 0; char_atual[i] >= '0' && char_atual[i] <= '9'; ++i) {
				elem_buffer[i] = char_atual[i];
			}
			elem_buffer[i] = '\0';

			if (!push(no_atual, elem_buffer, i, NUMERO)) {
				printf("Nao foi possivel adicionar a pilha\n");
				liberar_nos(inicio->prox);
				return 1;
			}
			
			char_atual += i;
			
			no_atual = &(*no_atual)->prox;

			continue;
		}
		if (*char_atual == '"') {
			int i;
			for (i = 0; *char_atual != '"'; ++i) {
				elem_buffer[i] = *char_atual;
			}
			elem_buffer[i] = '\0';

			if (!push(no_atual, elem_buffer, i, STRING)) {
				printf("Nao foi possivel adicionar a pilha\n");
				return 1;
			}
			 
			char_atual += i;

			no_atual = &(*no_atual)->prox;

			continue;
		}
		int i;
		for (i = 0; *char_atual != ' ' && !in(char_atual, SIMBOLOS, QNT_SIMBOLOS, 1); ++i) {
			elem_buffer[i] = *char_atual;
			++char_atual;
		}
		elem_buffer[i] = '\0';
		
		if (!push(no_atual, elem_buffer, i, (in(elem_buffer, *PALAVRAS_CHAVE, QNT_PALAVRAS_CHAVE, 12)) ? PALAVRA_CHAVE : IDENTIFICADOR)) {
			printf("Nao foi possivel adicionar a pilha\n");
			return 1;
		}

		++char_atual;

		no_atual = &(*no_atual)->prox;
	}

	fclose(entrada_arquivo);

	printf("<tokens>\n");

	no_atual = &inicio;
	while (*no_atual) {
		switch ((*no_atual)->token.tipo) {
			case PALAVRA_CHAVE: printf("<keyword> %s </keyword>\n", (*no_atual)->token.elemento); break;
			case SIMBOLO: printf("<symbol> %s </symbol>\n", (*no_atual)->token.elemento); break;
			case NUMERO: printf("<intConst> %s </intConst>\n", (*no_atual)->token.elemento); break;
			case STRING: printf("<stringConst> %s </stringConst>\n", (*no_atual)->token.elemento); break;
			case IDENTIFICADOR: printf("<identifier> %s </identifier>\n", (*no_atual)->token.elemento); break;
		}

		no_atual = &(*no_atual)->prox;
	}

	printf("</tokens>\n");

	liberar_nos(inicio->prox);

	return 0;
}
//NomeTokens token_atual;
//bool verificar_simbolo = true;
//bool flag_token = false;
//char* inicio = buffer;
//char elemento_atual[20];
//
//for (char* atual = buffer; atual; ++atual) {
//	*elemento_atual = *atual;
//	elemento_atual[1] = '\0';
//	
//	if (verificar_simbolo && in(elemento_atual, SIMBOLOS, QNT_SIMBOLOS, 1)) {
//		token_atual = SIMBOLO;
//		flag_token = true;
//		inicio = atual + 1;
//	}
//	else if (!verificar_simbolo && *atual == ' ') {
//		int tam_elemento = atual - inicio;
//		memcpy(elemento_atual, inicio, tam_elemento);
//		elemento_atual[tam_elemento] = '\0';
//		
//		token_atual = tokenizador(elemento_atual, PALAVRAS_CHAVE, QNT_PALAVRAS_CHAVE);
//		verificar_simbolo = true;

//		flag_token = true;
//		
//		inicio = atual + 1;
//	}
//	else verificar_simbolo = false;
//	
//	if (flag_token) {
//		switch (token_atual) {
//			case PALAVRA_CHAVE: printf("<keyword> %s </keyword>\n", elemento_atual); break;
//			case SIMBOLO: printf("<symbol> %s </symbol>\n", elemento_atual); break;
//			case NUMERO: printf("<intConst> %s </intConst>\n", elemento_atual); break;
//			case STRING: printf("<stringConst> %s </stringConst>\n", elemento_atual); break;
//			case IDENTIFICADOR: printf("<identifier> %s </identifier>\n", elemento_atual); break;
//		}

//		flag_token = false;
//	}
//}