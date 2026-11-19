#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum tokens {
	INDEFINIDO,
	IDENTIFICADOR,
	SIMBOLO,
	STRING,
	INTEIRO,
	PALAVRA_CHAVE
};

struct no_fechamentos {
	struct no_fechamentos* ant;
	char simbolo;
};

struct no_tokens {
	struct no_tokens* prox;
	enum tokens token;
	char elemento[1024];
};

void ler_arquivo(char* buffer_arquivo, int tam_buffer_arquivo, FILE* arq) {
	int offset = 0;
	while (fgets(buffer_arquivo + offset, tam_buffer_arquivo - offset, arq)) {
		offset += (int)strcspn(buffer_arquivo + offset, "\n") + 1;
	}
}

void limpar_entrada(char* buffer_arquivo) {
	for (char* cur_char = buffer_arquivo; *cur_char; ++cur_char) {
		if (*cur_char == '\n' || *cur_char == '\t') *cur_char = ' ';
	}
}

void push_fechamento(char simbolo, struct no_fechamentos** pilha_fechamento) {
	struct no_fechamentos* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) return;

	novo_no->simbolo = simbolo;
	novo_no->ant = *pilha_fechamento;

	*pilha_fechamento = novo_no;
}

bool pop_fechamento(char simbolo, struct no_fechamentos** pilha_fechamento) {
	if ((*pilha_fechamento)->simbolo + 1 != simbolo && (*pilha_fechamento)->simbolo + 2 != simbolo && (*pilha_fechamento)->simbolo != simbolo) return false;

	struct no_fechamentos* temp = *pilha_fechamento;
	*pilha_fechamento = (*pilha_fechamento)->ant;

	free(temp);

	return true;
}

bool validar_fechamentos(const char* buffer_arquivo) {
	struct no_fechamentos* pilha_fechamentos = { 0 };

	int linha_atual = 1;
	int linha_ultima_abertura;

	bool flag_aspas = false;

	for (const char* cur_char = buffer_arquivo; *cur_char; ++cur_char) {
		if (*cur_char == '\n') {
			++linha_atual;
			continue;
		}

		if (flag_aspas) {
			if (*cur_char == '"') {
				pop_fechamento('"', &pilha_fechamentos);
				flag_aspas = false;
			}
			continue;
		}

		switch (*cur_char) {
		case '(':
		case '[':
		case '{':
		case '"':
			push_fechamento(*cur_char, &pilha_fechamentos);
			linha_ultima_abertura = linha_atual;
			if (*cur_char == '"') flag_aspas = true;
			break;
		case ')':
		case ']':
		case '}':
			if (!pop_fechamento(*cur_char, &pilha_fechamentos)) {
				printf("Problema no fechamento '%c' da linha %d com a abertura '%c' da linha %d\n", *cur_char, linha_atual, pilha_fechamentos->simbolo, linha_ultima_abertura);
				return false;
			}
		}
	}

	if (pilha_fechamentos) {
		printf("Abertura '%c' da linha %d nao foi fechado\n", pilha_fechamentos->simbolo, linha_ultima_abertura);
		return false;
	}

	return true;
}

bool validar_linha(const char* buffer_arquivo) {
	int pos_quebra_linha = (int)strcspn(buffer_arquivo, "\n");

	int contador_linhas = 1;

	while (buffer_arquivo[pos_quebra_linha]) {
		if (buffer_arquivo[pos_quebra_linha - 1] != ';' && buffer_arquivo[pos_quebra_linha - 1] != '{' && buffer_arquivo[pos_quebra_linha - 1] != '}') {
			printf("Problema de terminacao na linha %d\n", contador_linhas);
			return false;
		}

		pos_quebra_linha += (int)strcspn(buffer_arquivo + pos_quebra_linha + 1, "\n") + 1;
		++contador_linhas;
	}
	return true;
}

bool validar_entrada(const char* buffer_arquivo) {
	return validar_fechamentos(buffer_arquivo) && validar_linha(buffer_arquivo);
}

bool in(const char* agulha, const char* palheiro, int tam_palheiro, int tam_elem_palheiro) {
	for (int i = 0; i < tam_palheiro; ++i) {
		if (strcmp(agulha, palheiro + (i * tam_elem_palheiro)) == 0) return true;
	}

	return false;
}

void push_token(struct no_tokens** pilha_tokens, const char* elemento, enum tokens token) {
	struct no_tokens* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) return;

	snprintf(novo_no->elemento, sizeof(novo_no->elemento), "%s", elemento);
	novo_no->prox = NULL;
	novo_no->token = token;

	if (!*pilha_tokens) *pilha_tokens = novo_no;
}

