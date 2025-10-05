#include <Arduino.h>

volatile int donneePartagee = 0;
/* Donnée partagée volatile : 
instruction directe au compilateur obligatoire en programme multi-tâches.*/

SemaphoreHandle_t mutex;
/* Mutex (Verrou d'Exclusion Mutuelle, de l'anglais Mutual Exclusion).
Protéger l'accès à la variable partagée. 
Un mutex fonctionne comme un "bâton de parole" ou la clé d'une pièce unique.
Une tâche doit d'abord prendre le mutex avant de pouvoir accéder à la variable partagée.
Si une autre tâche essaie de prendre le mutex alors qu'il est déjà pris, elle sera mise en pause, en attente.
Une fois que la première tâche a fini de manipuler la variable, elle doit rendre le mutex.
La tâche en attente peut alors prendre le mutex à son tour et accéder à la variable en toute sécurité.
Cette zone de code protégée par le mutex est appelée une section critique.*/

// Tâche 0 : Le Producteur
void tacheProducteur(void *parameter) {
  for (;;) {
    // On prend le mutex AVANT de toucher à quoi que ce soit de partagé (variable OU affichage)
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
      
      // --- DÉBUT DE LA SECTION CRITIQUE GLOBALE ---
      donneePartagee++;
      Serial.print("<<<<< Producteur (Cœur 0) a mis la valeur à : ");
      Serial.println(donneePartagee);
      // --- FIN DE LA SECTION CRITIQUE GLOBALE ---

      // On relâche le mutex une fois que TOUT est terminé
      xSemaphoreGive(mutex);
    }

    // Le délai reste en dehors
    delay(2000);
  }
}

// Tâche 1 : Le Consommateur
void tacheConsommateur(void *parameter) {
  int derniereValeurLue = -1;

  for (;;) {
    int valeurActuelle = 0;
    bool aChange = false;

    // Étape 1 : Vérifier s'il y a un changement (section critique très courte)
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
      if (donneePartagee != derniereValeurLue) {
        valeurActuelle = donneePartagee;
        derniereValeurLue = valeurActuelle;
        aChange = true;
      }
      xSemaphoreGive(mutex);
    }

    // Étape 2 : Afficher le changement (section critique séparée, uniquement si nécessaire)
    if (aChange) {
      if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        Serial.print(">>>>> Consommateur (Cœur 1) a lu la nouvelle valeur : ");
        Serial.println(valeurActuelle);
        xSemaphoreGive(mutex);
      }
    }
    
    delay(100); 
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- Test Producteur-Consommateur (Version Finale) ---");

  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL) {
    Serial.println("Erreur de création du Mutex");
    while(1);
  }

  xTaskCreatePinnedToCore(tacheProducteur, "Producteur", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tacheConsommateur, "Consommateur", 10000, NULL, 1, NULL, 1);
}

void loop() {
  // Vide
}