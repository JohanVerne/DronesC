#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>  // Bibliothèque pour charger des images

// Structure représentant un drone
typedef struct {
    int id;           // Identifiant unique du drone
    float x, y, z;    // Position du drone (XYZ)
    float vitesse;    // Vitesse de déplacement du drone
    float portee_com; // Portée de communication du drone
    int actif;        // Indique si le drone est actif ou détruit
    float prev_x, prev_y;
    float taille;  // Position précédente du drone pour laisser le tracé
} Drone;

// Initialisation d'un drone
void init_drone(Drone *drone, int id, float x, float y, float z, float vitesse, float portee_com, float taille) {
    drone->id = id;
    drone->x = x;
    drone->y = y;
    drone->z = z;
    drone->vitesse = vitesse;
    drone->taille = taille;
    drone->portee_com = portee_com;
    drone->actif = 1; // Drone actif par défaut
    drone->prev_x = x; // Initialiser la position précédente
    drone->prev_y = y; // Initialiser la position précédente
    printf("Drone %d initialisé à la position (%.2f, %.2f, %.2f) avec vitesse %.2f et portée %.2f\n", id, x, y, z, vitesse, portee_com);
}

// Déplacement des drones (mise à jour des positions)
void deplacer_drones(Drone *drones, int nb_drones) {
    for (int i = 0; i < nb_drones; i++) {
        drones[i].prev_x = drones[i].x; // Enregistrer la position précédente
        drones[i].prev_y = drones[i].y;

        // Simuler un déplacement simple pour chaque drone
        drones[i].x += drones[i].vitesse; // Déplacement à droite en fonction de la vitesse
        drones[i].y += drones[i].vitesse; // Déplacement vers le bas en fonction de la vitesse

        // Vérifier que les drones ne sortent pas de la fenêtre (800x600)
        if (drones[i].x > 790) drones[i].x = 10; // Réinitialiser la position X si trop loin
        if (drones[i].y > 590) drones[i].y = 10; // Réinitialiser la position Y si trop loin
    }
}

// Dessiner le drone et effacer le masque noir à sa nouvelle position
void reveal_map(SDL_Renderer *renderer, SDL_Texture *map_texture, Drone *drone) {
    // Afficher la carte à l'endroit où le drone passe (effacer le masque noir)
    SDL_Rect src_rect = { (int)drone->x -5*((int)drone->taille/10), (int)drone->y -5*((int)drone->taille/10), ((int)drone->taille)*2, ((int)drone->taille)*2 }; // Zone à dévoiler
    SDL_RenderCopy(renderer, map_texture, &src_rect, &src_rect);

    // Dessiner le drone à sa nouvelle position
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Couleur rouge pour les drones
    SDL_Rect rect = { (int)drone->x, (int)drone->y, (int)drone->taille, (int)drone->taille }; // Taille du drone
    SDL_RenderFillRect(renderer, &rect);
}

int main() {

    // Initialiser SDL et SDL_image
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);  // Pour charger une image PNG

    SDL_Window *window = SDL_CreateWindow("Surveillance par Drones",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Charger l'image de la carte
    SDL_Surface *image_surface = IMG_Load("carte.png");  // Charger l'image depuis un fichier
    if (!image_surface) {
        printf("Erreur de chargement de l'image: %s\n", IMG_GetError());
        return 1;
    }

    // Convertir l'image en texture pour l'afficher
    SDL_Texture *map_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface);  // Libérer la surface maintenant que nous avons la texture

    // Création de trois drones pour la démonstration
    int nb_drones = 3;
    Drone drones[nb_drones];

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 750.0, 20.0, 5.0, 1.5, 30.0, 30);
    init_drone(&drones[1], 2, 30.0, 20.0, 5.0, 1.2, 25.0, 20);
    init_drone(&drones[2], 3, 30.0, 550.0, 5.0, 1.8, 35.0, 10);

    int running = 1;
    SDL_Event event;

    while (running) {
        // Gérer les événements (comme la fermeture de la fenêtre)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Déplacer les drones à chaque cycle
        deplacer_drones(drones, nb_drones);

        // Afficher le masque noir (initialement couvrant la carte)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour le masque
        SDL_RenderClear(renderer);  // Remplir toute la fenêtre de noir

        // Révéler la carte progressivement avec les drones
        for (int i = 0; i < nb_drones; i++) {
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
