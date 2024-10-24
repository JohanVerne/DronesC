#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>  
#include <time.h>


// Structure représentant un drone
typedef struct {
    int id;           // Identifiant unique du drone
    float x, y, z;    // Position du drone (XYZ)
    float vitesse;    // Vitesse de déplacement du drone
    int actif;        // Indique si le drone est actif ou détruit
    float prev_x, prev_y; // Position précédente du drone pour laisser le tracé
    float taille;     // Taille du drone
    SDL_Texture *texture;
} Drone;


// Structure représentant un obstacle
typedef struct {
    float x, y;       // Position centrale de l'obstacle
    float rayon;     // Rayon de l'obstacle (supposé circulaire, peut être une taille si rectangulaire)
    SDL_Texture *texture; // Texture de l'image de l'obstacle
} Obstacle;


SDL_Texture* charger_image_drone(const char *drone, SDL_Renderer *renderer) {
    drone = "drone.png";

    // Charger l'image avec SDL_image
    SDL_Surface *surface = IMG_Load(drone);

    // Convertir la surface en texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);  // Libérer la surface après la conversion en texture
    return texture;
}


// Fonction pour charger une image PNG pour un obstacle
SDL_Texture* charger_image_obstacle(const char *fichier_image, SDL_Renderer *renderer) {
    SDL_Surface *surface = IMG_Load(fichier_image);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);  // Libérer la surface maintenant que la texture est créée
    return texture;
}


// Initialisation d'un drone
void init_drone(Drone *drone, int id, float x, float y, float z, float vitesse, float taille) {
    drone->id = id;
    drone->x = x;
    drone->y = y;
    drone->z = z;
    drone->vitesse = vitesse;
    drone->taille = taille;
    drone->actif = 1; // Drone actif par défaut
    drone->prev_x = x; // Initialiser la position précédente
    drone->prev_y = y; // Initialiser la position précédente
    printf("Drone %d initialisé à la position (%.2f, %.2f, %.2f) avec vitesse %.2f\n", id, x, y, z, vitesse);
}

// Déplacement des drones (mise à jour des positions)
void deplacer_drones(Drone *drones, int nb_drones, float *F) {

    for (int i = 0; i < nb_drones; i++) {
                drones[i].prev_x = drones[i].x; // Sauvegarder la position antérieure
                drones[i].prev_y = drones[i].y;

                float dx = F[2*i] * drones[i].vitesse; 
                float dy = F[2*i + 1] * drones[i].vitesse;

                drones[i].x += dx; // Remplacer la position
                drones[i].y += dy;

                if (drones[i].x < 0) drones[i].x = 0; // Ne pas dépasser les limites du cadre pour l'abscisse du drone
                if (drones[i].x > 770) drones[i].x = 770;

                if (drones[i].y < 0) drones[i].y = 0; // Ne pas dépasser les limites du cadre pour l'ordonée du drone
                if (drones[i].y > 570) drones[i].y = 570;
    }
    
}

// Dessiner le drone et effacer le masque noir à sa nouvelle position
void reveal_map(SDL_Renderer *renderer, SDL_Texture *map_texture, Drone *drone) {
    // Afficher la carte à l'endroit où le drone passe (effacer le masque noir)
    SDL_Rect src_rect = { (int)drone->x -5*((int)drone->taille/10), (int)drone->y -5*((int)drone->taille/10), ((int)drone->taille)*2, ((int)drone->taille)*2 }; // Zone à dévoiler
    SDL_RenderCopy(renderer, map_texture, &src_rect, &src_rect);

}

// Fonction pour appliquer un flou à une image (surface SDL)
void apply_blur(SDL_Surface *surface, float blur_factor) {
    // Vérifier si la surface est valide
    if (!surface) return;

    // Calcul de la taille de la matrice de flou en fonction du facteur
    int radius = (int)blur_factor;

    // Créer une copie de la surface pour lire les pixels d'origine
    SDL_Surface *temp_surface = SDL_ConvertSurface(surface, surface->format, 0);
    if (!temp_surface) return;

    // Parcourir chaque pixel de l'image
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            float r_sum = 0, g_sum = 0, b_sum = 0;
            int count = 0;

            // Parcourir les pixels voisins 
            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;

                    // Vérifier que les coordonnées sont dans les limites de l'image
                    if (nx >= 0 && nx < surface->w && ny >= 0 && ny < surface->h) {
                        // Obtenir la couleur du pixel voisin depuis l'image d'origine
                        Uint32 pixel = ((Uint32 *)temp_surface->pixels)[ny * temp_surface->w + nx];
                        Uint8 r, g, b;
                        SDL_GetRGB(pixel, temp_surface->format, &r, &g, &b);

                        // Additionner les valeurs des composantes RVB
                        r_sum += r;
                        g_sum += g;
                        b_sum += b;
                        count++;
                    }
                }
            }

            // Calculer la moyenne des couleurs
            Uint8 r = (Uint8)(r_sum / count);
            Uint8 g = (Uint8)(g_sum / count);
            Uint8 b = (Uint8)(b_sum / count);

            // Mettre à jour la couleur du pixel dans la surface d'origine
            Uint32 new_pixel = SDL_MapRGB(surface->format, r, g, b);
            ((Uint32 *)surface->pixels)[y * surface->w + x] = new_pixel;
        }
    }

    // Libérer la surface temporaire
    SDL_FreeSurface(temp_surface);
}

