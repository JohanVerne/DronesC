#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fonctionsEtStructures.h"

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
void deplacer_drone(Drone *drone, Zone *zone, float dx, float dy, float dz) // TODO :ajouter les restrictions de limite de zone et en intégrant la gestion des collisions potentielles entre drones. (potentiellement avec une autre fonction)
{
    if (drone->actif)
    {
        float nouveau_x = drone->x + dx * drone->vitesse;
        float nouveau_y = drone->y + dy * drone->vitesse;
        float nouveau_z = drone->z + dz * drone->vitesse;
        if (restrictionZone(zone, nouveau_x, nouveau_y, nouveau_z))
        {
            drone->x += dx * drone->vitesse;
            drone->y += dy * drone->vitesse;
            drone->z += dz * drone->vitesse;
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

int restrictionZone(Zone *zone, float nx, float ny, float nz)
{
    if (nx > zone->xMin && nx < zone->xMax && ny > zone->yMin && ny < zone->yMax && nz > 0)
    {
        return 1;
    }
    return 0;
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

// Simulation de base de la gestion des drones
int main()
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

    // Essai de capture d'images par tous les drones
    capturer_image(&drones[0]);
    capturer_image(&drones[1]); // Drone détruit
    capturer_image(&drones[2]);

    printf("\n");
    return 0;
}
