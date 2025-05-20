#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <sstream>
#include <iomanip>
#include <chrono>

#define BASE_DE_DATOS "C:/Codigos/ventas_sudamerica.csv"

using namespace std;
using namespace std::chrono;

// ======= ESTRUCTURAS Y DEFINICIONES =======
struct Venta {
    int idVenta{};
    string fecha;
    string pais;
    string ciudad;
    string cliente;
    string producto;
    string categoria;
    int cantidad{};
    double precioUnitario{};
    double montoTotal{};
    string medioEnvio;
    string estadoEnvio;
};

// ======= FUNCIONES AUXILIARES =======
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// ======= FUNCIONES DE CARGA CSV =======
void cargarVentas(vector<Venta>& ventas) {
    ifstream archivo(BASE_DE_DATOS);
    if (!archivo.is_open()) {
        cout << "Error al abrir el archivo." << endl;
        return;
    }

    string linea;
    getline(archivo, linea); // Saltar encabezado
    ventas.clear(); // Limpiar el vector por si tiene datos

    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string campo;
        Venta v;

        getline(ss, campo, ','); v.idVenta = stoi(campo);
        getline(ss, v.fecha, ',');

        getline(ss, v.pais, ',');
        v.pais = trim(v.pais);

        getline(ss, v.ciudad, ',');
        v.ciudad = trim(v.ciudad);

        getline(ss, v.cliente, ',');

        getline(ss, v.producto, ',');
        v.producto = trim(v.producto);

        getline(ss, v.categoria, ',');
        v.categoria = trim(v.categoria);

        getline(ss, campo, ','); v.cantidad = stoi(campo);
        getline(ss, campo, ','); v.precioUnitario = stod(campo);
        getline(ss, campo, ','); v.montoTotal = stod(campo);

        getline(ss, v.medioEnvio, ',');
        v.medioEnvio = trim(v.medioEnvio);

        getline(ss, v.estadoEnvio);
        v.estadoEnvio = trim(v.estadoEnvio);

        ventas.push_back(v);

    }
    int maxID = 0;
    for (const auto& v : ventas) {
        if (v.idVenta > maxID)
            maxID = v.idVenta;
    }
    cout << "El proximo ID sera: " << maxID + 1 << endl;
    archivo.close();
    cout << "Ventas cargadas correctamente. Total: " << ventas.size() << " registros." << endl;
}

// ======= FUNCIONES DE PROCESAMIENTO =======
void calcularTopCiudades(const vector<Venta>& ventas) {
    unordered_map<string, unordered_map<string, double>> ventasAcumuladas;
    for (size_t i = 0; i < ventas.size(); ++i) {
        ventasAcumuladas[ventas[i].pais][ventas[i].ciudad] += ventas[i].montoTotal;
    }

    for (unordered_map<string, unordered_map<string, double>>::const_iterator it = ventasAcumuladas.begin(); it != ventasAcumuladas.end(); ++it) {
        const string& pais = it->first;
        const unordered_map<string, double>& ciudades = it->second;
        vector<pair<string, double>> ciudadesOrdenadas;
        for (unordered_map<string, double>::const_iterator it2 = ciudades.begin(); it2 != ciudades.end(); ++it2) {
            ciudadesOrdenadas.push_back(make_pair(it2->first, it2->second));
        }
        sort(ciudadesOrdenadas.begin(), ciudadesOrdenadas.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
            return a.second > b.second;
        });
        cout << "----------------------------\n";
        cout << "Top 5 ciudades con mayor monto de ventas en " << pais << " (" << ciudadesOrdenadas.size() << " ciudades)\n";
        cout << "----------------------------\n";

        int limite = min(5, (int)ciudadesOrdenadas.size());
        cout << fixed << setprecision(2);
        for (int i = 0; i < limite; i++) {
            cout << i+1 << ". " << ciudadesOrdenadas[i].first << " - $" << ciudadesOrdenadas[i].second << endl;
        }
        cout << endl;
    }
}

void montoTotalPorProductoPorPais(const vector<Venta>& ventas) {
    unordered_map<string, unordered_map<string, double>> montoPorProductoPorPais;

    // Acumular ventas por país y producto
    for (size_t i = 0; i < ventas.size(); ++i) {
        montoPorProductoPorPais[ventas[i].pais][ventas[i].producto] += ventas[i].montoTotal;
    }

    cout << fixed << setprecision(2);
    for (unordered_map<string, unordered_map<string, double>>::const_iterator it = montoPorProductoPorPais.begin();
         it != montoPorProductoPorPais.end(); ++it) {

        const string& pais = it->first;
        const unordered_map<string, double>& productos = it->second;

        cout << "===============================\n";
        cout << "PAÍS: " << pais << endl;

        for (unordered_map<string, double>::const_iterator it2 = productos.begin(); it2 != productos.end(); ++it2) {
            cout << "- " << it2->first << ": $" << it2->second << endl;
        }
        cout << endl;
         }
}

