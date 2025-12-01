#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define MAX_NODOS 5
#define MAX_TIEMPO 100

// Estructura para representar un nodo
typedef struct {
    int id;
    int tiempo_local;
    int valor_hash;
    int ajuste;
    int valor_final;
} Node;

// Estructura para mensajes entre nodos
typedef struct {
    int id_emisor;
    int tiempo_valor;
    int valor_ajuste;
    char mensaje[20];
} Message;

// Función para calcular hash usando la hora del sistema y el ID del nodo
int calcularHora(int id_nodo) {
    time_t current_time = time(NULL);
    // Usar la hora del sistema y el ID del nodo para generar aleatoriedad
    int hash = (current_time + id_nodo * 7) % 1000;
    return hash;
}

// Función para generar tiempo local aleatorio
int generarTiempo() {
    return rand() % MAX_TIEMPO + 1;
}

// Función para intercambiar nodos (para ordenamiento)
void cambiarNodos(Node *a, Node *b) {
    Node temp = *a;
    *a = *b;
    *b = temp;
}

// Función para ordenar nodos por tiempo local (menor a mayor)
void ordenarNodos(Node nodo[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (nodo[j].tiempo_local > nodo[j + 1].tiempo_local) {
                cambiarNodos(&nodo[j], &nodo[j + 1]);
            }
        }
    }
}

// Función para encontrar el líder (menor hash)
int encontrarLider(Node nodo[], int n) {
    int indiceLi = 0;
    for (int i = 1; i < n; i++) {
        if (nodo[i].valor_hash < nodo[indiceLi].valor_hash) {
            indiceLi = i;
        }
    }
    return indiceLi;
}

// Función para simular el envío de tiempo al líder
void enviarTiempoL(Node *nodo, Node *lider, Message *mensaje) {
    printf("Nodo %d enviando hora local %d al líder (Nodo %d)\n", 
           nodo->id, nodo->tiempo_local, lider->id);
    
    strcpy(mensaje->mensaje, "TIME_REQUEST");
    mensaje->id_emisor = nodo->id;
    mensaje->tiempo_valor = nodo->tiempo_local;
    
    // Simular latencia de red
    struct timespec ts = {0, 100000000}; // 0.1 segundos (100 millones de nanosegundos)
    nanosleep(&ts, NULL);
}

// Función para calcular el tiempo promedio
int calcularTiempoPromedio(Node nodo[], int n) {
    int total_time = 0;
    for (int i = 0; i < n; i++) {
        total_time += nodo[i].tiempo_local;
    }
    return total_time / n;
}

// Función para calcular ajustes para cada nodo
void ajustesNodo(Node nodo[], int n, int tiempo_promedio) {
    printf("\n - Calculo de ajustes - \n");
    printf("Tiempo promedio calculado: %d\n", tiempo_promedio);
    
    for (int i = 0; i < n; i++) {
        nodo[i].ajuste = tiempo_promedio - nodo[i].tiempo_local;
        printf("Nodo %d: Tiempo local=%d, Ajuste=%d\n", 
               nodo[i].id, nodo[i].tiempo_local, nodo[i].ajuste);
    }
}

// Función para enviar ajustes a los nodos
void enviarAjuste(Node *lider, Node *nodo, Message *mensaje) {
    strcpy(mensaje->mensaje, "TIME_ajuste");
    mensaje->id_emisor = lider->id;
    mensaje->valor_ajuste = nodo->ajuste;
    
    printf("Líder (Nodo %d) enviando ajuste %d al Nodo %d\n", 
           lider->id, nodo->ajuste, nodo->id);
    
    // Simular latencia de red
    struct timespec ts = {0, 50000000}; // 0.05 segundos (50 millones de nanosegundos)
    nanosleep(&ts, NULL);
}

// Función para que un nodo reciba y aplique el ajuste
void recibir_ajustar(Node *nodo, Message *mensaje) {
    printf("Nodo %d recibiendo ajuste %d del líder\n", 
           nodo->id, mensaje->valor_ajuste);
    
    printf("Nodo %d: Hora antes del ajuste: %d\n", nodo->id, nodo->tiempo_local);
    
    nodo->valor_final = nodo->tiempo_local + nodo->ajuste;
    
    printf("Nodo %d: Hora después del ajuste: %d\n", nodo->id, nodo->valor_final);
}

