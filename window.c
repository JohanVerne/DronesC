#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "fonctionsEtStructures.h"

void deplacer_drones(int nb_drones, Drone drones[nb_drones], Zone zone)
{
    for (int i = 0; i < nb_drones; i++)
    {
        // (exemple) Générer des déplacements aléatoires en x et y
        float dx = (float)(rand() % 11 - 5); // Déplacement entre -5 et 5
        float dy = (float)(rand() % 11 - 5); // Déplacement entre -5 et 5

        // Générer un changement d'altitude aléatoire
        float dz = (float)(rand() % 3 - 1); // Changement d'altitude entre -1 et 1

        // Utiliser la fonction deplacer_drone existante
        deplacer_drone(&drones[i], &zone, dx, dy, dz);
    }
}

// Dessiner le drone et effacer le masque noir à sa nouvelle position
void reveal_map(SDL_Renderer *renderer, SDL_Texture *map_texture, Drone *drone)
{
    // Calculer la taille de la zone à révéler
    float base_size = drone->dimensions[0] + drone->dimensions[1];   // Taille de base proportionnelle au drone
    float altitude_factor = drone->z * drone->z;                     // Facteur d'altitude au carré
    int reveal_size = (int)(base_size * (1 + altitude_factor / 50)); // Ajuster le facteur selon les besoins

    // Calculer les coordonnées du carré centré sur le drone
    SDL_Rect src_rect = {
        (int)drone->x - reveal_size / 2,
        (int)drone->y - reveal_size / 2,
        reveal_size,
        reveal_size};
    // S'assurer que la zone révélée reste dans les limites de la fenêtre
    if (src_rect.x < 0)
        src_rect.x = 0;
    if (src_rect.y < 0)
        src_rect.y = 0;
    if (src_rect.x + src_rect.w > 800)
        src_rect.w = 800 - src_rect.x; // Supposant une largeur de fenêtre de 800
    if (src_rect.y + src_rect.h > 600)
        src_rect.h = 600 - src_rect.y; // Supposant une hauteur de fenêtre de 600
    SDL_RenderCopy(renderer, map_texture, &src_rect, &src_rect);

    // Dessiner le drone à sa nouvelle position
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);                                                                                                         // Couleur rouge pour les drones
    SDL_Rect rect = {(int)drone->x - drone->z / 2, (int)drone->y - drone->z / 2, (int)drone->dimensions[0] + drone->z, (int)drone->dimensions[1] + drone->z}; // Taille du drone
    SDL_RenderFillRect(renderer, &rect);
}

// Simulation de la gestion des drones
int main()
{
    // Initialiser SDL et SDL_image
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG); // Pour charger une image PNG

    SDL_Window *window = SDL_CreateWindow("Surveillance par Drones",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Charger l'image de la carte
    SDL_Surface *image_surface = IMG_Load("carte.png"); // Charger l'image depuis un fichier
    if (!image_surface)
    {
        printf("Erreur de chargement de l'image: %s\n", IMG_GetError());
        return 1;
    }

    // Convertir l'image en texture pour l'afficher
    SDL_Texture *map_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface); // Libérer la surface maintenant que nous avons la texture

    // Initialisation de la zone et des drones
    Zone zone;
    definir_zone(&zone, 0.0, 0.0, 800.0, 600.0);

    // Création de trois drones pour la démonstration
    int nb_drones = 3;
    Drone drones[nb_drones];
    int dims[3] = {5, 5, 5};

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 250.0, 120.0, 5.0, 1.5, 30.0, dims);
    init_drone(&drones[1], 2, 30.0, 20.0, 5.0, 1.2, 25.0, dims);
    init_drone(&drones[2], 3, 530.0, 50.0, 5.0, 1.8, 35.0, dims);

    int running = 1;
    SDL_Event event;

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

        // Afficher le masque noir (initialement couvrant la carte)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour le masque
        SDL_RenderClear(renderer);                      // Remplir toute la fenêtre de noir

        // Révéler la carte progressivement avec les drones
        for (int i = 0; i < nb_drones; i++)
        {
            reveal_map(renderer, map_texture, &drones[i]);
        }

        // Mettre à jour l'affichage
        SDL_RenderPresent(renderer);

        // Attendre 16 millisecondes (~60 FPS)
        SDL_Delay(16);
    }

    // Nettoyer les ressources
    SDL_DestroyTexture(map_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}