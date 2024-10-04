#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>  


// Structure représentant un drone
typedef struct
{
    int id;           // Identifiant unique du drone
    float x, y, z;    // Position du drone (XYZ)
    float vitesse;    // Vitesse de déplacement du drone
    float portee_com; // Portée de communication du drone
    int actif;        // Indique si le drone est actif ou détruit
} Drone;

// Structure représentant la zone à surveiller
typedef struct
{
    float xMin, yMin, xMax, yMax; // Coordonnées de la zone
} Zone;

// Initialisation d'un drone
void init_drone(Drone *drone, int id, float x, float y, float z, float vitesse, float portee_com)
{
    drone->id = id;
    drone->x = x;
    drone->y = y;
    drone->z = z;
    drone->vitesse = vitesse;
    drone->portee_com = portee_com;
    drone->actif = 1; // Drone actif par défaut
    printf("Drone %d initialisé à la position (%.2f, %.2f, %.2f) avec vitesse %.2f et portée %.2f\n", id, x, y, z, vitesse, portee_com);
}

// Définition de la zone à surveiller
void definir_zone(Zone *zone, float xMin, float yMin, float xMax, float yMax)
{
    zone->xMin = xMin;
    zone->yMin = yMin;
    zone->xMax = xMax;
    zone->yMax = yMax;
    printf("Zone définie : de (%.2f, %.2f) à (%.2f, %.2f)\n", xMin, yMin, xMax, yMax);
}

// Déplacement des drones (mise à jour des positions)
void deplacer_drone(Drone *drone, float dx, float dy, float dz)
{
    if (drone->actif)
    {
        drone->x += dx * drone->vitesse;
        drone->y += dy * drone->vitesse;
        drone->z += dz * drone->vitesse;
        printf("Drone %d déplacé à la position (%.2f, %.2f, %.2f)\n", drone->id, drone->x, drone->y, drone->z);
    }
    else
    {
        printf("Drone %d est inactif\n", drone->id);
    }
}

// Vérifie si le drone1 est à portée de communication du drone2
int est_voisin(Drone *drone1, Drone *drone2)
{
    float distance = sqrt(pow(drone1->x - drone2->x, 2) + pow(drone1->y - drone2->y, 2) + pow(drone1->z - drone2->z, 2));
    return distance <= drone1->portee_com;
}

// Capture d'une image par un drone
void capturer_image(Drone *drone)
{
    if (drone->actif)
    {
        printf("Drone %d capture une image à la position (%.2f, %.2f, %.2f)\n", drone->id, drone->x, drone->y, drone->z);
        // TODO : Code pour générer une image (en fonction des specs)
    }
    else
    {
        printf("Drone %d est inactif et ne peut pas capturer d'image.\n", drone->id);
    }
}



// Gestion de la destruction d'un drone (désactivation)
void detruire_drone(Drone *drone)
{
    drone->actif = 0;
    printf("Drone %d a été détruit.\n", drone->id);
}

void draw_drone(SDL_Renderer *renderer, Drone *drone) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Couleur rouge pour les drones
    SDL_Rect rect = { (int)drone->x, (int)drone->y, 10, 10 }; // Taille du drone
    SDL_RenderFillRect(renderer, &rect);
}

// Simulation de la gestion des drones
int main(int argc, char **argv)
{
    // Initialisation de la zone et des drones
    Zone zone;
    definir_zone(&zone, 0.0, 0.0, 100.0, 100.0);

    int nb_drones = 3;
    Drone drones[nb_drones];

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 10.0, 20.0, 5.0, 1.5, 30.0);
    init_drone(&drones[1], 2, 50.0, 60.0, 5.0, 1.2, 25.0);
    init_drone(&drones[2], 3, 30.0, 40.0, 5.0, 1.8, 35.0);



    // Vérification des voisins
    if (est_voisin(&drones[0], &drones[1]))
    {
        printf("Drone %d et Drone %d sont voisins.\n", drones[0].id, drones[1].id);
    }
    else
    {
        printf("Drone %d et Drone %d ne sont pas à portée de communication.\n", drones[0].id, drones[1].id);
    }



    // Essai de capture d'images par tous les drones
    capturer_image(&drones[0]);
    capturer_image(&drones[1]); // Drone détruit
    capturer_image(&drones[2]);

    printf("\n");

    SDL_Window *window = NULL;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("ERREUR : Initialisation SDL > %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Première fenêtre", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);

    if(window = NULL)
    {
        SDL_Log("ERREUR : Creation > %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour l'arrière-plan
        SDL_RenderClear(renderer);

        // Dessiner les drones
        for (int i = 0; i < 3; i++) {
            draw_drone(renderer, &drones[i]);
        }

        // Mettre à jour l'affichage
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;

}
