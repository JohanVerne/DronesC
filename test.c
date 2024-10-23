#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>  // Bibliothèque pour charger des images
#include <time.h>

#define MAX_X_0 770  // Limite en X (largeur de la zone)
#define MAX_Y_0 570  // Limite en Y (hauteur de la zone)
#define MAX_X_1 780  // Limite en X (largeur de la zone)
#define MAX_Y_1 580  // Limite en Y (hauteur de la zone)
#define MAX_X_2 790  // Limite en X (largeur de la zone)
#define MAX_Y_2 590  // Limite en Y (hauteur de la zone)


// Structure représentant un drone
typedef struct {
    int id;           // Identifiant unique du drone
    float x, y, z;    // Position du drone (XYZ)
    float vitesse;    // Vitesse de déplacement du drone
    float portee_com; // Portée de communication du drone
    int actif;        // Indique si le drone est actif ou détruit
    float prev_x, prev_y;
    float taille;  // Position précédente du drone pour laisser le tracé
    SDL_Texture *texture;
} Drone;

SDL_Texture* charger_image_drone(const char *fichier_image, SDL_Renderer *renderer) {

    fichier_image = "fichier_image.png";
    // Charger l'image avec SDL_image
    SDL_Surface *surface = IMG_Load(fichier_image);
    if (!surface) {
        printf("Erreur lors du chargement de l'image %s: %s\n", fichier_image, IMG_GetError());
        return NULL;
    }

    // Convertir la surface en texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);  // Libérer la surface après la conversion en texture
    return texture;
}

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
void deplacer_drones(Drone *drones, int nb_drones, float *F) {

    for (int i = 0; i < nb_drones; i++) {
                drones[i].prev_x = drones[i].x;
                drones[i].prev_y = drones[i].y;

                float dx = F[2*i] * drones[i].vitesse;
                float dy = F[2*i + 1] * drones[i].vitesse;

                drones[i].x += dx;
                drones[i].y += dy;

                if (drones[i].x < 0) drones[i].x = 0;
                if (drones[i].x > MAX_X_0) drones[i].x = MAX_X_0;

                if (drones[i].y < 0) drones[i].y = 0;
                if (drones[i].y > MAX_Y_0) drones[i].y = MAX_Y_0;
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

void apply_blur(SDL_Surface *surface, float blur) {
    // Obtenir les dimensions de l'image
    int width = surface->w;
    int height = surface->h;

    // Verrouiller la surface pour l'accès direct aux pixels
    SDL_LockSurface(surface);

    // Pointeur vers les pixels de la surface
    Uint32 *pixels = (Uint32 *)surface->pixels;

    // Créer un tableau temporaire pour stocker les nouveaux pixels
    Uint32 *new_pixels = malloc(width * height * sizeof(Uint32));

    // Parcourir chaque pixel de l'image
    for (int y = 1; y < height -1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int r_sum = 0, g_sum = 0, b_sum = 0;

            // Moyenne des couleurs des pixels voisins (3x3 box blur)
            for (int dy = -blur; dy <= blur; dy++) {
                for (int dx = -blur; dx <= blur; dx++) {
                    Uint32 pixel = pixels[(y + dy) * width + (x + dx)];

                    // Extraire les composants rouge, vert, bleu du pixel
                    Uint8 r, g, b;
                    SDL_GetRGB(pixel, surface->format, &r, &g, &b);

                    // Ajouter les composants au total
                    r_sum += r;
                    g_sum += g;
                    b_sum += b;
                }
            }

            // Calculer la moyenne des couleurs
            Uint8 r_avg = r_sum / 9;
            Uint8 g_avg = g_sum / 9;
            Uint8 b_avg = b_sum / 9;

            // Réassembler la couleur et la stocker dans le tableau temporaire
            new_pixels[y * width + x] = SDL_MapRGB(surface->format, r_avg, g_avg, b_avg);
        }
    }

    // Copier les nouveaux pixels dans la surface
    memcpy(pixels, new_pixels, width * height * sizeof(Uint32));

    // Libérer le tableau temporaire
    free(new_pixels);

    // Déverrouiller la surface
    SDL_UnlockSurface(surface);
}

void dessiner_drones(Drone *drones, int nb_drones, SDL_Renderer *renderer) {
    for (int i = 0; i < nb_drones; i++) {
        if (drones[i].actif && drones[i].texture) {
            // Définir la zone où dessiner l'image du drone
            SDL_Rect destination;
            destination.x = (int)drones[i].x;
            destination.y = (int)drones[i].y;
            float w = 20.0*(((int)drones[i].taille)/20.0);
            float h = 20.0*(((int)drones[i].taille)/20.0);
            destination.w = w;
            destination.h = h;
            
            // Dessiner la texture du drone
            SDL_RenderCopy(renderer, drones[i].texture, NULL, &destination);
        }
    }
}

float generate_random_float() {
    return 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
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
    init_drone(&drones[0], 1, 20.0, 20.0, 5.0, 1.5, 30.0, 30);
    init_drone(&drones[1], 2, 750.0, 20.0, 5.0, 1.2, 25.0, 20);
    init_drone(&drones[2], 3, 30.0, 550.0, 5.0, 1.8, 35.0, 10);

    drones[0].texture = charger_image_drone("drone1.jpg", renderer);
    drones[1].texture = charger_image_drone("drone2.jpg", renderer);
    drones[2].texture = charger_image_drone("drone3.jpg", renderer);

    int running = 1;
    SDL_Event event;

    Uint32 start_time = SDL_GetTicks();

    srand(time(NULL));

    float *F = (float *)malloc((nb_drones*2) * sizeof(float));

    for (int i = 0; i < nb_drones; i++) {
        if (drones[i].x < 100) {
            F[2*i] = 1; 

        } else {
            F[2*i] = -1;
        }

        if (drones[i].y < 100) {
            F[2*i + 1] = 1; 

        } else {
            F[2*i + 1] = -1;
        }

    }

    while (running) {

        // Gérer les événements (comme la fermeture de la fenêtre)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        Uint32 current_time = SDL_GetTicks();

        // Vérifier si 5 secondes se sont écoulées
        if (current_time - start_time >= 2000 && current_time - start_time <= 2050) {

            for (int i = 0; i < nb_drones*2; i++) {
                F[i] = generate_random_float();
            }

            start_time = SDL_GetTicks();
        }

    
        deplacer_drones(drones, nb_drones, F);


        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);  


        // Révéler la carte progressivement avec les drones
        for (int i = 0; i < nb_drones; i++) {
            reveal_map(renderer, map_texture, &drones[i]);
        }

        dessiner_drones(drones, nb_drones, renderer);

        // Mettre à jour l'affichage
        SDL_RenderPresent(renderer);

        // Attendre 16 millisecondes (~60 FPS)
        SDL_Delay(16);
    }

    for (int i = 0; i < 3; i++) {
        if (drones[i].texture) {
            SDL_DestroyTexture(drones[i].texture);
        }
    }

    // Nettoyer les ressources
    SDL_DestroyTexture(map_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
