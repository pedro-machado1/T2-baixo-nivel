/*
gcc main.c -I SDL2\i686-w64-mingw32\include -L SDL2\i686-w64-mingw32\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -o main.exe
*/

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> 

#define LARGURA_TELA 700
#define ALTURA_TELA 700
#define LINHAS 6
#define COLUNAS 7

#define LARGURA_CELULA 80
#define ALTURA_CELULA 80

const SDL_Color COR_FUNDO = {240, 240, 240, 255};

typedef enum { MENU, JOGANDO, FIM_DE_JOGO } EstadoJogo;
typedef enum { JOGADOR_VS_JOGADOR, JOGADOR_VS_PC } ModoJogo;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int tabuleiro[LINHAS][COLUNAS];
int jogadorAtual;
bool rodando = true;
EstadoJogo estadoJogo = MENU;
ModoJogo modoJogo;
int vencedor = 0;
bool empate = false;

SDL_Texture* boardImage = NULL;
SDL_Texture* yellowPieceImage = NULL;
SDL_Texture* redPieceImage = NULL;

SDL_Texture* tituloImage = NULL;
SDL_Texture* botaoJxjImage = NULL;
SDL_Texture* botaoJxpImage = NULL;
SDL_Texture* turnoJogador1Image = NULL;
SDL_Texture* turnoJogador2Image = NULL;
SDL_Texture* turnoPcImage = NULL;
SDL_Texture* vitoriaJogador1Image = NULL;
SDL_Texture* vitoriaJogador2Image = NULL;
SDL_Texture* vitoriaPC = NULL;
SDL_Texture* empateImage = NULL;
SDL_Texture* jogarNovamenteImage = NULL;

bool inicializar();
void fechar();
SDL_Texture* carregarTextura(const char* path);
void reiniciarJogo();
void manipularEventos(SDL_Event e);
void renderizar();
void desenharTabuleiro();
void desenharPecas();
void desenharUI();
void desenharMenu();
void desenharFimDeJogo();
bool movimentoValido(int col);
int obterProximaLinhaLivre(int col);
void soltarPeca(int linha, int col, int jogador);
bool checarVitoria(int jogador);
bool checarEmpate();
int escolherColunaPC();

int main(int argc, char* args[]) {
    if (!inicializar()) {
        printf("Falha ao inicializar!\n");
        return -1;
    }
    SDL_Event e;
    while (rodando) {
        while (SDL_PollEvent(&e) != 0) {
            manipularEventos(e);
        }
        if (estadoJogo == JOGANDO && modoJogo == JOGADOR_VS_PC && jogadorAtual == 2 && vencedor == 0) {
            SDL_Delay(500);
            int col = escolherColunaPC();
            if (movimentoValido(col)) {
                int linha = obterProximaLinhaLivre(col);
                soltarPeca(linha, col, jogadorAtual);
                if (checarVitoria(jogadorAtual)) {
                    vencedor = jogadorAtual;
                    estadoJogo = FIM_DE_JOGO;
                } else if (checarEmpate()) {
                    empate = true;
                    estadoJogo = FIM_DE_JOGO;
                } else {
                    jogadorAtual = 1;
                }
            }
        }
        renderizar();
    }
    fechar();
    return 0;
}



SDL_Texture* carregarTextura(const char* path) {
    SDL_Texture* novaTextura = IMG_LoadTexture(renderer, path);
    if (novaTextura == NULL) {
        printf("Não foi possível carregar a imagem %s! SDL_image Error: %s\n", path, IMG_GetError());
    }
    return novaTextura;
}

bool inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) return false;

    window = SDL_CreateWindow("Conecta 4", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGURA_TELA, ALTURA_TELA, SDL_WINDOW_SHOWN);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;
    boardImage = carregarTextura("fichas/jogo_tabuleiro.png");
    yellowPieceImage = carregarTextura("fichas/ficha_amarela.png");
    redPieceImage = carregarTextura("fichas/ficha_vermelha.png");
    tituloImage = carregarTextura("fichas/titulo.png");
    botaoJxjImage = carregarTextura("fichas/botao1.png");
    botaoJxpImage = carregarTextura("fichas/botao2.png");
    vitoriaJogador1Image = carregarTextura("fichas/jogador1.png");
    vitoriaJogador2Image = carregarTextura("fichas/jogador2.png");
    vitoriaPC = carregarTextura("fichas/vitoriaPC.png");
    empateImage = carregarTextura("fichas/empate.png");
    jogarNovamenteImage = carregarTextura("fichas/novamente.png");

    reiniciarJogo();
    return true;
}