void promedioVentasPorCategoriaPorPais(const vector<Venta>& ventas) {
    unordered_map<string, unordered_map<string, pair<double, int>>> datosPorCategoriaPorPais;

    // Acumular ventas y contar por país y categoría
    for (size_t i = 0; i < ventas.size(); ++i) {
        datosPorCategoriaPorPais[ventas[i].pais][ventas[i].categoria].first += ventas[i].montoTotal;
        datosPorCategoriaPorPais[ventas[i].pais][ventas[i].categoria].second++;
    }

    cout << fixed << setprecision(2);
    for (unordered_map<string, unordered_map<string, pair<double, int>>>::const_iterator it = datosPorCategoriaPorPais.begin();
         it != datosPorCategoriaPorPais.end(); ++it) {

        const string& pais = it->first;
        const unordered_map<string, pair<double, int>>& categorias = it->second;

        cout << "===============================\n";
        cout << "PAÍS: " << pais << endl;

        for (unordered_map<string, pair<double, int>>::const_iterator it2 = categorias.begin();
             it2 != categorias.end(); ++it2) {

            const string& categoria = it2->first;
            double suma = it2->second.first;
            int cantidad = it2->second.second;
            double promedio = cantidad > 0 ? suma / cantidad : 0;

            cout << "- Categoria: " << categoria << " - Promedio: $" << promedio << endl;
             }

        cout << endl;
         }
}

void medioEnvioMasUtilizadoPorPais(const vector<Venta>& ventas) {
    unordered_map<string, unordered_map<string, int>> conteoEnviosPorPais;

    // Contar medios de envío por país
    for (size_t i = 0; i < ventas.size(); ++i) {
        conteoEnviosPorPais[ventas[i].pais][ventas[i].medioEnvio]++;
    }

    for (unordered_map<string, unordered_map<string, int>>::const_iterator it = conteoEnviosPorPais.begin();
         it != conteoEnviosPorPais.end(); ++it) {

        const string& pais = it->first;
        const unordered_map<string, int>& envios = it->second;

        string medioMasUsado;
        int maxCantidad = 0;

        for (unordered_map<string, int>::const_iterator it2 = envios.begin(); it2 != envios.end(); ++it2) {
            if (it2->second > maxCantidad) {
                maxCantidad = it2->second;
                medioMasUsado = it2->first;
            }
        }

        cout << "PAÍS: " << pais << " Medio más usado: " << medioMasUsado << " (" << maxCantidad << " veces)" << endl;
         }
}

void medioEnvioMasUtilizadoPorCategoria(const vector<Venta>& ventas) {
    unordered_map<string, unordered_map<string, int>> conteoEnviosPorCategoria;

    // Contar medios de envío por categoría
    for (size_t i = 0; i < ventas.size(); ++i) {
        conteoEnviosPorCategoria[ventas[i].categoria][ventas[i].medioEnvio]++;
    }

    for (unordered_map<string, unordered_map<string, int>>::const_iterator it = conteoEnviosPorCategoria.begin();
         it != conteoEnviosPorCategoria.end(); ++it) {

        const string& categoria = it->first;
        const unordered_map<string, int>& envios = it->second;

        string medioMasUsado;
        int maxCantidad = 0;

        for (unordered_map<string, int>::const_iterator it2 = envios.begin(); it2 != envios.end(); ++it2) {
            if (it2->second > maxCantidad) {
                maxCantidad = it2->second;
                medioMasUsado = it2->first;
            }
        }

        cout << "CATEGORÍA: " << categoria << " Medio más usado: " << medioMasUsado << " (" << maxCantidad << " veces)" << endl;
         }
}

void diaConMasVentas(const vector<Venta>& ventas) {
    unordered_map<string, int> conteoPorFecha;

    // Contar ventas por fecha
    for (size_t i = 0; i < ventas.size(); ++i) {
        conteoPorFecha[ventas[i].fecha]++;
    }

    string fechaMax;
    int maxVentas = 0;

    for (unordered_map<string, int>::const_iterator it = conteoPorFecha.begin();
         it != conteoPorFecha.end(); ++it) {
        if (it->second > maxVentas) {
            maxVentas = it->second;
            fechaMax = it->first;
        }
         }

    if (!fechaMax.empty()) {
        cout << "Día con más ventas: " << fechaMax << " (" << maxVentas << " ventas)" << endl;
    } else {
        cout << "No hay ventas registradas." << endl;
    }
}

