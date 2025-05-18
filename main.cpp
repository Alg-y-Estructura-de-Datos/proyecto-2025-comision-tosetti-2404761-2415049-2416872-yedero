#include <windows.h> // (solo en Windows) Libreria para arreglar los caracteres en la consola.
#include <vector>      // Usamos vector
#include <unordered_map> // HashMap de la libreria estandar
#include <algorithm>   // Para sort
#include <iostream>    // Para cout, endl
#include <string>      // Para string
#include <fstream>     // Para ifstream
#include <limits> // Utilizado para generar un numero muy grande
#include <sstream>     // Para stringstream
#include <iomanip>     // Para fixed, setprecision (útil para imprimir double)
// #include <cctype>    // Para tolower (si quieres estandarizar mayúsculas/minúsculas)
// #include <locale>    // Para tolower con locale

// #include <chrono> // <--- Eliminado
// #include "Lista/Lista.h" // <--- Eliminado

#define baseDeDatos "ventas_sudamerica.csv"

using namespace std; // Considera no usar using namespace std; en archivos .h o proyectos grandes

// Función auxiliar para eliminar espacios al inicio y final de una cadena
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) {
        return str; // No hay caracteres no blancos
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

/*
// Opcional: Función para estandarizar a minúsculas si necesitas insensibilidad a mayúsculas/minúsculas
string to_lower(const string& str) {
    string lower_str = str;
    // locale loc; // Si necesitas soporte completo de locale
    // for (char& c : lower_str) {
    //     c = tolower(c, loc); // Usar con locale
    // }
    // O simple tolower para ASCII/basic:
    for (char& c : lower_str) {
       if (c >= 'A' && c <= 'Z') {
           c = c - ('A' - 'a');
       }
       // Considerar tildes y otros caracteres si es necesario
    }
    return lower_str;
}
*/

struct Venta {
    int idVenta;
    string fecha;
    string pais;
    string ciudad;
    string cliente;
    string producto;
    string categoria;
    int cantidad;
    double precioUnitario; // Usamos double
    double montoTotal;     // Usamos double
    string medioEnvio;
    string estadoEnvio;
};

// La función procesar ahora trabaja con vector<Venta>
void procesar(vector<Venta>& vectorBaseDeDatos){ // <-- Recibe un vector por referencia
    ifstream archivo(baseDeDatos);

    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo " << baseDeDatos << endl;
        return;
    }

    string linea;
    getline(archivo, linea); // Leer encabezado

    cout << "Iniciando lectura de datos..." << endl; // Mensaje de progreso
    // auto inicio_lectura = chrono::high_resolution_clock::now(); // <--- Eliminado

    // Bucle UNICO para leer las líneas de datos
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string campo;
        Venta venta;

        // Procesamos cada campo
        getline(ss, campo, ','); venta.idVenta = stoi(campo);
        getline(ss, venta.fecha, ',');

        getline(ss, venta.pais, ',');
        venta.pais = trim(venta.pais); // <-- Aplicar limpieza
        // if needed: venta.pais = to_lower(venta.pais);

        getline(ss, venta.ciudad, ',');
        venta.ciudad = trim(venta.ciudad); // <-- Aplicar limpieza
        // if needed: venta.ciudad = to_lower(venta.ciudad);

        getline(ss, venta.cliente, ',');

        getline(ss, venta.producto, ',');
        venta.producto = trim(venta.producto); // <-- Aplicar limpieza
        // if needed: venta.producto = to_lower(venta.producto);

        getline(ss, venta.categoria, ',');
        venta.categoria = trim(venta.categoria); // <-- Aplicar limpieza
        // if needed: venta.categoria = to_lower(venta.categoria);


        getline(ss, campo, ','); venta.cantidad = stoi(campo);
        getline(ss, campo, ','); venta.precioUnitario = stod(campo); // <-- Usar stod
        getline(ss, campo, ','); venta.montoTotal = stod(campo);     // <-- Usar stod

        getline(ss, venta.medioEnvio, ',');
        venta.medioEnvio = trim(venta.medioEnvio); // <-- Aplicar limpieza
        // if needed: venta.medioEnvio = to_lower(venta.medioEnvio);


        getline(ss, venta.estadoEnvio, '\n');
        if (!venta.estadoEnvio.empty() && venta.estadoEnvio.back() == '\r') {
            venta.estadoEnvio.pop_back();
        }
         venta.estadoEnvio = trim(venta.estadoEnvio); // <-- Aplicar limpieza
        // if needed: venta.estadoEnvio = to_lower(venta.estadoEnvio);


        vectorBaseDeDatos.push_back(venta); // <-- Insertamos en el vector
    }

    archivo.close();

    // auto fin_lectura = chrono::high_resolution_clock::now(); // <--- Eliminado
    // chrono::duration<double, milli> duracion_ms = fin_lectura - inicio_lectura; // <--- Eliminado

    cout << "Base de datos procesada con exito. Total de ventas leidas: " << vectorBaseDeDatos.size() << endl; // <-- Usar .size()
    // cout << "Tiempo de lectura y llenado de vector: " << duracion_ms.count() << " ms" << endl; // <--- Eliminado
}