void fechar() {
    SDL_DestroyTexture(boardImage);
    SDL_DestroyTexture(yellowPieceImage);
    SDL_DestroyTexture(redPieceImage);
    SDL_DestroyTexture(tituloImage);
    SDL_DestroyTexture(botaoJxjImage);
    SDL_DestroyTexture(botaoJxpImage);
    SDL_DestroyTexture(vitoriaJogador1Image);
    SDL_DestroyTexture(vitoriaJogador2Image);
    SDL_DestroyTexture(vitoriaPC);
    SDL_DestroyTexture(empateImage);
    SDL_DestroyTexture(jogarNovamenteImage);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void reiniciarJogo() {
    for (int i = 0; i < LINHAS; ++i) {
        for (int j = 0; j < COLUNAS; ++j) {
            tabuleiro[i][j] = 0;
        }
    }
    jogadorAtual = 1;
    vencedor = 0;
    empate = false;
}

void manipularEventos(SDL_Event e) {
    if (e.type == SDL_QUIT) {
        rodando = false;
    }
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        if (estadoJogo == MENU) {
            if (x > 200 && x < 500 && y > 350 && y < 425) {
                modoJogo = JOGADOR_VS_JOGADOR;
                estadoJogo = JOGANDO;
                reiniciarJogo();
            }
            if (x > 200 && x < 500 && y > 475 && y < 575) {
                modoJogo = JOGADOR_VS_PC;
                estadoJogo = JOGANDO;
                reiniciarJogo();
            }
        } else if (estadoJogo == JOGANDO && (modoJogo == JOGADOR_VS_JOGADOR || jogadorAtual == 1)) {
            int col = (x-50) / LARGURA_CELULA;
            if (col >= 0 && col < COLUNAS && movimentoValido(col)) {
                int linha = obterProximaLinhaLivre(col);
                soltarPeca(linha, col, jogadorAtual);
                if (checarVitoria(jogadorAtual)) {
                    vencedor = jogadorAtual;
                    estadoJogo = FIM_DE_JOGO;
                } else if (checarEmpate()) {
                    empate = true;
                    estadoJogo = FIM_DE_JOGO;
                } else {
                    jogadorAtual = (jogadorAtual == 1) ? 2 : 1;
                }
            }
        } else if (estadoJogo == FIM_DE_JOGO) {
            estadoJogo = MENU;
        }
    }
}

void desenharTexturaCentro(SDL_Texture* tex, int y, int largura) {
    int x = (LARGURA_TELA - largura) / 2;
    SDL_Rect dest = {x, y, largura, 100}; 
    SDL_RenderCopy(renderer, tex, NULL, &dest);
}

void renderizar() {
    SDL_SetRenderDrawColor(renderer, COR_FUNDO.r, COR_FUNDO.g, COR_FUNDO.b, COR_FUNDO.a);
    SDL_RenderClear(renderer);

    if (estadoJogo == MENU) {
        desenharMenu();
    } else {
        desenharTabuleiro();
        desenharPecas();
        if (estadoJogo == FIM_DE_JOGO) {
            desenharFimDeJogo();
        }
    }
    SDL_RenderPresent(renderer);
}

void desenharTabuleiro() {
    SDL_Rect destRect = {0, 125, 700, 600};
    SDL_RenderCopy(renderer, boardImage, NULL, &destRect);
    SDL_Rect titulo = {0, 0, LARGURA_TELA, 125};
    SDL_RenderCopy(renderer, tituloImage, NULL, &titulo);
}

void desenharPecas() {
    for (int i = 0; i < LINHAS; ++i) {
        for (int j = 0; j < COLUNAS; ++j) {
            if (tabuleiro[i][j] != 0) {
                SDL_Texture* pecaAtualImage = (tabuleiro[i][j] == 1) ? yellowPieceImage : redPieceImage;
                SDL_Rect destRect = {48+(j * (LARGURA_CELULA+5.1)), ((ALTURA_TELA - (LINHAS * ALTURA_CELULA)) + (i * (ALTURA_CELULA+11))-72), LARGURA_CELULA, ALTURA_CELULA};
                SDL_RenderCopy(renderer, pecaAtualImage, NULL, &destRect);
            }
        }
    }
}


