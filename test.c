#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>  
#include <SDL2/SDL_image.h>

// Structure représentant un drone
typedef struct
{
    int id;           // Identifiant unique du drone
    float x, y, z;    // Position actuelle du drone (XYZ)
    float x_prev, y_prev, z_prev; // Position précédente du drone
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
    drone->x_prev = x; // Initialement, la position précédente est la même que la position actuelle
    drone->y_prev = y;
    drone->z_prev = z;
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
void deplacer_drones(Drone *drones, int nb_drones)
{
    for (int i = 0; i < nb_drones; i++)
    {
        if (drones[i].actif)
        {
            // Stocker la position précédente
            drones[i].x_prev = drones[i].x;
            drones[i].y_prev = drones[i].y;
            drones[i].z_prev = drones[i].z;

            // Simuler un déplacement simple pour chaque drone
            drones[i].x += drones[i].vitesse; // Déplacement à droite en fonction de la vitesse
            drones[i].y += drones[i].vitesse; // Déplacement vers le bas en fonction de la vitesse

            // Vérifier que les drones ne sortent pas de la fenêtre (800x600)
            if (drones[i].x > 790) drones[i].x = 10; // Réinitialiser la position X si trop loin
            if (drones[i].y > 590) drones[i].y = 10; // Réinitialiser la position Y si trop loin

            printf("Drone %d déplacé à la position (%.2f, %.2f, %.2f)\n", drones[i].id, drones[i].x, drones[i].y, drones[i].z);
        }
        else
        {
            printf("Drone %d est inactif\n", drones[i].id);
        }
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

// Fonction pour dessiner un drone
void draw_drone(SDL_Renderer *renderer, Drone *drone)
{
    // Dessiner le drone
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Couleur rouge pour les drones
    SDL_Rect rect = { (int)drone->x, (int)drone->y, 15, 15 }; // Taille du drone
    SDL_RenderFillRect(renderer, &rect);
}

// Fonction pour dessiner le tracé du drone
void draw_trail(SDL_Renderer *renderer, Drone *drone)
{
    // Dessiner une ligne entre l'ancienne position et la nouvelle position
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Couleur marron pour le tracé
    SDL_RenderDrawLine(renderer, (int)drone->x_prev + 5, (int)drone->y_prev + 5, (int)drone->x + 5, (int)drone->y + 5);
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

    SDL_Surface *image_surface = IMG_Load("carte.png");  // Charger l'image depuis un fichier
    if (!image_surface) {
        printf("Erreur de chargement de l'image: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Texture *background_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface);  // Libérer la surface maintenant que nous avons la texture

    // Création de trois drones pour la démonstration
    int nb_drones = 3;
    Drone drones[nb_drones];

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 750.0, 20.0, 5.0, 1.5, 30.0);
    init_drone(&drones[1], 2, 30.0, 20.0, 5.0, 1.2, 25.0);
    init_drone(&drones[2], 3, 30.0, 550.0, 5.0, 1.8, 35.0);

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
        deplacer_drones(drones, nb_drones);

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