void tokenizar(struct no_tokens** inicio, const char* elemento, const char* PALAVRAS_CHAVE, int tam_palavras_chave, int tam_elem_palavras_chave, const char* SIMBOLOS, int tam_simbolos, int tam_elem_simbolos) {
	struct no_tokens** pilha_tokens = inicio;

	char buffer_elemento[1024];
	int idx = 0;

	enum tokens token_atual = INDEFINIDO;

	const char* cur_char = elemento;
	while (*cur_char) {
		if (*cur_char == ' ') {
			++cur_char;
			continue;
		}

		buffer_elemento[idx] = *cur_char;
		buffer_elemento[idx + 1] = '\0';

		if (*cur_char == '"') {
			token_atual ^= STRING;
			if (!token_atual) {
				++cur_char;
				continue;
			}
			++cur_char;

			while (*cur_char != '"') {
				buffer_elemento[idx++] = *cur_char++;
			}
		}
		else if (in(buffer_elemento, SIMBOLOS, tam_simbolos, tam_elem_simbolos)) {
			buffer_elemento[++idx] = *(cur_char + 1);
			buffer_elemento[idx + 1] = '\0';

			if (in(buffer_elemento, SIMBOLOS, tam_simbolos, tam_elem_simbolos)) {
				++idx;
				++cur_char;
			}

			token_atual = SIMBOLO;
		}
		else {
			if (*cur_char >= '0' && *cur_char <= '9') token_atual = INTEIRO;
			char buffer_simbolo[2];
			buffer_simbolo[1] = '\0';
			while (*cur_char != ' ') {
				buffer_simbolo[0] = *cur_char;
				if (in(buffer_simbolo, SIMBOLOS, tam_simbolos, tam_elem_simbolos)) {
					--cur_char;
					break;
				}

				buffer_elemento[idx++] = *cur_char++;
			}

			if (!token_atual) token_atual = (in(buffer_elemento, PALAVRAS_CHAVE, tam_palavras_chave, tam_elem_palavras_chave)) ? PALAVRA_CHAVE : IDENTIFICADOR;
		}

		buffer_elemento[idx] = '\0';
		push_token(pilha_tokens, buffer_elemento, token_atual);
		idx = 0;

		token_atual = INDEFINIDO;

		if (!*cur_char) break;

		++cur_char;

		pilha_tokens = &(*pilha_tokens)->prox;
	}
}

void print_tokens(struct no_tokens* no_atual) {
	puts("<tokens>");
	char token[14];
	while (no_atual) {
		switch (no_atual->token) {
		case IDENTIFICADOR: memcpy(token, "identifier", sizeof("identifier")); break;
		case SIMBOLO: memcpy(token, "symbol", sizeof("symbol")); break;
		case PALAVRA_CHAVE: memcpy(token, "keyword", sizeof("keyword")); break;
		case INTEIRO: memcpy(token, "constInt", sizeof("constInt")); break;
		case STRING: memcpy(token, "string", sizeof("string")); break;
		}

		printf("<%s> %s <\\%s>\n", token, no_atual->elemento, token);

		no_atual = no_atual->prox;
	}
	puts("<\\tokens>");
}

void liberar_nos(struct no_tokens* no_atual) {
	while (no_atual) {
		struct no_tokens* temp = no_atual->prox;
		free(no_atual);
		no_atual = temp;
	}
}

int main(void) {
	const char PALAVRAS_CHAVE[][12] = {
		"class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return"
	};
	const int tam_elem_palavras_chave = sizeof(*PALAVRAS_CHAVE);
	const int tam_palavras_chave = sizeof(PALAVRAS_CHAVE) / tam_elem_palavras_chave;

	const char SIMBOLOS[][3] = { "{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/", "&", "|", "<", ">", "=", "~", "!", "==", "!=", "<=", ">=" };
	const int tam_elem_simbolos = sizeof(*SIMBOLOS);
	const int tam_simbolos = sizeof(SIMBOLOS) / tam_elem_simbolos;

	FILE* arq = fopen("teste.txt", "r");
	if (!arq) {
		puts("arquivo nao encotnrado");
		return 1;
	}

	struct no_tokens* inicio = NULL;

	char buffer_arquivo[1024];
	ler_arquivo(buffer_arquivo, sizeof(buffer_arquivo), arq);

	if (!validar_entrada(buffer_arquivo)) {
		fclose(arq);
		return 1;
	}

	limpar_entrada(buffer_arquivo);

	tokenizar(&inicio, buffer_arquivo, *PALAVRAS_CHAVE, tam_palavras_chave, tam_elem_palavras_chave, *SIMBOLOS, tam_simbolos, tam_elem_simbolos);

	print_tokens(inicio);

	fclose(arq);

	liberar_nos(inicio);

	return 0;
}