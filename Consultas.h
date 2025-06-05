#pragma once // Directiva para asegurar que este archivo se incluya una sola vez al compilar.

#include <iostream>
#include <string>
#include <vector>
#include <limits>       // Necesario para limpiar el buffer de entrada (cin).
#include "Estructuras.h" // Importamos nuestras definiciones de Venta, MapaPaises, etc.

// --- FUNCIÓN AUXILIAR PARA MANEJO DE FECHAS ---
/**
 * Convierte una fecha en formato string "DD/MM/YYYY" a un entero YYYYMMDD.
 * Dato: fecha El string de la fecha a convertir.
 * esta funcion devuelve Un entero que representa la fecha. Ej: "05/06/2025" -> 20250605.
 *
 * Se crea esta función para simplificar las comparaciones de rangos.
 * Comparar enteros es mucho más eficiente y simple que comparar strings de fechas.
 * El formato YYYYMMDD asegura que una fecha posterior siempre tendrá un número mayor.
 * Se usa try-catch para manejar de forma segura fechas con formato incorrecto.
 */
int fechaAEntero(const string& fecha) {
    try {
        if (fecha.length() < 10) return 0; // Comprobación básica de longitud.
        // Se extraen y concatenan los substrings de año, mes y día, y se convierten a entero.
        return stoi(fecha.substr(6, 4) + fecha.substr(3, 2) + fecha.substr(0, 2));
    } catch(...) {
        // Si la conversión falla, se devuelve 0 para evitar que el programa se detenga.
        return 0;
    }
}

// =====================================================================================
// --- CONSULTAS BASADAS EN EL VECTOR DE VENTAS (DATOS CRUDOS) ---
// =====================================================================================

/**
 * Muestra todas las ventas realizadas en una ciudad específica.
 * ventas El vector que contiene TODOS los registros de venta individuales.
 *
 * Esta consulta necesita mostrar detalles de ventas individuales, por lo que
 * debe usar el vector `todasLasVentas` que contiene los datos crudos. No puede usar
 * la estructura agregada (el mapa) porque esa estructura pierde los detalles de la ciudad.
 * La estrategia es una simple iteración lineal.
 */
void consultaPorCiudad(const vector<Venta>& ventas) {
    string ciudad;
    cout << "-> Ingrese el nombre de la ciudad: ";
    getline(cin, ciudad); // Usamos getline para leer nombres con espacios.

    cout << "\n--- Ventas en " << ciudad << " ---" << endl;
    bool encontradas = false; // Flag para dar un mensaje amigable si no hay resultados.

    // Se recorre cada venta en el vector.
    for (const auto& v : ventas) {
        // Si la ciudad de la venta coincide con la buscada, se imprime.
        if (v.ciudad == ciudad) {
            cout << "  ID: " << v.idVenta << ", Fecha: " << v.fecha << ", Producto: " << v.producto
                 << ", Monto: $" << v.montoTotal << endl;
            encontradas = true;
        }
    }
    if (!encontradas) {
        cout << "No se encontraron ventas para esa ciudad." << endl;
    }
}

/**
 * Muestra las ventas de un país dentro de un rango de fechas.
 * Ventas El vector con todos los registros de venta.
 *
 * Al igual que la consulta por ciudad, esta requiere datos individuales,
 * por lo que opera sobre el vector. La clave aquí es el uso de la función auxiliar
 * `fechaAEntero` para hacer la comparación del rango de fechas de forma eficiente.
 */
void consultaPorFechaYPais(const vector<Venta>& ventas) {
    string pais, fechaInicio, fechaFin;
    cout << "-> Ingrese el pais: ";
    getline(cin, pais);
    cout << "-> Ingrese la fecha de inicio (DD/MM/YYYY): ";
    getline(cin, fechaInicio);
    cout << "-> Ingrese la fecha de fin (DD/MM/YYYY): ";
    getline(cin, fechaFin);

    // Convertimos las fechas de entrada a enteros para poder compararlas numéricamente.
    int f_inicio = fechaAEntero(fechaInicio);
    int f_fin = fechaAEntero(fechaFin);

    cout << "\n--- Ventas en " << pais << " entre " << fechaInicio << " y " << fechaFin << " ---" << endl;
    bool encontradas = false;
    for (const auto& v : ventas) {
        // Primero filtramos por país, que es una comparación de strings simple.
        if (v.pais == pais) {
            // Luego, solo para las ventas del país correcto, hacemos la conversión de fecha y comparamos.
            int f_venta = fechaAEntero(v.fecha);
            if (f_venta >= f_inicio && f_venta <= f_fin) {
                 cout << "  ID: " << v.idVenta << ", Ciudad: " << v.ciudad << ", Fecha: " << v.fecha << ", Producto: " << v.producto
                      << ", Monto: $" << v.montoTotal << endl;
                 encontradas = true;
            }
        }
    }
    if (!encontradas) {
        cout << "No se encontraron ventas para ese rango." << endl;
    }
}


