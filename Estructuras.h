//
// Created by aroma on 5/6/2025.
//

#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H
#include <string>
#include <unordered_map>
using namespace std;


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
    // --- Operators for ArbolBinarioAVL ---

    // Operador menor que (<)
    bool operator<(const Venta& other) const {
        return idVenta < other.idVenta;
    }

    // Operador de igualdad (==)
    bool operator==(const Venta& other) const {
        return idVenta == other.idVenta;
    }

    // Operador mayor que (>)
    // Puedes definirlo en terminos de <, o directamente.
    bool operator>(const Venta& other) const {
        return idVenta > other.idVenta;
    }

    // Operador no igual (!=) - A menudo util, se puede definir en terminos de ==
    bool operator!=(const Venta& other) const {
        return !(*this == other); // o return idVenta != other.idVenta;
    }

    // Operador menor o igual (<=) - Si es necesario para tu arbol
    bool operator<=(const Venta& other) const {
        return idVenta <= other.idVenta; // o !(*this > other)
    }

    // Operador mayor o igual (>=) - Si es necesario para tu arbol
    bool operator>=(const Venta& other) const {
        return idVenta >= other.idVenta; // o !(*this < other)
    }
};

// Struct para guardar las estadisticas finales de forma agregada
struct EstadisticasAgregadas {
    double montoTotalVendido = 0.0;
    int cantidadTotalVendida = 0;
    // Usamos otro unordered_map para contar eficientemente los medios de envio
    unordered_map<string, int> conteoMediosEnvio;
};

// Definimos alias para nuestros tipos de mapas anidados para que el codigo sea mas legible
using MapaProductos = unordered_map<string, EstadisticasAgregadas>;
using MapaPaises = unordered_map<string, MapaProductos>;

#endif //ESTRUCTURAS_H