void estadoEnvioMasFrecuentePorPais(const vector<Venta>& ventas) {
    unordered_map<string, unordered_map<string, int>> estadosPorPais;

    // Contar estados de envío por país
    for (size_t i = 0; i < ventas.size(); ++i) {
        estadosPorPais[ventas[i].pais][ventas[i].estadoEnvio]++;
    }

    cout << "Estado de envío más frecuente por país:\n";

    for (unordered_map<string, unordered_map<string, int>>::const_iterator it = estadosPorPais.begin();
         it != estadosPorPais.end(); ++it) {

        const string& pais = it->first;
        const unordered_map<string, int>& estados = it->second;

        string estadoMasFrecuente;
        int maxCantidad = 0;

        for (unordered_map<string, int>::const_iterator it2 = estados.begin(); it2 != estados.end(); ++it2) {
            if (it2->second > maxCantidad) {
                maxCantidad = it2->second;
                estadoMasFrecuente = it2->first;
            }
        }

        cout << "- " << pais << ": " << estadoMasFrecuente << " (" << maxCantidad << " veces)" << endl;
         }
}

void productoMasVendido(const vector<Venta>& ventas) {
    unordered_map<string, int> cantidadPorProducto;

    // Acumular cantidades por producto
    for (size_t i = 0; i < ventas.size(); ++i) {
        cantidadPorProducto[ventas[i].producto] += ventas[i].cantidad;
    }

    string productoTop;
    int maxCantidad = 0;

    for (unordered_map<string, int>::const_iterator it = cantidadPorProducto.begin();
         it != cantidadPorProducto.end(); ++it) {
        if (it->second > maxCantidad) {
            maxCantidad = it->second;
            productoTop = it->first;
        }
    }

    cout << "Producto más vendido en cantidad total:\n";
    cout << "- " << productoTop << " (" << maxCantidad << " unidades vendidas)" << endl;
}

void productoMenosVendido(const vector<Venta>& ventas) {
    unordered_map<string, int> cantidadPorProducto;

    // Acumular cantidades por producto
    for (size_t i = 0; i < ventas.size(); ++i) {
        cantidadPorProducto[ventas[i].producto] += ventas[i].cantidad;
    }

    if (cantidadPorProducto.empty()) {
        cout << "No hay productos registrados." << endl;
        return;
    }

    string productoMin;
    int minCantidad = numeric_limits<int>::max();

    for (unordered_map<string, int>::const_iterator it = cantidadPorProducto.begin();
         it != cantidadPorProducto.end(); ++it) {
        if (it->second < minCantidad) {
            minCantidad = it->second;
            productoMin = it->first;
        }
         }

    cout << "Producto menos vendido en cantidad total:\n";
    cout << "- " << productoMin << " (" << minCantidad << " unidades vendidas)" << endl;
}

// ======= FUNCIONES DE MODIFICACIÓN =======
void agregarVenta(vector<Venta>& ventas) {
    // Implementación pendiente
    cout << "Función en desarrollo..." << endl;
    int maxID = 0;
    for (const auto& v : ventas) {
        if (v.idVenta > maxID)
            maxID = v.idVenta;
    }
    cout << "ID: " << maxID+1 << endl;
    cout<<"Fecha: "<<endl;
    cout<<"Seleccione su pais de origen: "<<endl;
    cout<<"1. Argentina"<<endl;
    cout<<"2. Brasil"<<endl;
    cout<<"3. Chile"<<endl;
    cout<<"4. Colombia"<<endl;
    cout<<"5. Ecuador"<<endl;
    cout<<"6. Peru"<<endl;
    cout<<"7. Uruguay"<<endl;
    cout<<"8. Venezuela"<<endl;
    cout<<"9. Otro"<<endl;
    int opcion;
    cin>>opcion;
    cout<<"Ingrese su ciudad"<<endl;
    string ciudad;
    getline(cin,ciudad); //getline por si ingresa espacios
    cout<<"Ingrese su nombre y apellido"<<endl;
    string nombre;
    getline(cin,nombre);
    cout<<"Seleccione el producto que desea comprar: "<<endl;
    cout<<"1. Auriculares"<<endl;
    cout<<"2. Celular"<<endl;
    cout<<"3. Cámara"<<endl;
    cout<<"4. Escritorio"<<endl;
    cout<<"5. Impresora"<<endl;
    cout<<"6. Laptop"<<endl;
    cout<<"7. Monitor"<<endl;
    cout<<"8. Silla ergonómica"<<endl;
    cout<<"9. Tablet"<<endl;
    cout<<"10. Teclado"<<endl;
    int opcion2;
    cin>>opcion2;
    //switch para que dependiendo el producto asigne automaticamente la categoria
    cout<<"Ingrese la cantidad que desea comprar del producto: "<<endl;
    int cantidad;
    cin>>cantidad;
    //asignar el precio automaticamente en base a la lista de precios (crear una)
    int precio=0;

    cout<<"El monto total a pagar es de: "<<precio*cantidad<<endl;
    cout<<"Seleccione el medio de envio que prefiera: "<<endl;
    int medioEnvio;
    cout<<"1. Aéreo"<<endl;
    cout<<"2. Marítimo"<<endl;
    cout<<"3. Terreste"<<endl;
    cin>>medioEnvio;

}

