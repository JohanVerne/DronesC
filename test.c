#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>  // Bibliothèque pour charger des images
#include <time.h>


// Structure représentant un drone
typedef struct {
    int id;           // Identifiant unique du drone
    float x, y, z;    // Position du drone (XYZ)
    float vitesse;    // Vitesse de déplacement du drone
    int actif;        // Indique si le drone est actif ou détruit
    float prev_x, prev_y; // Position précédente du drone pour laisser le tracé
    float taille; // Taille du drone
    SDL_Texture *texture;
} Drone;


SDL_Texture* charger_image_drone(const char *drone, SDL_Renderer *renderer) {
    drone = "drone.png";

    // Charger l'image avec SDL_image
    SDL_Surface *surface = IMG_Load(drone);

    if (!surface) {
        printf("Erreur lors du chargement de l'image %s: %s\n", drone, IMG_GetError());
        return NULL;
    }

    // Convertir la surface en texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);  // Libérer la surface après la conversion en texture
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

                if (drones[i].x < 0) drones[i].x = 0; // Ne pas dépasser les limites du cadre pour l'abscisse 
                if (drones[i].x > 770) drones[i].x = 770;

                if (drones[i].y < 0) drones[i].y = 0; // Ne pas dépasser les limites du cadre pour l'ordonée
                if (drones[i].y > 570) drones[i].y = 570;
    }
    
}

// Dessiner le drone et effacer le masque noir à sa nouvelle position
void reveal_map(SDL_Renderer *renderer, SDL_Texture *map_texture, Drone *drone) {
    // Afficher la carte à l'endroit où le drone passe (effacer le masque noir)
    SDL_Rect src_rect = { (int)drone->x -5*((int)drone->taille/10), (int)drone->y -5*((int)drone->taille/10), ((int)drone->taille)*2, ((int)drone->taille)*2 }; // Zone à dévoiler
    SDL_RenderCopy(renderer, map_texture, &src_rect, &src_rect);

}

// Appliquer le flou à l'image
void apply_blur(SDL_Surface *surface, float facteur) {
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

            // Moyenne des couleurs des pixels voisins (3x3) en prenant en compte le facteur de flou
            for (int dy = -facteur; dy <= facteur; dy++) {
                for (int dx = -facteur; dx <= facteur; dx++) {
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

// Générer un réel aléatoire entre -1 et 1 
float generate_random_float() {
    return 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
}

// Fonction pour vérifier si deux drones sont en collision
int verifier_collision(Drone *drone1, Drone *drone2) {
    // Calcul de la distance entre les deux drones
    float distance = sqrt(pow(drone1->x - drone2->x, 2) + pow(drone1->y - drone2->y, 2));
    
    // Rayon arbitraire pour chaque drone (à ajuster selon la taille réelle des drones)
    float rayon1 = 10.0;  // Par exemple, un rayon de 10 unités pour drone1
    float rayon2 = 10.0;  // Par exemple, un rayon de 10 unités pour drone2

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
                // Si une collision est détectée, inverser la direction des drones impliqués
                drones[i].vitesse = -drones[i].vitesse;
                drones[j].vitesse = -drones[j].vitesse;

                // Vous pouvez ajouter d'autres actions ici (par exemple, les éloigner l'un de l'autre)
                printf("Collision détectée entre drone %d et drone %d\n", drones[i].id, drones[j].id);
            }
        }
    }
}

// Fonction pour vérifier si un drone est proche des murs et ajuster sa direction
void gerer_murs(Drone *drone, int largeur_zone, int hauteur_zone) {
    // Marge de sécurité pour éviter que les drones se rapprochent trop des bords
    float marge = 20.0;

    // Vérifier si le drone est proche du mur gauche ou droit
    if (drone->x <= marge) {
        drone->vitesse = fabs(drone->vitesse);  // Diriger vers la droite
    } else if (drone->x >= (largeur_zone - marge)) {
        drone->vitesse = -fabs(drone->vitesse);  // Diriger vers la gauche
    }

    // Vérifier si le drone est proche du mur supérieur ou inférieur
    if (drone->y <= marge) {
        drone->vitesse = fabs(drone->vitesse);  // Diriger vers le bas
    } else if (drone->y >= (hauteur_zone - marge)) {
        drone->vitesse = -fabs(drone->vitesse);  // Diriger vers le haut
    }
}

// Fonction pour gérer tous les drones et vérifier leur proximité avec les murs
void gerer_murs_pour_tous_les_drones(Drone *drones, int nb_drones, int largeur_zone, int hauteur_zone) {
    for (int i = 0; i < nb_drones; i++) {
        gerer_murs(&drones[i], largeur_zone, hauteur_zone);
    }
}

int main() {

    // Initialiser SDL et SDL_image
    SDL_Init(SDL_INIT_VIDEO); // Initialise le sous-système vidéo de la bibliothèque SDL
    IMG_Init(IMG_INIT_PNG);  // Pour charger une image PNG


    SDL_Window *window = SDL_CreateWindow("Surveillance par Drones",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_SHOWN); // Crée une fenêtre SDL de 800x600 pixels, centrée sur l'écran

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    // Charger l'image de la carte
    SDL_Surface *image_surface = IMG_Load("carte.png");  // Charger l'image depuis un fichier
    
    SDL_Surface *image_surface2 = IMG_Load("carte.png");  // Charger l'image depuis un fichier

    if (!image_surface) {
        printf("Erreur de chargement de l'image: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Applique un flou à l'image
    apply_blur(image_surface, 1.5);

    apply_blur(image_surface2, 0.5);
    

    // Convertir l'image en texture pour l'afficher
    SDL_Texture *map_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface);  // Libérer la surface maintenant que nous avons la texture

  // Convertir l'image en texture pour l'afficher
    SDL_Texture *map_texture2 = SDL_CreateTextureFromSurface(renderer, image_surface2);
    SDL_FreeSurface(image_surface2);  // Libérer la surface maintenant que nous avons la texture

    
    // Création de trois drones pour la démonstration
    int nb_drones = 4;
    Drone drones[nb_drones];

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 20.0, 20.0, 5.0, 2.0, 10);
    init_drone(&drones[1], 2, 750.0, 20.0, 5.0, 1.7, 20);
    init_drone(&drones[2], 3, 30.0, 550.0, 5.0, 1.3, 30);
    init_drone(&drones[3], 3, 750.0, 550.0, 5.0, 1.0, 40);

    // Charge une image JPEG pour chaque drone
    for (int i = 0; i < nb_drones; i++) {
        drones[i].texture = charger_image_drone("drone1.jpg", renderer);
    }


    int running = 1;
    SDL_Event event;

    Uint32 start_time = SDL_GetTicks(); // Sauvegarde le temps

    // Initialise la graine pour la génération de nombres aléatoires.
    srand(time(NULL));

    // Alloue un tableau dynamique pour les directions X et Y de chaque drone.
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

        // Gérer les événements (comme la fermeture de la fenêtre)
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
        gerer_murs_pour_tous_les_drones(drones, nb_drones, 800, 600);


        // Vérification des collisions entre les drones
        gerer_collisions(drones, nb_drones);


        // Déplacer les drones
        deplacer_drones(drones, nb_drones, F);

        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);




        // Révéler la carte progressivement avec les drones
        for (int i = 0; i < nb_drones; i++) {
            reveal_map(renderer, map_texture, &drones[i]);
        }


        // Dessiner les drones
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
    SDL_DestroyTexture(map_texture2);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
