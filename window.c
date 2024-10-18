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
SDL_Texture *charger_image_drone(const char *fichier_image, SDL_Renderer *renderer)
{

    fichier_image = "fichier_image.png";
    // Charger l'image avec SDL_image
    SDL_Surface *surface = IMG_Load(fichier_image);
    if (!surface)
    {
        printf("Erreur lors du chargement de l'image %s: %s\n", fichier_image, IMG_GetError());
        return NULL;
    }

    // Convertir la surface en texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Libérer la surface après la conversion en texture
    return texture;
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
}

void apply_blur(SDL_Surface *surface, float blur)
{
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
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int r_sum = 0, g_sum = 0, b_sum = 0;

            // Moyenne des couleurs des pixels voisins (3x3 box blur)
            for (int dy = -blur; dy <= blur; dy++)
            {
                for (int dx = -blur; dx <= blur; dx++)
                {
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
void dessiner_drones(Drone *drones, int nb_drones, SDL_Renderer *renderer)
{
    for (int i = 0; i < nb_drones; i++)
    {
        if (drones[i].actif && drones[i].texture)
        {
            int w, h;
            SDL_QueryTexture(drones[i].texture, NULL, NULL, &w, &h);

            // Scale the drone image based on the z parameter
            float scale = drones[i].z / 100.0f; // adjust the scale factor as needed
            int scaled_w = (int)(w * scale);
            int scaled_h = (int)(h * scale);

            // Définir la zone où dessiner l'image du drone
            SDL_Rect destination;
            destination.x = (int)drones[i].x - scaled_w / 2;
            destination.y = (int)drones[i].y - scaled_h / 2;
            destination.w = scaled_w;
            destination.h = scaled_h;

            // Dessiner la texture du drone
            SDL_RenderCopy(renderer, drones[i].texture, NULL, &destination);
        }
    }
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
    SDL_Surface *image_surface = IMG_Load("carte.png");
    if (!image_surface)
    {
        printf("Erreur de chargement de l'image: %s\n", IMG_GetError());
        return 1;
    }

    // Redimensionner l'image
    SDL_Surface *scaled_surface = SDL_CreateRGBSurface(0,
                                                       SDL_GetWindowSurface(window)->w,
                                                       SDL_GetWindowSurface(window)->h,
                                                       32, 0, 0, 0, 0);
    SDL_BlitScaled(image_surface, NULL, scaled_surface, NULL);

    // Convertir la surface redimensionnée en texture
    SDL_Texture *map_texture = SDL_CreateTextureFromSurface(renderer, scaled_surface);
    SDL_FreeSurface(image_surface);  // Libérer la surface d'origine
    SDL_FreeSurface(scaled_surface); // Libérer la surface redimensionnée

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

    for (int i = 0; i < nb_drones; i++)
    {
        drones[i].texture = charger_image_drone("fichier_image.png", renderer);
    }

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

        dessiner_drones(drones, 3, renderer);

        // Mettre à jour l'affichage
        SDL_RenderPresent(renderer);

        // Attendre 16 millisecondes (~60 FPS)
        SDL_Delay(16);
    }
    for (int i = 0; i < nb_drones; i++)
    {
        if (drones[i].texture)
        {
            SDL_DestroyTexture(drones[i].texture);
        }
    }
    SDL_FreeSurface(image_surface);
    // Nettoyer les ressources
    SDL_DestroyTexture(map_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}