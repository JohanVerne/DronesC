#ifndef MES_FONCTIONS_H
#define MES_FONCTIONS_H

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

void init_drone(Drone *drone, int id, float x, float y, float z, float vitesse, float portee_com);
void definir_zone(Zone *zone, float xMin, float yMin, float xMax, float yMax);

void deplacer_drone(Drone *drone, Zone *zone, float dx, float dy, float dz);
int est_voisin(Drone *drone1, Drone *drone2);
void capturer_image(Drone *drone);
void detruire_drone(Drone *drone);
int restrictionZone(Zone *zone, float nx, float ny, float nz);

#endif