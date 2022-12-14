#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allegro */
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

/* Gameplay */
#include "ballzGameplay.h"

/* Inicializa os quadrados com uma posição inicial */
void blocksInit(square_t square[SIZE_BLOCK_LINES][SIZE_BLOCK_COLUMNS], ballBlock_t ballBlock[SIZE_BLOCK_LINES][SIZE_BLOCK_COLUMNS])
{
    srand(time(NULL));

    /* To do: Implementar linha nula e de gameover corretamente*/

    for(int i = 0; i < SIZE_BLOCK_LINES; i++)
    {
        for(int j= 0; j < SIZE_BLOCK_COLUMNS; j++)
        {
            square[i][j].x1 = MARGIN + SQUARE_WIDTH * j;
            square[i][j].y1 = LIMIT_Y_GAME + SQUARE_HEIGHT * (i+1) + MARGIN;
            square[i][j].x2 = SQUARE_WIDTH * (j+1);
            square[i][j].y2 = LIMIT_Y_GAME + SQUARE_HEIGHT * (i+2);

            ballBlock[i][j].x = (square[i][j].x1 + square[i][j].x2)/2;
            ballBlock[i][j].y = (square[i][j].y1 + square[i][j].y2)/2;
            
            square[i][j].alive = false;
            square[i][j].life  = 0;

            ballBlock[i][j].alive = false;

            square[i][j].text.x = (square[i][j].x1 + square[i][j].x2)  / 2;
            square[i][j].text.y = ((square[i][j].y1 + square[i][j].y2) / 2) - FONT_NUM_SIZE/3;

            // To do: fazer com que o life apareça no meio do quadrado   
        }
    }

    /* Primeira geração da linha aleatória inicial */
    int i = 0;
    int alives = 0;
    for(int j= 0; j < SIZE_BLOCK_COLUMNS; j++)
    {
        if(alives < MAX_ALIVE)
        {
            square[i][j].alive = rand() % 2;
                    
            if(square[i][j].alive)
            {
                if( !(square[i][j].life = rand() % 2) )
                    square[i][j].life = 1;
                alives++;
            }
            else 
                square[i][j].life = 0;
        }
        else
        {
            square[i][j].alive = false;
            square[i][j].life = 0;
        }
    }
}

/* Collide Functions */

bool collide(int p1_x1, int p1_y1, int p1_x2, int p1_y2, int p2_x1, int p2_y1, int p2_x2, int p2_y2)
{
    if(p1_x1 > p2_x2)
        return false;
    if(p1_x2 < p2_x1)
        return false;
    if(p1_y1 > p2_y2)
        return false;
    if(p1_y2 < p2_y1)
        return false;

    return true;
}


bool collide_vertical(float square_x1, float square_y1, float square_x2, float square_y2, float ball_x, float ball_y, float dx, float dy)
{
    /* Testa colisao por baixo */
    if((ball_x >= square_x1 - SPEED) && (ball_x <= square_x2 + SPEED))
    {
        if(abs(ball_y - square_y2) <= SPEED)
        {
            if(dy < 0)
                return true;
        }
    }

    /* Testa colisao por cima */
    if((ball_x >= square_x1 - SPEED) && (ball_x <= square_x2 + SPEED))
    {
        if(abs(ball_y - square_y1) <= SPEED)
        {
            if(dy > 0)
                return true;
        }
    }        

    return false;
}

bool collide_lateral(float square_x1, float square_y1, float square_x2, float square_y2, float ball_x, float ball_y, float dx, float dy)
{
    /* Testa colisao pela esquerda */
    if((ball_y >= square_y1 - SPEED) && (ball_y <= square_y2 + SPEED))
    {
        if(abs(ball_x - square_x1) <= SPEED)
        {
            if(dx > 0)
                return true;
        }
    }
    
    /* Testa colisao pela direita */
    if((ball_y >= square_y1 - SPEED) && (ball_y <= square_y2 + SPEED))
    {
        if( (ball_x >= square_x2) && (abs(ball_x - square_x2) <= SPEED) ) 
        {
            if(dx < 0)
                return true;
        }
    }

    return false;
}

int click_centre_button(ALLEGRO_MOUSE_STATE *mouse_state)
{
    return collide
    (
        mouse_state->x,
        mouse_state->y,
        mouse_state->x,
        mouse_state->y,
        PLAY_BUTTON_POS_X1,
        PLAY_BUTTON_POS_Y1,
        PLAY_BUTTON_POS_X2,
        PLAY_BUTTON_POS_Y2
    );
}

int click_side_button(ALLEGRO_MOUSE_STATE *mouse_state)
{
    return collide
    (
        mouse_state->x,
        mouse_state->y,
        mouse_state->x,
        mouse_state->y,
        MARGIN*2,
        LIMIT_Y_GAME/7,
        DISPLAY_WIDTH/5, 
        LIMIT_Y_GAME/2
    );
}

int scoreCompare(int lastScore)
{
    FILE *arq;
    int scoreArq;

    arq = fopen(".highest_score.txt", "r");
    
    /* Se não existe arquivo de score, cria-se um com o score da última partida*/ 
    if( !arq )
    {
        printf("Criando arquivo de High Score...\n");

        arq = fopen(".highest_score.txt", "w");
        fprintf(arq, "%d", lastScore);
        fclose(arq);

        return lastScore;
    }
    /* Existe arquivo de score, então compara-se o score da última partida com o score do arquivo */
    else
    {
        fscanf(arq, "%d", &scoreArq);

        if(scoreArq < lastScore)
        {
            fclose(arq);

            arq = fopen(".highest_score.txt", "w");
            fprintf(arq, "%d", lastScore);

            fclose(arq);

            printf("%d é maior que %d. Substituindo High Score...\n", lastScore, scoreArq);
            return lastScore;
        }
        else
        {
            fclose(arq);
            printf("Mantém-se o score do arquivo.\n");
            return scoreArq;
        }
    }
}

ball_t* createBall(float x, float y)
{
    ball_t *ball = malloc(sizeof(ball_t*));

    ball->x = x;
    ball->y = y;
    ball->dx = 0.0;
    ball->dy = 0.0;
    ball->ground = true;
    ball->moving = false;
    ball->launched = false;

    return ball;
}

ball_t** initBallsArray(ball_t* ball)
{
    ball_t** aux = malloc(sizeof(ball_t*));
    aux[0] = ball;
    return aux;
}

ball_t** addBalls(ball_t** balls, ball_t * aux, int tam)
{
    balls = realloc(balls, (tam+1)*sizeof(ball_t*));
    balls[tam] = aux;
    return balls;
}

int ballsGround(ball_t** balls, int tam)
{
    for(int i = 0; i < tam; i++)
        if(balls[i]->moving)
            return 0;
    return 1;
}

void resetBalls(ball_t** balls, float initialX, int tam)
{
    for(int i = 0; i < tam; i++)
    { 
        balls[i]->x = initialX;
        balls[i]->y = BALL_FLOOR;
    }
}