// =====================================================================================
// --- CONSULTAS BASADAS EN EL MAPA AGREGADO (DATOS PROCESADOS) ---
// =====================================================================================

/**
 * Compara diversas métricas (monto total, producto más vendido, etc.) entre dos países.
 * Datos: El mapa anidado (`MapaPaises`) que contiene los datos pre-agregados.
 *
 * Esta es una consulta analítica. Usar la estructura agregada es clave para
 * el rendimiento. En lugar de recorrer miles de ventas, solo recorremos los pocos
 * productos vendidos en cada país. Se usa `.at()` para acceder a los datos, que
 * es seguro porque está dentro de un bloque try-catch que maneja el caso de que un país no exista.
 */
void compararDosPaises(const MapaPaises& datos) {
    string p1, p2;
    cout << "-> Ingrese el primer pais: ";
    getline(cin, p1);
    cout << "-> Ingrese el segundo pais: ";
    getline(cin, p2);

    try {
        // Accedemos a los datos de cada país. Si no existe, .at() lanza una excepción.
        const MapaProductos& map1 = datos.at(p1);
        const MapaProductos& map2 = datos.at(p2);

        // --- a. Monto total de ventas ---
        double monto1 = 0;
        // Iteramos sobre los pares (producto, estadísticas) del primer país.
        for (const auto& par : map1) { monto1 += par.second.montoTotalVendido; }
        double monto2 = 0;
        for (const auto& par : map2) { monto2 += par.second.montoTotalVendido; }

        cout << "\n--- a. Monto Total de Ventas ---" << endl;
        cout << "  " << p1 << ": $" << monto1 << endl;
        cout << "  " << p2 << ": $" << monto2 << endl;

        // --- b. Producto más vendido (por cantidad de unidades) ---
        string prodMasVendido1; int maxVentas1 = -1;
        for (const auto& par : map1) {
            if (par.second.cantidadTotalVendida > maxVentas1) {
                maxVentas1 = par.second.cantidadTotalVendida;
                prodMasVendido1 = par.first;
            }
        }
        string prodMasVendido2; int maxVentas2 = -1;
        for (const auto& par : map2) {
            if (par.second.cantidadTotalVendida > maxVentas2) {
                maxVentas2 = par.second.cantidadTotalVendida;
                prodMasVendido2 = par.first;
            }
        }
        cout << "\n--- b. Producto Mas Vendido ---" << endl;
        cout << "  " << p1 << ": " << prodMasVendido1 << " (" << maxVentas1 << " unidades)" << endl;
        cout << "  " << p2 << ": " << prodMasVendido2 << " (" << maxVentas2 << " unidades)" << endl;

        // --- c. Medio de envío más usado ---
        // Para esto, necesitamos agregar los conteos de todos los productos de cada país.
        // Creamos mapas temporales para esta suma.
        unordered_map<string, int> envios1, envios2;
        // Doble bucle: por cada producto, recorremos su mapa de medios de envío.
        for(const auto& par : map1) { for(const auto& envio : par.second.conteoMediosEnvio) { envios1[envio.first] += envio.second; }}
        for(const auto& par : map2) { for(const auto& envio : par.second.conteoMediosEnvio) { envios2[envio.first] += envio.second; }}

        // Ahora buscamos el máximo en los mapas temporales ya agregados.
        string envioMasUsado1; int maxEnvios1 = -1;
        for(const auto& par : envios1) { if(par.second > maxEnvios1) {maxEnvios1 = par.second; envioMasUsado1 = par.first;}}
        string envioMasUsado2; int maxEnvios2 = -1;
        for(const auto& par : envios2) { if(par.second > maxEnvios2) {maxEnvios2 = par.second; envioMasUsado2 = par.first;}}

        cout << "\n--- c. Medio de Envio Mas Usado ---" << endl;
        cout << "  " << p1 << ": " << envioMasUsado1 << " (" << maxEnvios1 << " veces)" << endl;
        cout << "  " << p2 << ": " << envioMasUsado2 << " (" << maxEnvios2 << " veces)" << endl;

    } catch(const out_of_range& e) {
        cerr << "Error: Uno o ambos paises no fueron encontrados en los datos." << endl;
    }
}