void dessiner_drones(Drone *drones, int nb_drones, SDL_Renderer *renderer) {
    for (int i = 0; i < nb_drones; i++) {
        if (drones[i].actif && drones[i].texture) {
            
            // Définir la zone où dessiner le drone
            SDL_Rect destination;
            destination.x = (int)drones[i].x;
            destination.y = (int)drones[i].y;
            float w = ((int)drones[i].taille);
            float h = ((int)drones[i].taille);
            destination.w = w;
            destination.h = h;

            // Dessiner l'image du drone
            SDL_RenderCopy(renderer, drones[i].texture, NULL, &destination);
        }
    }
}

// Fonction pour dessiner les obstacles avec leurs textures
void dessiner_obstacles(Obstacle *obstacles, int nb_obstacles, SDL_Renderer *renderer) {
    for (int i = 0; i < nb_obstacles; i++) {
        if (obstacles[i].texture) {
            // Définir le rectangle de destination pour dessiner l'obstacle
            SDL_Rect destination;
            destination.x = (int)(obstacles[i].x - obstacles[i].rayon);  // Centrer l'image de l'obstacle
            destination.y = (int)(obstacles[i].y - obstacles[i].rayon);
            destination.w = (int)(2 * obstacles[i].rayon);  // Largeur = diamètre de l'obstacle
            destination.h = (int)(2 * obstacles[i].rayon);  // Hauteur = diamètre de l'obstacle

            // Dessiner la texture de l'obstacle
            SDL_RenderCopy(renderer, obstacles[i].texture, NULL, &destination);
        }
    }
}


// Fonction pour vérifier si deux drones sont en collision
int verifier_collision(Drone *drone1, Drone *drone2) {
    // Calcul de la distance entre les deux drones
    float distance = sqrt(pow(drone1->x - drone2->x, 2) + pow(drone1->y - drone2->y, 2));
    
    // Rayon qui correspond à la taille des drones
    float rayon1 = (drone1->taille);  
    float rayon2 = (drone2->taille);  

    // Vérifier si la distance entre les deux drones est inférieure à la somme de leurs rayons
    if (distance < (rayon1 + rayon2)) {
        return 1;  // Collision détectée
    }

    return 0;  // Pas de collision
}

// Fonction pour gérer les collisions entre tous les drones
void gerer_collisions(Drone *drones, int nb_drones) {
    // Parcourir chaque paire de drones
    for (int i = 0; i < nb_drones; i++) {
        for (int j = i + 1; j < nb_drones; j++) {
            if (verifier_collision(&drones[i], &drones[j])) {
                // Si une collision est détectée, inverser la direction
                drones[i].vitesse = -drones[i].vitesse;
                drones[j].vitesse = -drones[j].vitesse;
                printf("Collision détectée entre drone %d et drone %d\n", drones[i].id, drones[j].id);
            }
        }
    }
}

// Fonction pour vérifier si un drone est proche des murs et ajuster sa direction
void gerer_murs(Drone *drone, int largeur_zone, int hauteur_zone) {

    // Vérifier si le drone est proche du mur gauche ou droit
    if (drone->x == 0) {
        drone->vitesse = -(drone->vitesse);  // Diriger vers la droite
    } else if (drone->x >= (largeur_zone)) {
        drone->vitesse = -(drone->vitesse);  // Diriger vers la gauche
    }

    // Vérifier si le drone est proche du mur supérieur ou inférieur
    if (drone->y == 0) {
        drone->vitesse = -(drone->vitesse);  // Diriger vers le bas
    } else if (drone->y >= (hauteur_zone)) {
        drone->vitesse = -(drone->vitesse);  // Diriger vers le haut
    }
}

