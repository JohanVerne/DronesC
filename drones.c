#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fonctionsEtStructures.h"

// Initialisation d'un drone
void init_drone(Drone *drone, int id, float x, float y, float z, float vitesse, float portee_com, int dims[3])
{
    drone->id = id;
    drone->x = x;
    drone->y = y;
    drone->z = z;
    drone->vitesse = vitesse;
    drone->portee_com = portee_com;
    drone->actif = 1; // Drone actif par défaut
    drone->dimensions[3] = dims;
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
void deplacer_drone(Drone *drone, Zone *zone, float dx, float dy, float dz) // TODO intégrer la gestion des collisions potentielles entre drones.
{
    if (drone->actif)
    {
        float nouveau_x = drone->x + dx * drone->vitesse;
        float nouveau_y = drone->y + dy * drone->vitesse;
        float nouveau_z = drone->z + dz * drone->vitesse;
        if (restrictionZone(zone, nouveau_x, nouveau_y, nouveau_z))
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
int restrictionZone(Zone *zone, float nx, float ny, float nz)
{
    if (nx > zone->xMin && nx < zone->xMax && ny > zone->yMin && ny < zone->yMax && nz > 0)
    {
        return 1;
    }
    return 0;
}

// Fonction de test de collision entre 2 drones

int collisionDrones(Drone *drone1, Drone *drone2) // TODO : Implémenter la gestion des collisions entre drones
{
    if (drone1->id != drone2->id)
    {
        return 1
    }
    return 0
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

    int dims[3] = {5, 5, 5};

    // Initialisation des drones avec des paramètres arbitraires
    init_drone(&drones[0], 1, 10.0, 20.0, 5.0, 1.5, 30.0, dims);
    init_drone(&drones[1], 2, 50.0, 60.0, 5.0, 1.2, 25.0, dims);
    init_drone(&drones[2], 3, 30.0, 40.0, 5.0, 1.8, 35.0, dims);

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