// La función top5 ahora trabaja con vector<Venta> (por referencia constante)
void top5VentasPorPaisCiudad(const vector<Venta>& vectorBD) { // <-- Recibe vector por referencia constante
    unordered_map<string, unordered_map<string, double>> ventasPorPaisCiudad;

    cout << "\nIniciando agregación de ventas por país y ciudad..." << endl; // Mensaje de progreso
    // auto inicio_agregacion = chrono::high_resolution_clock::now(); // <--- Eliminado

    // --- Bucle eficiente O(N) para recorrer el vector ---
    for (const auto& ventaActual : vectorBD) { // <-- Bucle basado en rango (O(N))

        // La lógica de agregación es la misma, usando las variables de la venta actual del vector
        // Las claves ya deben estar limpias si aplicaste trim/tolower en procesar
        unordered_map<string,double>& mapaCiudadesDelPais = ventasPorPaisCiudad[ventaActual.pais];
        mapaCiudadesDelPais[ventaActual.ciudad] += ventaActual.montoTotal;
    }
    // --- Fin bucle de agregación ---

    // auto fin_agregacion = chrono::high_resolution_clock::now(); // <--- Eliminado
    // chrono::duration<double, micro> duracion_agregacion_us = fin_agregacion - inicio_agregacion; // <--- Eliminado
    // cout << "Tiempo de agregación en el mapa (País/Ciudad): " << duracion_agregacion_us.count() << " us" << endl; // <--- Eliminado


    cout << "\n--- Top 5 Ciudades con Mayor Monto de Ventas por País ---" << endl;
    // auto inicio_top5 = chrono::high_resolution_clock::now(); // <--- Eliminado

    // Bucle exterior: Recorre el mapa principal (países)
    for (const auto& [pais, mapaCiudades] : ventasPorPaisCiudad) {

        cout << "País: " << pais << ":" << endl;

        // Lógica para obtener el Top 5 para el país actual (copiar a vector y ordenar)
        vector<pair<string, double>> ciudades_a_ordenar;
        for (const auto& [ciudad, totalVentas] : mapaCiudades) {
            ciudades_a_ordenar.push_back({ciudad, totalVentas});
        }

        sort(ciudades_a_ordenar.begin(), ciudades_a_ordenar.end(),
                  [](const pair<string, double>& a, const pair<string, double>& b) {
                      return a.second > b.second;
                  });

        int count = 0;
        for (const auto& pair : ciudades_a_ordenar) {
            if (count < 5) {
                cout << "  " << (count + 1) << ". " << pair.first << ": " << fixed << setprecision(2) << pair.second << endl;
                count++;
            } else {
                break;
            }
        }

        if (ciudades_a_ordenar.empty()) {
             cout << "  (No hay datos de ventas para ciudades en este país)" <<endl;
        }

        cout << "--------------------" << endl;
    }
}


