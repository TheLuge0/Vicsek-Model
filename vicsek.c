// gcc -g -Wall -Wextra -fsanitize=address -o vicsek.exe -lm vicsek.c $(sdl2-config --cflags --libs)

/* Importation des modules */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "vicsek.h"

/* Création du type pour représenter un poisson */
struct rectangle_s{
  SDL_Rect rect;
  double Vx;
  double Vy;
};

typedef struct rectangle_s rectangle_t;

/* Création du type pour représenter plusieurs poissons */
struct rectangle_list_s{
  rectangle_t* poissons;
  int number;
};

typedef struct rectangle_list_s rectangle_list_t;

/* Constantes */
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool game_is_running = true;
SDL_Color bleu = {0, 0, 255, 255};
SDL_Color blanc = {255, 255, 255, 255};

/* Fonction pour créer la fenêtre et le rendu */
bool initialize_window(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        "Modélisation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );
    if (!window){
        fprintf(stderr, "Error creating SDL Window.\n");
        SDL_Quit();
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer){
        fprintf(stderr, "Error creating SDL Renderer.\n");
        SDL_Quit();
        return false;
    }
    return true;
}


/* Fonction pour détruire la fenêtre */
void destroy_window(void){
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


/* Fonction pour récupérer les inputs claviers et fermer la modélisation */
void process_input(){
  SDL_Event event;
  while (SDL_PollEvent(&event)){
      switch (event.type){
          case SDL_QUIT:
              game_is_running = false;
              break;
          case SDL_KEYDOWN:
              if (event.key.keysym.sym == SDLK_ESCAPE){
                  game_is_running = false;
              }
              break;
      }
  }
}


/* Dessiner un vecteur avec une vraie flèche (ligne + pointe en V) */
void DrawVector(SDL_Renderer* renderer, int x, int y, double vx, double vy){
    int x2 = x + (int)(VECTOR_SIZE*vx);
    int y2 = y + (int)(VECTOR_SIZE*vy);

    // Tracer le corps du vecteur
    SDL_RenderDrawLine(renderer, x, y, x2, y2);

    // Calculer l’angle du vecteur
    double angle = atan2(vy, vx);

    // Calcul des deux côtés de la pointe (±30° autour de l'angle du vecteur)
    int xLeft  = x2 - (int)(ARROW_SIZE * cos(angle - M_PI / 6));
    int yLeft  = y2 - (int)(ARROW_SIZE * sin(angle - M_PI / 6));

    int xRight = x2 - (int)(ARROW_SIZE * cos(angle + M_PI / 6));
    int yRight = y2 - (int)(ARROW_SIZE * sin(angle + M_PI / 6));

    // Tracer la pointe en V
    SDL_RenderDrawLine(renderer, x2, y2, xLeft, yLeft);
    SDL_RenderDrawLine(renderer, x2, y2, xRight, yRight);
}

/* Calcul de l'ordre de Vicsek */
double calcul_order_1(rectangle_list_t groupe){
  double sum_x = 0.0;
  double sum_y = 0.0;

  for (int i = 0; i < groupe.number; i ++){
    double Vx = groupe.poissons[i].Vx;
    double Vy = groupe.poissons[i].Vy;
    double norme = sqrt(Vx*Vx + Vy*Vy);

    sum_x += Vx / norme;
    sum_y += Vy / norme;
  }

  sum_x /= groupe.number;
  sum_y /= groupe.number;

  return sqrt(sum_x*sum_x + sum_y*sum_y);
}


/* Fonction pour initialiser une liste de X poisson : type rectangle_list_t */
rectangle_list_t init_list_rectangle_t(int number){
  rectangle_t* groupe = malloc(sizeof(struct rectangle_s)*number);
  for (int i = 0; i < number; i ++){
    /* Création du SDL_Rect */
    SDL_Rect rect = {(rand() % (WINDOW_WIDTH - BORDER_WIDTH)) + BORDER_WIDTH/2, (rand() % (WINDOW_HEIGHT - BORDER_HEIGHT)) + BORDER_HEIGHT/2, FISH_WIDTH, FISH_HEIGHT};

    /* Initialisation de Vx et Vy */
    double theta = fmod((double)rand(), 2*M_PI);
    double Vx = cos(theta) * FISH_SPEED;
    double Vy = sin(theta) * FISH_SPEED;

    /* Création du poisson d'indice i et ajout dans la liste */
    rectangle_t rectangle = {rect, Vx, Vy};
    groupe[i] = rectangle;
  }
  rectangle_list_t result = {groupe, number};
  return result;
}

/* Fonction pour mettre a jour le rendu */
void render(rectangle_list_t groupe, SDL_Color color){
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  for (int i = 0; i < groupe.number; i ++){
    DrawVector(renderer, groupe.poissons[i].rect.x, groupe.poissons[i].rect.y, groupe.poissons[i].Vx, groupe.poissons[i].Vy);
  }
  SDL_RenderPresent(renderer);
}

/* Calculer la nouvelle vitesse du poisson i */
double* get_new_vitesse(int i, rectangle_list_t groupe){
  double* result = malloc(sizeof(double)*2);
  double n = 0;
  double vx = 0, vy = 0;
  for (int j = 0; j < groupe.number; j ++){
    if (pow((groupe.poissons[j].rect.x - groupe.poissons[i].rect.x), 2) + pow((groupe.poissons[j].rect.y - groupe.poissons[i].rect.y), 2) < INFLUENCE_RADIUS*INFLUENCE_RADIUS){
      n += 1;
      vx += groupe.poissons[j].Vx;
      vy += groupe.poissons[j].Vy;
    }
  }
  vx = vx / n;
  vy = vy / n;
  double theta = atan2(vy, vx) + (-NOISE + ((double)rand() / (double)RAND_MAX * (2*NOISE)));
  result[0] = cos(theta) * FISH_SPEED;
  result[1] = sin(theta) * FISH_SPEED;
  return result;
}


void update(rectangle_list_t groupe, int T){
  for (int t = 0; t < T; t ++){
    double* Wx = malloc(sizeof(double)*groupe.number);
    double* Wy = malloc(sizeof(double)*groupe.number);
    for (int i = 0; i < groupe.number; i ++){
      double* result = get_new_vitesse(i, groupe);
      Wx[i] = result[0];
      Wy[i] = result[1];
      free(result);
      }
    for (int i = 0; i < groupe.number; i ++){
      groupe.poissons[i].Vx = Wx[i];
      groupe.poissons[i].Vy = Wy[i];

      double new_x = groupe.poissons[i].rect.x + groupe.poissons[i].Vx * DT;
      double new_y = groupe.poissons[i].rect.y + groupe.poissons[i].Vy * DT;

      // Gestion des murs
      if (new_x < 0){
          new_x += WINDOW_WIDTH;
      }
      else if (new_x >= WINDOW_WIDTH){
          new_x -= WINDOW_WIDTH;
      }

      if (new_y < 0){
          new_y += WINDOW_HEIGHT;
      }
      else if (new_y >= WINDOW_HEIGHT){
          new_y -= WINDOW_HEIGHT;
      }

      // Mise a jour des positions
      groupe.poissons[i].rect.x = (int)new_x;
      groupe.poissons[i].rect.y = (int)new_y;
    }
    free(Wx);
    free(Wy);
  }
}


/* Fonction principale */
int main(int argc, char *argv[]){
  srand(time(NULL));
  game_is_running = initialize_window();

  rectangle_list_t groupe = init_list_rectangle_t(FISH_NUMBER);
  render(groupe, blanc);
  int timer = 1;
  double counter = 0;
  double total_order_1 = 0;

  /* Gestion des résultats */
  FILE* fichier_donnees = fopen("res.csv", "w");
  fprintf(fichier_donnees, "Pas_de_Temps, Ordre_de_Polarisation\n");

  /* SDL_Delay(300); */
  /* update(groupe, DELTA_TIME); */
  /* render(groupe, bleu); */
  /* SDL_Delay(5000); */
  /* printf("Ordre de Vicsek : %f\n", calcul_order_1(groupe)); */

  while (game_is_running){
    if (timer > 100){
      total_order_1 += calcul_order_1(groupe);
      counter += 1;
    }
    if (timer == 400){
      printf("Ordre de Vicsek moyen : %f\n", total_order_1/counter);
      break;
    }
    /* if (calcul_order_1(groupe) > 0.8){ */
    /*   printf("Nombre d'itération : %d\n", timer); */
    /*   break; */
    /* } */

    fprintf(fichier_donnees, "%d, %f\n", timer, calcul_order_1(groupe));

    timer += 1;
    update(groupe, DELTA_TIME);
    render(groupe, bleu);
    SDL_Delay(30);
    process_input();
  }
  fclose(fichier_donnees);
  free(groupe.poissons);
  destroy_window();

  return 0;
}