// Fonction pour gérer tous les drones et vérifier leur proximité avec les murs
void gerer_murs_pour_tous_les_drones(Drone *drones, int nb_drones, int largeur_zone, int hauteur_zone) {
    for (int i = 0; i < nb_drones; i++) {
        gerer_murs(&drones[i], largeur_zone, hauteur_zone);
    }
}

// Fonction pour vérifier si un drone est proche d'un obstacle
int verifier_collision_obstacle(Drone *drone, Obstacle *obstacle) {
    // Calculer la distance entre le drone et l'obstacle
    float distance = sqrt(pow(drone->x - obstacle->x, 2) + pow(drone->y - obstacle->y, 2));
    
    // Vérifier si la distance est inférieure à la somme du rayon de l'obstacle et du drone 
    float rayon_drone = (drone->taille*2.0); 
    if (distance < (obstacle->rayon + rayon_drone)) {
        return 1;  // Collision détectée
    }

    return 0;  // Pas de collision
}

// Fonction pour faire éviter les obstacles à tous les drones
void esquiver_obstacles(Drone *drones, int nb_drones, Obstacle *obstacles, int nb_obstacles) {
    for (int i = 0; i < nb_drones; i++) {
        for (int j = 0; j < nb_obstacles; j++) {
            if (verifier_collision_obstacle(&drones[i], &obstacles[j])) {

                // Inverser la direction de déplacement
                drones[i].vitesse = -drones[i].vitesse;
                printf("Drone %d a évité un obstacle en position (%.2f, %.2f)\n", drones[i].id, drones[i].x, drones[i].y);
            }
        }
    }
}

// Fonction pour générer un réel aléatoire entre -1 et 1 
float generate_random_float() {
    return 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
}