// La función montoTotal ahora trabaja con vector<Venta> (por referencia constante)
void montoTotalProductoXPais(const vector<Venta>& vectorBD) { // <-- Recibe vector por referencia constante
    unordered_map<string, unordered_map<string, double>> montoPorPaisProducto;

    cout << "\nIniciando agregación de ventas por producto y país..." << endl; // Mensaje de progreso

    // --- Bucle eficiente O(N) para recorrer el vector ---
    for (const auto& ventaActual : vectorBD) { // <-- Bucle basado en rango (O(N))

        // La lógica de agregación es la misma
        // Las claves ya deben estar limpias si aplicaste trim/tolower en procesar
        unordered_map<string,double>& mapaProductosDelPais = montoPorPaisProducto[ventaActual.pais];
        mapaProductosDelPais[ventaActual.producto] += ventaActual.montoTotal;
    }
     // --- Fin bucle de agregación ---

    cout << "\n--- Ventas de productos discriminado por Pais ---" << endl;

    // Bucle exterior: Recorre el mapa principal (países)
    for (const auto& [pais, mapaProductosDelPais] : montoPorPaisProducto) {

        cout << "País: " << pais << ":" << endl;

        for (const auto& pair : mapaProductosDelPais) {
                cout << "  - " << pair.first << ": " << fixed << setprecision(2) << pair.second << endl;
        }

        cout << "--------------------" << endl;
    }


}

// Promedio de ventas por categoría en cada país
void promVentasXCategoriaXPais(vector <Venta> vectorBD) {
    struct datosVenta {
        double totalVentas;
        int cantidadVentas;
    };

    unordered_map<string, unordered_map<string, datosVenta>> promVentasPaisCategoria;
    // --- Bucle eficiente O(N) para recorrer el vector ---
    int i = 0; // Contador para saber cuantos productos de una categoria hay
    for (const auto& ventaActual : vectorBD) { // <-- Bucle basado en rango (O(N))

        // La lógica de agregación es la misma
        // Las claves ya deben estar limpias si aplicaste trim/tolower en procesar
        unordered_map<string,datosVenta>& mapaCategoriasVentas = promVentasPaisCategoria[ventaActual.pais];
        mapaCategoriasVentas[ventaActual.categoria].totalVentas += ventaActual.montoTotal;
        mapaCategoriasVentas[ventaActual.categoria].cantidadVentas++;

    }
    // --- Fin bucle de agregación ---

    // Bucle exterior: Recorre el mapa principal (países)
    for (const auto& [pais, mapaCategoriasVentas] : promVentasPaisCategoria) {
        cout << "País: " << pais << ":" << endl;

        for (const auto& pair : mapaCategoriasVentas) {
            // Verifica si la cantidad es mayor que cero antes de dividir
            if (pair.second.cantidadVentas > 0) {
                double promedio = static_cast<double>(pair.second.totalVentas) / pair.second.cantidadVentas; // Asegurar división flotante
                cout << "  - Promedio de venta de " << pair.first << ": "
                     << fixed << setprecision(2) << promedio << endl;
            } else {
                // Manejar el caso donde la cantidad es 0 (aunque con tu lógica no debería pasar si el total > 0)
                cout << "  - Promedio de venta de " << pair.first << ": Sin ventas registradas" << endl;
            }
        }
        cout << "--------------------" << endl;
    }
}