void desenharMenu() {
    SDL_Rect titulo = {200, 0, 300 , 300};
    SDL_RenderCopy(renderer, tituloImage, NULL, &titulo);
    SDL_Rect r_jxj = {200, 300, 300, 150};
    SDL_RenderCopy(renderer, botaoJxjImage, NULL, &r_jxj);
    SDL_Rect r_jxp = {200, 450, 300, 150};
    SDL_RenderCopy(renderer, botaoJxpImage, NULL, &r_jxp);
}

void desenharFimDeJogo() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
    SDL_Rect overlay = {0, 0, LARGURA_TELA, ALTURA_TELA};
    SDL_RenderFillRect(renderer, &overlay);
    
    if (vencedor == 1) {
        desenharTexturaCentro(vitoriaJogador1Image, ALTURA_TELA / 2 - 50, 400);
    } else if (vencedor == 2) {
        if (modoJogo == JOGADOR_VS_JOGADOR) {
            desenharTexturaCentro(vitoriaJogador2Image, ALTURA_TELA / 2 - 50, 400);
        } else {
            desenharTexturaCentro(vitoriaPC, ALTURA_TELA / 2 - 50, 400);
        }
    } else if (empate) {
        desenharTexturaCentro(empateImage, ALTURA_TELA / 2 - 50, 400);
    }
    
    desenharTexturaCentro(jogarNovamenteImage, ALTURA_TELA / 2 + 20, 400);
}


bool movimentoValido(int col) { return tabuleiro[0][col] == 0; }
int obterProximaLinhaLivre(int col) {
    for (int i = LINHAS - 1; i >= 0; i--) {
        if (tabuleiro[i][col] == 0) return i;
    }
    return -1;
}
void soltarPeca(int linha, int col, int jogador) { tabuleiro[linha][col] = jogador; }
bool checarVitoria(int jogador) {
    // Horizontal
    for (int l = 0; l < LINHAS; l++) for (int c = 0; c < COLUNAS - 3; c++) if (tabuleiro[l][c] == jogador && tabuleiro[l][c + 1] == jogador && tabuleiro[l][c + 2] == jogador && tabuleiro[l][c + 3] == jogador) return true;
    // Vertical
    for (int c = 0; c < COLUNAS; c++) for (int l = 0; l < LINHAS - 3; l++) if (tabuleiro[l][c] == jogador && tabuleiro[l + 1][c] == jogador && tabuleiro[l + 2][c] == jogador && tabuleiro[l + 3][c] == jogador) return true;
    // Diagonal 
    for (int c = 0; c < COLUNAS - 3; c++) for (int l = 3; l < LINHAS; l++) if (tabuleiro[l][c] == jogador && tabuleiro[l - 1][c + 1] == jogador && tabuleiro[l - 2][c + 2] == jogador && tabuleiro[l - 3][c + 3] == jogador) return true;
    // Diagonal 
    for (int c = 0; c < COLUNAS - 3; c++) for (int l = 0; l < LINHAS - 3; l++) if (tabuleiro[l][c] == jogador && tabuleiro[l + 1][c + 1] == jogador && tabuleiro[l + 2][c + 2] == jogador && tabuleiro[l + 3][c + 3] == jogador) return true;
    return false;
}
bool checarEmpate() {
    for (int c = 0; c < COLUNAS; c++) {
        if (tabuleiro[0][c] == 0) return false;
    }
    return true; 
}
int escolherColunaPC() {
    for (int c = 0; c < COLUNAS; c++) { // Checa se pode vencer
        if (movimentoValido(c)) {
            int r = obterProximaLinhaLivre(c);
            tabuleiro[r][c] = 2; if (checarVitoria(2)) { tabuleiro[r][c] = 0; return c; } tabuleiro[r][c] = 0;
        }
    }
    for (int c = 0; c < COLUNAS; c++) { // Checa se precisa bloquear
        if (movimentoValido(c)) {
            int r = obterProximaLinhaLivre(c);
            tabuleiro[r][c] = 1; if (checarVitoria(1)) { tabuleiro[r][c] = 0; return c; } tabuleiro[r][c] = 0;
        }
    }
    int col; do { col = rand() % COLUNAS; } while (!movimentoValido(col)); // randomizador
    return col;
}