int main() {

    // Initialiser SDL et SDL_image
    SDL_Init(SDL_INIT_VIDEO); // Initialise le sous-système vidéo de la bibliothèque SDL
    IMG_Init(IMG_INIT_PNG);  // Pour charger une image PNG


    SDL_Window *window = SDL_CreateWindow("Surveillance par Drones",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_SHOWN); // Crée une fenêtre SDL de 800x600 pixels, centrée sur l'écran

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    // Création de quatre drones pour la démonstration
    int nb_drones = 4;
    Drone drones[nb_drones];

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 20.0, 20.0, 5.0, 2.0, 10);
    init_drone(&drones[1], 2, 750.0, 20.0, 5.0, 1.7, 20);
    init_drone(&drones[2], 3, 30.0, 550.0, 5.0, 1.3, 30);
    init_drone(&drones[3], 3, 750.0, 550.0, 5.0, 1.0, 40);


    int nb_obstacles = 2;
    Obstacle obstacles[2] = {
        {200, 300, 50},  // Un obstacle de rayon 50 en position (200, 300)
        {500, 400, 30}   // Un obstacle de rayon 30 en position (500, 400)
    };


    // Charger les 4 images de la carte qui correspondent aux différents niveaux de flou

    SDL_Surface *image_surface = IMG_Load("carte.png");  // Charger l'image depuis un fichier
    
    SDL_Surface *image_surface2 = IMG_Load("carte.png"); 

    SDL_Surface *image_surface3 = IMG_Load("carte.png");  

    SDL_Surface *image_surface4 = IMG_Load("carte.png");  


    // Redimensionner les 4 images
    SDL_Surface *scaled_surface = SDL_CreateRGBSurface(0,
                                                       SDL_GetWindowSurface(window)->w,
                                                       SDL_GetWindowSurface(window)->h,
                                                       32, 0, 0, 0, 0);
    SDL_BlitScaled(image_surface, NULL, scaled_surface, NULL);


    SDL_Surface *scaled_surface2 = SDL_CreateRGBSurface(0,
                                                       SDL_GetWindowSurface(window)->w,
                                                       SDL_GetWindowSurface(window)->h,
                                                       32, 0, 0, 0, 0);
    SDL_BlitScaled(image_surface2, NULL, scaled_surface2, NULL);


    SDL_Surface *scaled_surface3 = SDL_CreateRGBSurface(0,
                                                       SDL_GetWindowSurface(window)->w,
                                                       SDL_GetWindowSurface(window)->h,
                                                       32, 0, 0, 0, 0);
    SDL_BlitScaled(image_surface3, NULL, scaled_surface3, NULL);


    SDL_Surface *scaled_surface4 = SDL_CreateRGBSurface(0,
                                                       SDL_GetWindowSurface(window)->w,
                                                       SDL_GetWindowSurface(window)->h,
                                                       32, 0, 0, 0, 0);
    SDL_BlitScaled(image_surface4, NULL, scaled_surface4, NULL);



    // Applique un flou différent aux 4 images
    apply_blur(scaled_surface, 0.5);

    apply_blur(scaled_surface2, 1.0);

    apply_blur(scaled_surface3, 1.5);

    apply_blur(scaled_surface4, 2.0);
    

    // Convertir l'image en texture pour l'afficher
    SDL_Texture *map_texture = SDL_CreateTextureFromSurface(renderer, scaled_surface);
    SDL_FreeSurface(scaled_surface); // Libérer la surface redimensionnée

  
    SDL_Texture *map_texture2 = SDL_CreateTextureFromSurface(renderer, scaled_surface2);
    SDL_FreeSurface(scaled_surface2); 

 
    SDL_Texture *map_texture3 = SDL_CreateTextureFromSurface(renderer, scaled_surface3);
    SDL_FreeSurface(scaled_surface3); 

        
    SDL_Texture *map_texture4 = SDL_CreateTextureFromSurface(renderer, scaled_surface4);
    SDL_FreeSurface(scaled_surface4); 



    // Charge une image PNG pour chaque drone
    for (int i = 0; i < nb_drones; i++) {
        drones[i].texture = charger_image_drone("drone.png", renderer);
    }

    for (int i = 0; i < nb_obstacles; i++) {
        obstacles[i].texture = charger_image_obstacle("obstacle.png", renderer);
    }


    int running = 1;
    SDL_Event event;

    Uint32 start_time = SDL_GetTicks(); // Sauvegarde le temps

    // Initialise la graine pour la génération de nombres aléatoires
    srand(time(NULL));

    // Alloue un tableau dynamique pour les directions X et Y de chaque drone
    float *F = (float *)malloc((nb_drones*2) * sizeof(float));

    for (int i = 0; i < nb_drones; i++) {
        if (drones[i].x < 100) { 
            F[2*i] = 1; // Mouvement positif en X si le drone est proche du bord gauche initialement

        } else {
            F[2*i] = -1; // Mouvement négatif en X sinon
        }

        if (drones[i].y < 100) {
            F[2*i + 1] = 1; // Mouvement positif en Y si le drone est proche du bord supérieur

        } else {
            F[2*i + 1] = -1; // Mouvement négatif en Y sinon
        }

    }

    while (running) {

        // Gérer la fermeture de la fenêtre)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        Uint32 current_time = SDL_GetTicks();

        // Vérifier si 3 secondes se sont écoulées
        if (current_time - start_time >= 2000 && current_time - start_time <= 2100) {

            // Générer des nombres aléatoires pour chaque direction (en X et Y) des drones 
            for (int i = 0; i < nb_drones*2; i++) {
                F[i] = generate_random_float();
            }

            start_time = SDL_GetTicks();
        }

         // Vérifier les collisions avec les murs pour chaque drone
        gerer_murs_pour_tous_les_drones(drones, nb_drones, 770, 570);


        // Vérification des collisions entre les drones
        gerer_collisions(drones, nb_drones);


        // Vérifier les collisions entre les drones et les obstacles
        esquiver_obstacles(drones, nb_drones, obstacles, nb_obstacles);


        // Déplacer les drones
        deplacer_drones(drones, nb_drones, F);

        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


        // Révéler la carte progressivement avec les drones selon leur résolution
        reveal_map(renderer, map_texture, &drones[0]);
        reveal_map(renderer, map_texture2, &drones[1]);
        reveal_map(renderer, map_texture3, &drones[2]);
        reveal_map(renderer, map_texture4, &drones[3]);


        // Dessiner les drones
        dessiner_drones(drones, nb_drones, renderer);

        // Dessiner les obstacles
        dessiner_obstacles(obstacles, nb_obstacles, renderer);
    
        
        // Mettre à jour l'affichage
        SDL_RenderPresent(renderer);
        
        

        // Attendre 16 millisecondes (~60 FPS)
        SDL_Delay(16);
    }

    // Nettoyer les ressources

    for (int i = 0; i < nb_obstacles; i++) {
        if (obstacles[i].texture) {
            SDL_DestroyTexture(obstacles[i].texture);
        }
    }

    for (int i = 0; i < nb_drones; i++) {
        if (drones[i].texture) {
            SDL_DestroyTexture(drones[i].texture);
        }

    
    }

    SDL_DestroyTexture(map_texture);
    SDL_DestroyTexture(map_texture2);
    SDL_DestroyTexture(map_texture3);
    SDL_DestroyTexture(map_texture4);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
