#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fonctionsEtStructures.h"
#include <SDL2/SDL.h>

// Initialisation d'un drone
void init_drone(Drone *drone, int id, float x, float y, float z, float vitesse, float portee_com, int dims[3])
{
    drone->id = id;
    drone->x = x;
    drone->y = y;
    drone->z = z;
    drone->vitesse = vitesse;
    drone->portee_com = portee_com;
    drone->actif = 1;      // Drone actif par défaut
    drone->texture = NULL; // initialize texture to NULL
    for (int i = 0; i < 3; i++)
    {
        drone->dimensions[i] = dims[i];
    }
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

// Déplacement des drones (mise à jour des positions) et gestion de collisions entre drones
void deplacer_drone(Drone *drone, Zone *zone, float dx, float dy, float dz)
{
    if (drone->actif)
    {
        float nouveau_x = drone->x + dx * drone->vitesse;
        float nouveau_y = drone->y + dy * drone->vitesse;
        float nouveau_z = drone->z + dz * drone->vitesse;
        if (restrictionZone(zone, nouveau_x, nouveau_y, nouveau_z, drone->dimensions))
        {
            drone->x = nouveau_x;
            drone->y = nouveau_y;
            drone->z = nouveau_z;
            printf("Drone %d déplacé à la position (%.2f, %.2f, %.2f)\n", drone->id, drone->x, drone->y, drone->z);
        }

        else
        {
            printf("Déplacement impossible du drone %d: sortie de la zone\n", drone->id);
            return;
        }
    }
    else
    {
        printf("Drone %d est inactif\n", drone->id);
    }
}

// Vérification que le déplacement du drone se fait bien dans la zone
int restrictionZone(Zone *zone, float nx, float ny, float nz, int dims[3])
{
    if (nx - dims[0] > zone->xMin && nx + dims[0] < zone->xMax && ny - dims[1] > zone->yMin && ny + dims[1] < zone->yMax && nz > 0) // restrictions en tenant compte des dimensions du drone
    {
        return 1;
    }
    return 0;
}

// Fonction de test de collision entre 2 drones

int collisionDrones(Drone *drone1, Drone *drone2)
{
    if (drone1->id != drone2->id)
    {
        float d1x = drone1->x; // définition des variables des drones pour une écriture plus concise des conditions de collision
        float d1y = drone1->y;
        float d1z = drone1->z;
        int dims1[3] = {drone1->dimensions[0], drone1->dimensions[1], drone1->dimensions[2]};
        float d2x = drone2->x;
        float d2y = drone2->y;
        float d2z = drone2->z;
        int dims2[3] = {drone2->dimensions[0], drone2->dimensions[1], drone2->dimensions[2]};

        if (d1x - dims1[0] <= d2x + dims2[0] && d1x + dims1[0] >= d2x - dims2[0]) // condition de collision sur l'axe x
        {
            if (d1y - dims1[1] <= d2y + dims2[1] && d1y + dims1[1] >= d2y - dims2[1]) // condition de collision sur l'axe y
            {
                if (d1z - dims1[2] <= d2z + dims2[2] && d1z + dims1[2] >= d2z - dims2[2]) // condition de collision sur l'axe z
                {
                    return 1;
                }
            }
        }
    }
    return 0; // pas de collision détectée
}

// Vérifie si le drone1 est à portée de communication du drone2
int est_voisin(Drone *drone1, Drone *drone2)
{
    float distance = sqrt(pow(drone1->x - drone2->x, 2) + pow(drone1->y - drone2->y, 2) + pow(drone1->z - drone2->z, 2));
    return distance <= drone1->portee_com;
}

// Gestion de la destruction d'un drone (désactivation)
void detruire_drone(Drone *drone)
{
    drone->actif = 0;
    printf("Drone %d a été détruit.\n", drone->id);
}

// Simulation de base de la gestion des drones
int test()
{
    // Initialisation de la zone et des drones
    Zone zone;
    definir_zone(&zone, 0.0, 0.0, 100.0, 100.0);

    int nb_drones = 4;
    Drone drones[nb_drones];

    int dims[3] = {5, 5, 5};

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 10.0, 20.0, 5.0, 1.5, 30.0, dims);
    init_drone(&drones[1], 2, 50.0, 60.0, 5.0, 1.2, 25.0, dims);
    init_drone(&drones[2], 3, 30.0, 40.0, 5.0, 1.8, 35.0, dims);
    init_drone(&drones[3], 4, 30.0, 36.0, 5.0, 1.0, 20.0, dims); // drone qui rentre en collision avec 3

    // Simulation de la collision entre drones 3 et 4
    if (collisionDrones(&drones[2], &drones[3]))
    {
        printf("Collision entre drones %d et %d.\n", drones[2].id, drones[3].id);
    }
    else
    {
        printf("Pas de collision entre drones %d et %d.\n", drones[2].id, drones[3].id);
    }

    // Simuler le déplacement des drones
    deplacer_drone(&drones[0], &zone, 1.0, 1.0, 0.0);
    deplacer_drone(&drones[1], &zone, -1.0, -1.0, 0.0);
    deplacer_drone(&drones[2], &zone, 0.5, 0.5, 0.0);
    deplacer_drone(&drones[0], &zone, 100.0, 0.0, 0.0); // Drone sortie de la zone
    // Vérification des voisins
    if (est_voisin(&drones[0], &drones[1]))
    {
        printf("Drone %d et Drone %d sont voisins.\n", drones[0].id, drones[1].id);
    }
    else
    {
        printf("Drone %d et Drone %d ne sont pas à portée de communication.\n", drones[0].id, drones[1].id);
    }

    // Simulation de la destruction d'un drone
    detruire_drone(&drones[1]);

    printf("\n");
    return 0;
}
// test(); // Lancement de la simulation de base
