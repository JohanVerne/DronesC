#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "fonctionsEtStructures.h"

// Fonction pour dessiner un drone
void draw_drone(SDL_Renderer *renderer, Drone *drone)
{
    // Dessiner le drone
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);       // Couleur rouge pour les drones
    SDL_Rect rect = {(int)drone->x, (int)drone->y, 15, 15}; // Taille du drone
    SDL_RenderFillRect(renderer, &rect);
}

void deplacer_drones(int nb_drones, Drone drones[nb_drones], Zone zone)
{
    for (int i = 0; i < nb_drones; i++)
    {
        deplacer_drone(&drones[i], &zone, 0.01, 0.01, 0); // Déplacement des drones
    }
}

// Simulation de la gestion des drones
int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Erreur lors de l'initialisation de SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Surveillance par Drones",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_SHOWN);

    if (!window)
    {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        printf("Erreur lors de la création du renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *image_surface = IMG_Load("carte.png"); // Charger l'image depuis un fichier
    if (!image_surface)
    {
        printf("Erreur de chargement de l'image: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Texture *background_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface); // Libérer la surface maintenant que nous avons la texture

    // Initialisation de la zone et des drones
    Zone zone;
    definir_zone(&zone, 0.0, 0.0, 100.0, 100.0);

    // Création de trois drones pour la démonstration
    int nb_drones = 3;
    Drone drones[nb_drones];
    int dims[3] = {5, 5, 5};

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 750.0, 20.0, 5.0, 1.5, 30.0, dims);
    init_drone(&drones[1], 2, 30.0, 20.0, 5.0, 1.2, 25.0, dims);
    init_drone(&drones[2], 3, 30.0, 550.0, 5.0, 1.8, 35.0, dims);

    int running = 1;
    SDL_Event event;

    // Effacer l'écran une fois au début pour le fond
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour l'arrière-plan
    SDL_RenderClear(renderer);

    // Boucle principale
    while (running)
    {
        // Gérer les événements (comme la fermeture de la fenêtre)
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        // Déplacer les drones à chaque cycle
        deplacer_drones(nb_drones, drones, zone);

        // Dessiner l'image de fond (carte)
        SDL_RenderCopy(renderer, background_texture, NULL, NULL); // Afficher la texture sur toute la fenêtre

        // Dessiner les drones
        for (int i = 0; i < nb_drones; i++)
        {
            draw_drone(renderer, &drones[i]);
        }

        // Mettre à jour l'affichage
        SDL_RenderPresent(renderer);

        // Attendre 16 millisecondes (~60 FPS)
        SDL_Delay(16);
    }

    SDL_DestroyTexture(background_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}