// Medio de envío más utilizado por país
void envioMasUtilPais(vector <Venta> vectorBD) {
   unordered_map<string, unordered_map<string, int>> envioMasUtilizadoPais;
    // --- Bucle eficiente O(N) para recorrer el vector ---
    for (const auto& ventaActual : vectorBD) { // <-- Bucle basado en rango (O(N))

        // La lógica de agregación es la misma
        // Las claves ya deben estar limpias si aplicaste trim/tolower en procesar
        unordered_map<string,int>& mapaConteos = envioMasUtilizadoPais[ventaActual.pais];
        mapaConteos[ventaActual.medioEnvio]++;
    }
    // --- Fin bucle de agregación ---






    cout << "\n--- Metodo de envio mas utilizado en cada Pais ---" << endl;

    // Bucle exterior: Recorre el mapa principal (países)
    for (const auto& [pais, mapaConteos] : envioMasUtilizadoPais) {

        cout << "País: " << pais << ":" << endl;

        //Encontra el medio mas utilizado

        int max_conteo = -1; //Valor a comparar menor a cualquiera.
        string metodoMasUsado = "";

        for (const auto& [metodo, conteoEnvio] : mapaConteos) {
            if (conteoEnvio > max_conteo) {
                metodoMasUsado = metodo;
                max_conteo = conteoEnvio;
            }
        }

        // Imprimimos el metodo mas usado
        cout << "  - " << metodoMasUsado << ": " <<  max_conteo << " envios" << endl;


        cout << "--------------------" << endl;
    }

}
// Medio de envío más utilizado por categoría
void envioMasUtilCategoria(vector<Venta> vectorBD) {
    unordered_map<string, unordered_map<string, int>> envioMasUtilizadoCategoria;
    // --- Bucle eficiente O(N) para recorrer el vector ---
    for (const auto& ventaActual : vectorBD) { // <-- Bucle basado en rango (O(N))

        // La lógica de agregación es la misma
        // Las claves ya deben estar limpias si aplicaste trim/tolower en procesar
        unordered_map<string,int>& mapaConteos = envioMasUtilizadoCategoria[ventaActual.categoria];
        mapaConteos[ventaActual.medioEnvio]++;
    }
    // --- Fin bucle de agregación ---


    cout << "\n--- Metodo de envio mas utilizado en cada Categoria ---" << endl;

    // Bucle exterior: Recorre el mapa principal (países)
    for (const auto& [pais, mapaConteos] : envioMasUtilizadoCategoria) {

        cout << "Categoria: " << pais << ":" << endl;

        //Encontra el medio mas utilizado

        int max_conteo = -1; //Valor a comparar menor a cualquiera.
        string metodoMasUsado = "";

        for (const auto& [metodo, conteoEnvio] : mapaConteos) {
            if (conteoEnvio > max_conteo) {
                metodoMasUsado = metodo;
                max_conteo = conteoEnvio;
            }
        }

        // Imprimimos el metodo mas usado
        cout << "  - " << metodoMasUsado << ": " <<  max_conteo << " envios" << endl;


        cout << "--------------------" << endl;
    }

}
// Día con mayor cantidad de ventas (por monto de dinero) en toda la base de datos
void diaMasVentas(vector<Venta> vectorBD) {
    unordered_map<string, double> ventasPorDia;
    for (const auto& ventaActual : vectorBD) {
        ventasPorDia[ventaActual.fecha] += ventaActual.montoTotal;
    }
    double max_monto = -1.0;
    string fechaMasVentas = "";
    for (const auto& [fecha, monto] : ventasPorDia) {
        if (monto > max_monto) {
            fechaMasVentas = fecha;
            max_monto = monto;
        }
    }

    cout<<"El Dia con mas ventas fue el "<<fechaMasVentas<< " Con un monto de $"<<max_monto<<endl;
}
// Estado de envío más frecuente por país
void estadoEnvioMasFrecuenteXPais(vector <Venta> vectorBD) {
    unordered_map<string, unordered_map<string, int>> estadoEnvioPais;
    // --- Bucle eficiente O(N) para recorrer el vector ---
    for (const auto& ventaActual : vectorBD) { // <-- Bucle basado en rango (O(N))

        // La lógica de agregación es la misma
        // Las claves ya deben estar limpias si aplicaste trim/tolower en procesar
        unordered_map<string,int>& mapaConteoEstado = estadoEnvioPais[ventaActual.pais];
        mapaConteoEstado[ventaActual.estadoEnvio]++;
    }
    // --- Fin bucle de agregación ---

    cout << "\n--- Estado de de envio mas frecuente en cada Pais ---" << endl;

    // Bucle exterior: Recorre el mapa principal (países)
    for (const auto& [pais, mapaConteos] : estadoEnvioPais) {

        cout << "País: " << pais << ":" << endl;

        //Encontra el medio mas utilizado

        int max_conteo = -1; //Valor a comparar menor a cualquiera.
        string estadoMasUsado = "";

        for (const auto& [metodo, conteoEnvio] : mapaConteos) {
            if (conteoEnvio > max_conteo) {
                estadoMasUsado = metodo;
                max_conteo = conteoEnvio;
            }
        }

        // Imprimimos el metodo mas usado
        cout << "  - " << estadoMasUsado << ": " <<  max_conteo << " envios" << endl;


        cout << "--------------------" << endl;
    }

}
// Producto más vendido en cantidad total (no en dinero, sino en unidades)
    void productoMasVendido(vector <Venta> vectorBD) {
    unordered_map<string, int> productosVendidos;
    for (const auto& ventaActual : vectorBD) {
        productosVendidos[ventaActual.producto] += ventaActual.cantidad;
    }

    int max_conteo = -1;
    string nombre_producto = "";
    for (const auto& [producto, contador] : productosVendidos) {
        if (max_conteo < contador) {
            max_conteo = contador;
            nombre_producto = producto;
        }
    }

    cout<<"El Producto mas vendido es: "<<nombre_producto<<" Con una cantidad de "<<max_conteo<<" unidades vendidas."<<endl;

}
// Producto menos vendido en cantidad total
    void productoMenosVendido(vector <Venta> vectorBD) {
    unordered_map<string, int> productosVendidos;
    for (const auto& ventaActual : vectorBD) {
        productosVendidos[ventaActual.producto] += ventaActual.cantidad;
    }

    // Inicializar el mínimo con el valor máximo posible para int.
    // Esto asegura que la cantidad del primer producto que encontremos sea menor o igual a menos_vendido.
    int menos_vendido = numeric_limits<int>::max();

    string nombre_producto = "";
    for (const auto& [producto, contador] : productosVendidos) {
        if (menos_vendido > contador) {
            menos_vendido = contador;
            nombre_producto = producto;
        }
    }

    cout<<"El Producto menos vendido es: "<<nombre_producto<<" Con una cantidad de "<<menos_vendido<<" unidades vendidas."<<endl;


}