void modificarVenta(vector<Venta>& ventas) {
    // Implementación pendiente
    cout << "Función en desarrollo..." << endl;
}

void eliminarVenta(vector<Venta>& ventas) {
    // Implementación pendiente
    cout << "Función en desarrollo..." << endl;
}

// ======= FUNCIONES DE CONSULTAS =======
void consultasDinamicas(const vector<Venta>& ventas) {
    // Implementación pendiente
    cout << "Función en desarrollo..." << endl;
}

// ======= FUNCIONES DE SUBMENU =======
void menuResumenVentas(const vector<Venta>& ventas) {
    int opcionResumen;
    do {
        cout << "\n--- Resumen de Ventas ---\n";
        cout << "1. Top 5 ciudades con mayor monto por pais\n";
        cout << "2. Monto total vendido por producto por pais\n";
        cout << "3. Promedio de ventas por categoria por pais\n";
        cout << "4. Medio de envio mas utilizado por pais\n";
        cout << "5. Medio de envio mas utilizado por categoria\n";
        cout << "6. Dia con mayor cantidad de ventas\n";
        cout << "7. Estado de envio mas frecuente por pais\n";
        cout << "8. Producto mas vendido en cantidad total\n";
        cout << "9. Producto menos vendido en cantidad total\n";
        cout << "10. Volver al menu principal\n";
        cout << "Ingrese una opcion: ";
        cin >> opcionResumen;

        switch (opcionResumen) {
            case 1: {
                auto inicio = steady_clock::now();
                calcularTopCiudades(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 2: {
                auto inicio = steady_clock::now();
                montoTotalPorProductoPorPais(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 3: {
                auto inicio = steady_clock::now();
                promedioVentasPorCategoriaPorPais(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 4: {
                auto inicio = steady_clock::now();
                medioEnvioMasUtilizadoPorPais(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 5: {
                auto inicio = steady_clock::now();
                medioEnvioMasUtilizadoPorCategoria(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 6: {
                auto inicio = steady_clock::now();
                diaConMasVentas(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 7: {
                auto inicio = steady_clock::now();
                estadoEnvioMasFrecuentePorPais(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 8: {
                auto inicio = steady_clock::now();
                productoMasVendido(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 9: {
                auto inicio = steady_clock::now();
                productoMenosVendido(ventas);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                break;
            }
            case 10:
                cout << "Volviendo al menu principal...\n";
                break;
            default:
                cout << "Opcion invalida. Intente de nuevo.\n";
        }

    } while (opcionResumen != 10);
}

// ======= FUNCIÓN MAIN =======
int main() {
    vector<Venta> ventas;
    int opcion;

    do {
        cout << "\n====== MENU ======" << endl;
        cout << "1. Cargar datos" << endl;
        cout << "2. Modificar una venta" << endl;
        cout << "3. Agregar una venta" << endl;
        cout << "4. Eliminar una venta" << endl;
        cout << "5. Consultas dinámicas" << endl;
        cout << "6. Mostrar resumen de ventas" << endl;
        cout << "7. Salir" << endl;
        cout << "Ingrese una opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                cout << "Procesando base de datos, espere un momento..." << endl;
                cargarVentas(ventas);
                break;
            case 2:
                modificarVenta(ventas);
                break;
            case 3:
                agregarVenta(ventas);
                break;
            case 4:
                eliminarVenta(ventas);
                break;
            case 5:
                consultasDinamicas(ventas);
                break;
            case 6:
                if (ventas.empty()) {
                    cout << "Primero debe cargar los datos (opción 1)." << endl;
                } else {
                    menuResumenVentas(ventas);
                }
                break;
            case 7:
                cout << "Saliendo..." << endl;
                break;
            default:
                cout << "Opcion incorrecta, intente nuevamente." << endl;
                break;
        }
    } while (opcion != 7);

    return 0;
}