// Función principal del algoritmo de Berkeley
void algoritmoBerkeley(Node nodo[], int n) {
    printf("ALGORITMO DE BERKELEY \n");
    
    // Calcular hash para cada nodo usando hora del sistema
    printf("- Calculo hash para determinar al lider -\n");
    for (int i = 0; i < n; i++) {
        nodo[i].valor_hash = calcularHora(nodo[i].id);
        printf("Nodo %d: Hash = %d\n", nodo[i].id, nodo[i].valor_hash);
    }
    
    // Encontrar el líder (menor hash)
    int indiceLi = encontrarLider(nodo, n);
    printf("\nLíder seleccionado: Nodo %d (Hash: %d)\n\n", 
           nodo[indiceLi].id, nodo[indiceLi].valor_hash);
    
    // Mostrar horas locales iniciales
    printf("- Horas locales inicales -\n");
    for (int i = 0; i < n; i++) {
        printf("Nodo %d: Hora local = %d\n", nodo[i].id, nodo[i].tiempo_local);
    }
    
    // Ordenar nodos por tiempo local (menor a mayor)
    Node ordenar_nodo[MAX_NODOS];
    for (int i = 0; i < n; i++) {
        ordenar_nodo[i] = nodo[i];
    }
    ordenarNodos(ordenar_nodo, n);
    
    printf("\n- Enviar hora al lider (menor a mayor) -\n");
    
    // Los nodos envían sus horas al líder en orden
    Message mensaje;
    for (int i = 0; i < n; i++) {
        if (ordenar_nodo[i].id != nodo[indiceLi].id) {
            enviarTiempoL(&ordenar_nodo[i], &nodo[indiceLi], &mensaje);
        }
    }
    
    // El líder calcula el tiempo promedio
    int tiempo_promedio = calcularTiempoPromedio(nodo, n);
    
    // El líder calcula los ajustes
    ajustesNodo(nodo, n, tiempo_promedio);
    
    // El líder envía ajustes a cada nodo
    printf("\n- Envio de ajustes -\n");
    for (int i = 0; i < n; i++) {
        if (nodo[i].id != nodo[indiceLi].id) {
            enviarAjuste(&nodo[indiceLi], &nodo[i], &mensaje);
            recibir_ajustar(&nodo[i], &mensaje);
        } else {
            // El líder también se ajusta a sí mismo
            printf("Líder (Nodo %d): Hora antes del ajuste: %d\n", 
                   nodo[i].id, nodo[i].tiempo_local);
            nodo[i].valor_final = nodo[i].tiempo_local + nodo[i].ajuste;
            printf("Líder (Nodo %d): Hora después del ajuste: %d\n", 
                   nodo[i].id, nodo[i].valor_final);
        }
    }
    
    // Mostrar resultados finales
    printf("\n - Resultados finales -\n");
    for (int i = 0; i < n; i++) {
        printf("Nodo %d: Hora ajustada final = %d\n", nodo[i].id, nodo[i].valor_final);
    }
}

int main() {
    srand(time(NULL));
    
    printf("- Simulacion del algoritmo de Berkeley -\n\n");
    
    // Inicializar nodos
    Node nodo[MAX_NODOS];
    int num_nodo = MAX_NODOS;
    
    printf("Inicializacion de los nodos: \n");
    for (int i = 0; i < num_nodo; i++) {
        nodo[i].id = i + 1;
        nodo[i].tiempo_local = generarTiempo();
        nodo[i].ajuste = 0;
        nodo[i].valor_final = 0;
        printf("Nodo %d inicializado con hora local: %d\n", 
               nodo[i].id, nodo[i].tiempo_local);
    }
    
    printf("\n");
    
    // Ejecutar algoritmo de Berkeley
    algoritmoBerkeley(nodo, num_nodo);
    
    // Verificación final
    printf("\n - Verificar sincronizacion - \n");
    printf("Todos los nodos deben tener la misma hora final (o muy similar):\n");
    for (int i = 0; i < num_nodo; i++) {
        printf("Nodo %d: %d\n", nodo[i].id, nodo[i].valor_final);
    }
    
    return 0;
}