/**
 * Compara las ventas de dos productos específicos a través de todos los países.
 * Datos: El mapa anidado con los datos agregados.
 *
 * La estrategia aquí es iterar sobre el nivel superior del mapa (los países)
 * y para cada país, buscar los datos de los dos productos de interés.
 * Usamos `.count()` para verificar si un producto existe en un país antes de acceder a él,
 * lo que evita errores y nos permite mostrar '0' si no se vendió.
 */
void compararDosProductos(const MapaPaises& datos) {
    string prod1_str, prod2_str;
    cout << "-> Ingrese el primer producto: ";
    getline(cin, prod1_str);
    cout << "-> Ingrese el segundo producto: ";
    getline(cin, prod2_str);

    cout << "\n--- Comparativa de '" << prod1_str << "' vs '" << prod2_str << "' ---" << endl;
    // Usamos printf para un formato de tabla alineado y limpio.
    printf("%-15s | %-25s | %-25s\n", "Pais", (prod1_str + " (Cant/Monto)").c_str(), (prod2_str + " (Cant/Monto)").c_str());
    cout << "----------------+---------------------------+--------------------------" << endl;

    // Iteramos sobre todos los países en nuestra estructura de datos.
    for (const auto& parPais : datos) {
        const string& pais = parPais.first;
        const MapaProductos& mapaProd = parPais.second;

        int cant1 = 0; double monto1 = 0.0;
        // .count() es una forma segura y eficiente de ver si la clave existe.
        if(mapaProd.count(prod1_str)) {
            const auto& stats = mapaProd.at(prod1_str); // Si existe, la obtenemos con .at()
            cant1 = stats.cantidadTotalVendida;
            monto1 = stats.montoTotalVendido;
        }

        int cant2 = 0; double monto2 = 0.0;
        if(mapaProd.count(prod2_str)) {
            const auto& stats = mapaProd.at(prod2_str);
            cant2 = stats.cantidadTotalVendida;
            monto2 = stats.montoTotalVendido;
        }

        // Solo mostramos la fila si al menos uno de los productos tuvo ventas en ese país.
        if (cant1 > 0 || cant2 > 0) {
            printf("%-15s | %-4d / $%-15.2f | %-4d / $%-15.2f\n", pais.c_str(), cant1, monto1, cant2, monto2);
        }
    }
}

/**
 *  Busca productos cuyo monto promedio de venta está por encima o por debajo de un umbral.
 *  datos: El mapa anidado con los datos agregados.
 *
 * Esta consulta es un ejemplo perfecto de por qué pre-agregar los datos es
 * tan útil. El cálculo del promedio (`monto / cantidad`) se puede hacer al momento de la
 * consulta de forma muy rápida, ya que tenemos los totales listos.
 */
void buscarPorPromedio(const MapaPaises& datos) {
    string pais, modo;
    double umbral;
    cout << "-> Ingrese el pais: ";
    getline(cin, pais);
    cout << "-> Ingrese el monto umbral (ej: 500): ";
    cin >> umbral;
    cout << "-> Buscar productos con promedio (mayor/menor) al umbral?: ";
    cin >> modo;
    // Limpiamos el buffer de entrada después de leer un número o palabra.
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    try {
        const MapaProductos& mapaProd = datos.at(pais);
        cout << "\n--- Resultados para " << pais << " ---" << endl;

        for (const auto& par : mapaProd) {
            const string& producto = par.first;
            const auto& stats = par.second;
            // Importante: verificar que la cantidad no sea cero para evitar división por cero.
            if (stats.cantidadTotalVendida > 0) {
                double promedio = stats.montoTotalVendido / stats.cantidadTotalVendida;

                // Comprobamos si el modo y el promedio cumplen la condición.
                if ((modo == "mayor" && promedio > umbral) || (modo == "menor" && promedio < umbral)) {
                    cout << "  " << producto << " (Promedio por unidad: $" << promedio << ")" << endl;
                }
            }
        }
    } catch(const out_of_range& e) {
        cerr << "Error: El pais ingresado no fue encontrado." << endl;
    }
}