void modificaciones() {
    //Agregar una venta (guiando al usuario paso a paso para ingresar todos los campos)
    //Eliminar una venta (El usuario ingresarà un pais o una ciudad y el programa filtrarà mostrando lo seleccionado)
    //Modificar una venta (selección por ID de venta; permitir modificar cualquier campo) ACA PODEMOS USAR UN ARBOLLL
    //Reprocesar
}

void consultas() {
    //El listado de ventas realizadas en una ciudad específica
    //El listado de ventas realizadas en un rango de fechas por país
    //Comparación entre dos países:
        //a.monto total de ventas
        //b.productos más vendidos
        //c.medio de envío más usado
    //Comparación entre dos productos discriminado por todos los paises:
        //a. cantidad total vendida
        //b. monto total
    //Buscar productos vendidos en promedio por debajo de un monto total especificado por el usuario(umbral) y por paìs
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // UTF-8 Para arreglar outputs de la consola.
    // Declaramos un vector en lugar de Lista
    vector<Venta> vectorBaseDeDatos;

    cout<<"Procesando base de datos espere un momento..."<<endl;

    // Llamamos a procesar, pasando el vector
    procesar(vectorBaseDeDatos);

    // Llamamos a las funciones de análisis, pasando el vector
    top5VentasPorPaisCiudad(vectorBaseDeDatos);
    montoTotalProductoXPais(vectorBaseDeDatos);
    promVentasXCategoriaXPais(vectorBaseDeDatos);
    envioMasUtilPais(vectorBaseDeDatos);
    envioMasUtilCategoria(vectorBaseDeDatos);
    diaMasVentas(vectorBaseDeDatos);
    estadoEnvioMasFrecuenteXPais(vectorBaseDeDatos);
    productoMasVendido(vectorBaseDeDatos);
    productoMenosVendido(vectorBaseDeDatos);
    int opcion;
    do {
        cout<<"MENU"<<endl;
        cout<<endl;
        cout<<"1.Modificar"<<endl;
        cout<<"2."<<endl;
        cout<<"3."<<endl;
        cout<<"4."<<endl;
        cout<<"5."<<endl;
        cout<<"6.Salir"<<endl;
        cout<<"Ingrese una opcion:"<<endl;
        cin>>opcion;
        switch (opcion) {
            case 1:
                // modificaciones(vectorBaseDeDatos); // Pasar el vector si la funcion lo necesita
                break;
            case 2:
                // consultas(vectorBaseDeDatos); // Pasar el vector si la funcion lo necesita
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            default: cout<<"Error"<<endl;
            break;
        }
    }while(opcion!=6);
cout<<"Saliendo..."<<endl;
}