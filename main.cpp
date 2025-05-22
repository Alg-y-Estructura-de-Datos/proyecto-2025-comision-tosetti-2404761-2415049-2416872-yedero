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
#include <ctime>

// Asegúrate de que esta ruta sea correcta para tu entorno.
#define BASE_DE_DATOS "../ventas_sudamerica.csv"

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

// Variable global para almacenar el encabezado del CSV.
// Es una solución simple para este caso; en una aplicación más grande,
// se podría pasar como parámetro o ser parte de una clase de gestión de datos.
string csvHeader = "idVenta,fecha,pais,ciudad,cliente,producto,categoria,cantidad,precioUnitario,montoTotal,medioEnvio,estadoEnvio";


// ======= FUNCIONES AUXILIARES =======
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

string obtenerFechaActual() {
    auto now = system_clock::now();
    time_t now_c = system_clock::to_time_t(now);
    tm* ltm = localtime(&now_c); // Usar localtime_s en Windows o gmtime_r en sistemas POSIX para seguridad de hilos

    stringstream ss;
    ss << put_time(ltm, "%Y-%m-%d"); // Formato AAAA-MM-DD
    return ss.str();
}

// ======= FUNCIONES DE CARGA CSV =======
void cargarVentas(vector<Venta>& ventas) {
    ifstream archivo(BASE_DE_DATOS);
    if (!archivo.is_open()) {
        // En lugar de salir, podrías crear el archivo si no existe
        ofstream crearArchivo(BASE_DE_DATOS);
        if (crearArchivo.is_open()) {
            cout << "El archivo de base de datos no existía y ha sido creado." << endl;
            // Escribe el encabezado en el archivo recién creado
            crearArchivo << csvHeader << endl;
            crearArchivo.close();
            ventas.clear(); // El vector está vacío, ya que el archivo estaba vacío
            return;
        } else {
            cout << "Error CRÍTICO: No se pudo abrir ni crear el archivo de base de datos." << endl;
            return; // No se puede continuar sin el archivo
        }
    }

    string linea;
    // Leer la primera línea (encabezado) y guardarla, pero no procesarla como datos.
    // Si el archivo está vacío, getline devolverá falso, y el encabezado global se mantiene.
    if (getline(archivo, linea)) {
        // Opcional: podrías verificar si 'linea' coincide con tu 'csvHeader' esperado.
        // Pero para este problema, solo necesitamos asegurarnos de que se lea y se salte.
        // Asignamos a csvHeader si es que existe en el archivo
        csvHeader = linea;
    }

    ventas.clear(); // Limpiar el vector antes de cargar nuevas ventas

    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string campo;
        Venta v;

        // Es importante verificar que se pueda leer cada campo.
        // Si una línea está mal formateada (menos campos), esto evitará un crash.
        try {
            if (!getline(ss, campo, ',')) continue; v.idVenta = stoi(campo);
            if (!getline(ss, v.fecha, ',')) continue;
            if (!getline(ss, v.pais, ',')) continue; v.pais = trim(v.pais);
            if (!getline(ss, v.ciudad, ',')) continue; v.ciudad = trim(v.ciudad);
            if (!getline(ss, v.cliente, ',')) continue;
            if (!getline(ss, v.producto, ',')) continue; v.producto = trim(v.producto);
            if (!getline(ss, v.categoria, ',')) continue; v.categoria = trim(v.categoria);
            if (!getline(ss, campo, ',')) continue; v.cantidad = stoi(campo);
            if (!getline(ss, campo, ',')) continue; v.precioUnitario = stod(campo);
            if (!getline(ss, campo, ',')) continue; v.montoTotal = stod(campo);
            if (!getline(ss, v.medioEnvio, ',')) continue; v.medioEnvio = trim(v.medioEnvio);
            if (!getline(ss, v.estadoEnvio)) continue; v.estadoEnvio = trim(v.estadoEnvio);

            ventas.push_back(v);
        } catch (const exception& e) {
            cout << "Advertencia: Salto una línea mal formateada en el CSV: " << linea << " (" << e.what() << ")" << endl;
            continue; // Saltar a la siguiente línea si hay un error de parseo
        }
    }
    archivo.close();
    cout << "Ventas cargadas correctamente. Total: " << ventas.size() << " registros." << endl;
}

// ======= FUNCIONES DE MODIFICACIÓN =======
void agregarVenta(vector<Venta>& ventas, Venta v) {
    // Para asegurar que el archivo tenga el encabezado al agregar la primera venta
    // o si el archivo fue borrado/corrupto.
    ofstream archivo(BASE_DE_DATOS, ios::out | ios::trunc); // Abrir en modo truncado para reescribir si está vacío
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo para escritura." << endl;
        return;
    }

    // Escribe el encabezado cada vez que se abre el archivo para escritura (importante)
    archivo << csvHeader << endl;

    // Vuelve a agregar todas las ventas existentes en el vector
    for (const auto& ventaExistente : ventas) {
        archivo << ventaExistente.idVenta << ","
                << ventaExistente.fecha << ","
                << ventaExistente.pais << ","
                << ventaExistente.ciudad << ","
                << ventaExistente.cliente << ","
                << ventaExistente.producto << ","
                << ventaExistente.categoria << ","
                << ventaExistente.cantidad << ","
                << fixed << setprecision(2) << ventaExistente.precioUnitario << ","
                << fixed << setprecision(2) << ventaExistente.montoTotal << ","
                << ventaExistente.medioEnvio << ","
                << ventaExistente.estadoEnvio << endl;
    }

    int maxID = 0;
    if (!ventas.empty()) {
        for (const auto& ventaExistente : ventas) {
            if (ventaExistente.idVenta > maxID)
                maxID = ventaExistente.idVenta;
        }
    }
    v.idVenta = maxID + 1;
    cout << "ID asignado: " << v.idVenta << endl;

    v.fecha = obtenerFechaActual();
    cout << "Fecha: " << v.fecha << endl;

    // --- Lógica de entrada de datos para la nueva venta (país, ciudad, cliente, producto, etc.) ---
    int opcion = 0;
    string entrada;
    bool entradaValida = false;

    do {
        cout << "Seleccione su pais de origen: " << endl;
        cout << "1. Argentina\n2. Brasil\n3. Chile\n4. Colombia\n5. Ecuador\n6. Peru\n7. Uruguay\n8. Venezuela" << endl;
        cout << "Opcion: ";
        cin >> entrada;

        try {
            opcion = stoi(entrada);
            if (opcion < 1 || opcion > 8) {
                cout << "Opcion fuera de rango. Intente nuevamente.\n";
            } else {
                entradaValida = true;
            }
        } catch (const exception& e) {
            cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

    } while (!entradaValida);

    switch (opcion) {
        case 1: v.pais = "Argentina"; break;
        case 2: v.pais = "Brasil"; break;
        case 3: v.pais = "Chile"; break;
        case 4: v.pais = "Colombia"; break;
        case 5: v.pais = "Ecuador"; break;
        case 6: v.pais = "Peru"; break;
        case 7: v.pais = "Uruguay"; break;
        case 8: v.pais = "Venezuela"; break;
        default: ; // Debería ser inalcanzable debido a la validación.
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout<<"Ingrese su ciudad"<<endl;
    getline(cin,v.ciudad);

    cout<<"Ingrese su nombre y apellido"<<endl;
    getline(cin,v.cliente);

    int opcion2=0;
    string entrada2;
    bool entrada2Valida = false;

    do{
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
        cin>>entrada2;
        try {
            opcion2 = stoi(entrada2);
            if (opcion2 < 1 || opcion2 > 10) {
                cout << "Opcion fuera de rango. Intente nuevamente.\n";
            } else {
                entrada2Valida = true;
            }
        }
        catch (const exception& e) {
        cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (!entrada2Valida);

    switch (opcion2) {
        case 1: v.producto="Auriculares"; v.categoria="Accesorios"; break;
        case 2: v.producto="Celular"; v.categoria="Electronica"; break;
        case 3: v.producto="Cámara"; v.categoria="Electronica"; break;
        case 4: v.producto="Escritorio"; v.categoria="Muebles"; break;
        case 5: v.producto="Impresora"; v.categoria="Oficina"; break;
        case 6: v.producto="Laptop"; v.categoria="Electronica"; break;
        case 7: v.producto="Monitor"; v.categoria="Electronica"; break;
        case 8: v.producto="Silla ergonómica"; v.categoria="Muebles"; break;
        case 9: v.producto="Tablet"; v.categoria="Electronica"; break;
        case 10: v.producto="Teclado"; v.categoria="Accesorios"; break;
        default: ; // Inalcanzable
    }

    string cantidadStr;
    bool cantidadValida = false;
    do {
        cout<<"Ingrese la cantidad a comprar: ";
        cin>>cantidadStr;
        try {
            v.cantidad=stoi(cantidadStr);
            if (v.cantidad <= 0) {
                cout << "La cantidad debe ser mayor que cero.\n";
            } else {
                cantidadValida = true;
            }
        }
        catch (const exception& e) {
            cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }while (!cantidadValida);

    string entradaPrecio;
    bool precioValido = false;

    do {
        cout << "Ingrese el precio del producto: ";
        cin >> entradaPrecio;

        try {
            v.precioUnitario = stod(entradaPrecio);
            if (v.precioUnitario <= 0) {
                cout << "El precio debe ser mayor que cero.\n";
            } else {
                precioValido = true;
            }
        } catch (const exception& e) {
            cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (!precioValido);

    v.montoTotal = v.cantidad * v.precioUnitario;
    cout << "El monto total a pagar es de: " << fixed << setprecision(2) << v.montoTotal << endl;

    int medioEnvio = 0;
    string entradaMedio;
    bool medioValido = false;

    do {
        cout << "Seleccione el medio de envio:\n";
        cout << "1. Terrestre\n2. Maritimo\n3. Aereo\n";
        cout << "Opcion: ";
        cin >> entradaMedio;

        try {
            medioEnvio = stoi(entradaMedio);

            if (medioEnvio < 1 || medioEnvio > 3) {
                cout << "Opcion fuera de rango. Intente nuevamente.\n";
            } else {
                medioValido = true;
            }
        } catch (const exception& e) {
            cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (!medioValido);

    switch (medioEnvio) {
        case 1: v.medioEnvio = "Terrestre"; break;
        case 2: v.medioEnvio = "Marítimo"; break;
        case 3: v.medioEnvio = "Aéreo"; break;
        default: ; // Inalcanzable
    }

    v.estadoEnvio="Pendiente";

    // Escribe la nueva venta al final del archivo CSV.
    // NOTA: Con la corrección de reescribir todo el archivo, esta parte ya no es
    // estrictamente "al final" del archivo después de abrir con ios::app.
    // Ahora, agregamos 'v' al vector y luego reescribimos todo el vector.
    // El archivo se abre en truncado al principio de la función, se escribe el encabezado,
    // se escriben las ventas anteriores, y luego se agregará esta nueva venta al vector.

    // ¡IMPORTANTE! Agrega la nueva venta también al vector en memoria.
    ventas.push_back(v);

    // Ahora reescribe todo el archivo con la nueva venta incluida
    // Esto es crucial para que los cambios persistan y el encabezado se mantenga.
    ofstream reescrituraArchivo(BASE_DE_DATOS, ios::out | ios::trunc); // Abrir en modo truncado
    if (!reescrituraArchivo.is_open()) {
        cout << "Error CRÍTICO: No se pudo reabrir el archivo para reescritura. Los cambios no se guardaron permanentemente." << endl;
        return;
    }
    reescrituraArchivo << csvHeader << endl; // Vuelve a escribir el encabezado
    for (const auto& ventaActual : ventas) {
        reescrituraArchivo << ventaActual.idVenta << ","
                           << ventaActual.fecha << ","
                           << ventaActual.pais << ","
                           << ventaActual.ciudad << ","
                           << ventaActual.cliente << ","
                           << ventaActual.producto << ","
                           << ventaActual.categoria << ","
                           << ventaActual.cantidad << ","
                           << fixed << setprecision(2) << ventaActual.precioUnitario << ","
                           << fixed << setprecision(2) << ventaActual.montoTotal << ","
                           << ventaActual.medioEnvio << ","
                           << ventaActual.estadoEnvio << endl;
    }
    reescrituraArchivo.close();
    cout << "Venta guardada correctamente en el archivo CSV y en memoria." << endl;
}


void modificarVenta(std::vector<Venta>& bdVector) {
    // 1. Crear un mapa hash para acceso rápido.
    std::unordered_map<int, Venta*> bdHashMap;
    for (Venta& v : bdVector) {
        bdHashMap[v.idVenta] = &v;
    }

    // 2. Solicitar el ID de la venta a modificar.
    int idModificar;
    std::string entradaID;
    bool idValido = false;

    do {
        std::cout << "Ingrese el ID de la venta que desea modificar: ";
        std::cin >> entradaID;
        try {
            idModificar = std::stoi(entradaID);
            if (bdHashMap.count(idModificar)) {
                idValido = true;
            } else {
                std::cout << "ID de venta no encontrado. Intente nuevamente.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (!idValido);

    Venta* ventaAModificar = bdHashMap[idModificar];

    // 3. Mostrar los detalles de la venta seleccionada.
    std::cout << "\n--- Venta seleccionada para modificar ---\n";
    std::cout << "ID: " << ventaAModificar->idVenta << std::endl;
    std::cout << "Fecha: " << ventaAModificar->fecha << std::endl;
    std::cout << "País: " << ventaAModificar->pais << std::endl;
    std::cout << "Ciudad: " << ventaAModificar->ciudad << std::endl;
    std::cout << "Cliente: " << ventaAModificar->cliente << std::endl;
    std::cout << "Producto: " << ventaAModificar->producto << std::endl;
    std::cout << "Categoría: " << ventaAModificar->categoria << std::endl;
    std::cout << "Cantidad: " << ventaAModificar->cantidad << std::endl;
    std::cout << "Precio unitario: " << std::fixed << std::setprecision(2) << ventaAModificar->precioUnitario << std::endl;
    std::cout << "Monto total: " << std::fixed << std::setprecision(2) << ventaAModificar->montoTotal << std::endl;
    std::cout << "Medio de envío: " << ventaAModificar->medioEnvio << std::endl;
    std::cout << "Estado de envío: " << ventaAModificar->estadoEnvio << std::endl;
    std::cout << "--------------------------------------\n";

    // 4. Solicitar el campo a modificar.
    int opcionCampo;
    std::string entradaCampo;
    bool campoValido = false;

    do {
        std::cout << "\n¿Qué campo desea modificar?\n";
        std::cout << "1. País\n";
        std::cout << "2. Ciudad\n";
        std::cout << "3. Cliente\n";
        std::cout << "4. Producto\n";
        std::cout << "5. Cantidad\n";
        std::cout << "6. Precio Unitario\n";
        std::cout << "7. Medio de Envío\n";
        std::cout << "8. Estado de Envío\n";
        std::cout << "Opción: ";
        std::cin >> entradaCampo;

        try {
            opcionCampo = std::stoi(entradaCampo);
            if (opcionCampo >= 1 && opcionCampo <= 8) {
                campoValido = true;
            } else {
                std::cout << "Opción fuera de rango. Intente nuevamente.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (!campoValido);

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 5. Aplicar la modificación según el campo seleccionado.
    switch (opcionCampo) {
        case 1: { // Modificar País
            int opcionPais = 0;
            std::string entradaPais;
            bool paisValido = false;
            do {
                std::cout << "Seleccione el nuevo país:\n";
                std::cout << "1. Argentina\n2. Brasil\n3. Chile\n4. Colombia\n5. Ecuador\n6. Peru\n7. Uruguay\n8. Venezuela\n";
                std::cout << "Opción: ";
                std::cin >> entradaPais;
                try {
                    opcionPais = std::stoi(entradaPais);
                    if (opcionPais >= 1 && opcionPais <= 8) {
                        paisValido = true;
                    } else {
                        std::cout << "Opción fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } while (!paisValido);
            switch (opcionPais) {
                case 1: ventaAModificar->pais = "Argentina"; break;
                case 2: ventaAModificar->pais = "Brasil"; break;
                case 3: ventaAModificar->pais = "Chile"; break;
                case 4: ventaAModificar->pais = "Colombia"; break;
                case 5: ventaAModificar->pais = "Ecuador"; break;
                case 6: ventaAModificar->pais = "Peru"; break;
                case 7: ventaAModificar->pais = "Uruguay"; break;
                case 8: ventaAModificar->pais = "Venezuela"; break;
            }
            std::cout << "País modificado correctamente.\n";
            break;
        }
        case 2: { // Modificar Ciudad
            std::cout << "Ingrese la nueva ciudad: ";
            std::getline(std::cin, ventaAModificar->ciudad);
            std::cout << "Ciudad modificada correctamente.\n";
            break;
        }
        case 3: { // Modificar Cliente
            std::cout << "Ingrese el nuevo nombre y apellido del cliente: ";
            std::getline(std::cin, ventaAModificar->cliente);
            std::cout << "Cliente modificado correctamente.\n";
            break;
        }
        case 4: { // Modificar Producto y Categoría
            int opcionProducto = 0;
            std::string entradaProducto;
            bool productoValido = false;
            do {
                std::cout << "Seleccione el nuevo producto:\n";
                std::cout << "1. Auriculares\n2. Celular\n3. Cámara\n4. Escritorio\n5. Impresora\n6. Laptop\n7. Monitor\n8. Silla ergonómica\n9. Tablet\n10. Teclado\n";
                std::cout << "Opción: ";
                std::cin >> entradaProducto;
                try {
                    opcionProducto = std::stoi(entradaProducto);
                    if (opcionProducto >= 1 && opcionProducto <= 10) {
                        productoValido = true;
                    } else {
                        std::cout << "Opción fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } while (!productoValido);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch (opcionProducto) {
                case 1: ventaAModificar->producto = "Auriculares"; ventaAModificar->categoria = "Accesorios"; break;
                case 2: ventaAModificar->producto = "Celular"; ventaAModificar->categoria = "Electronica"; break;
                case 3: ventaAModificar->producto = "Cámara"; ventaAModificar->categoria = "Electronica"; break;
                case 4: ventaAModificar->producto = "Escritorio"; ventaAModificar->categoria = "Muebles"; break;
                case 5: ventaAModificar->producto = "Impresora"; ventaAModificar->categoria = "Oficina"; break;
                case 6: ventaAModificar->producto = "Laptop"; ventaAModificar->categoria = "Electronica"; break;
                case 7: ventaAModificar->producto = "Monitor"; ventaAModificar->categoria = "Electronica"; break;
                case 8: ventaAModificar->producto = "Silla ergonómica"; ventaAModificar->categoria = "Muebles"; break;
                case 9: ventaAModificar->producto = "Tablet"; ventaAModificar->categoria = "Electronica"; break;
                case 10: ventaAModificar->producto = "Teclado"; ventaAModificar->categoria = "Accesorios"; break;
            }
            std::cout << "Producto y categoría modificados correctamente.\n";
            ventaAModificar->montoTotal = ventaAModificar->cantidad * ventaAModificar->precioUnitario;
            std::cout << "Nuevo monto total: " << std::fixed << std::setprecision(2) << ventaAModificar->montoTotal << std::endl;
            break;
        }
        case 5: { // Modificar Cantidad
            std::string nuevaCantidadStr;
            bool cantidadValida = false;
            do {
                std::cout << "Ingrese la nueva cantidad: ";
                std::cin >> nuevaCantidadStr;
                try {
                    int nuevaCantidad = std::stoi(nuevaCantidadStr);
                    if (nuevaCantidad > 0) {
                        ventaAModificar->cantidad = nuevaCantidad;
                        cantidadValida = true;
                    } else {
                        std::cout << "La cantidad debe ser mayor que cero.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } while (!cantidadValida);
            ventaAModificar->montoTotal = ventaAModificar->cantidad * ventaAModificar->precioUnitario;
            std::cout << "Cantidad modificada correctamente. Nuevo monto total: " << std::fixed << std::setprecision(2) << ventaAModificar->montoTotal << std::endl;
            break;
        }
        case 6: { // Modificar Precio Unitario
            std::string nuevoPrecioStr;
            bool precioValido = false;
            do {
                std::cout << "Ingrese el nuevo precio unitario: ";
                std::cin >> nuevoPrecioStr;
                try {
                    double nuevoPrecio = std::stod(nuevoPrecioStr);
                    if (nuevoPrecio > 0) {
                        ventaAModificar->precioUnitario = nuevoPrecio;
                        precioValido = true;
                    } else {
                        std::cout << "El precio debe ser mayor que cero.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } while (!precioValido);
            ventaAModificar->montoTotal = ventaAModificar->cantidad * ventaAModificar->precioUnitario;
            std::cout << "Precio unitario modificado correctamente. Nuevo monto total: " << std::fixed << std::setprecision(2) << ventaAModificar->montoTotal << std::endl;
            break;
        }
        case 7: { // Modificar Medio de Envío
            int opcionMedio = 0;
            std::string entradaMedio;
            bool medioValido = false;
            do {
                std::cout << "Seleccione el nuevo medio de envío:\n";
                std::cout << "1. Terrestre\n2. Marítimo\n3. Aéreo\n";
                std::cout << "Opción: ";
                std::cin >> entradaMedio;
                try {
                    opcionMedio = std::stoi(entradaMedio);
                    if (opcionMedio >= 1 && opcionMedio <= 3) {
                        medioValido = true;
                    } else {
                        std::cout << "Opción fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } while (!medioValido);
            switch (opcionMedio) {
                case 1: ventaAModificar->medioEnvio = "Terrestre"; break;
                case 2: ventaAModificar->medioEnvio = "Marítimo"; break;
                case 3: ventaAModificar->medioEnvio = "Aéreo"; break;
            }
            std::cout << "Medio de envío modificado correctamente.\n";
            break;
        }
        case 8: { // Modificar Estado de Envío
            int opcionEstado = 0;
            std::string entradaEstado;
            bool estadoValido = false;
            do {
                std::cout << "Seleccione el nuevo estado de envío:\n";
                std::cout << "1. Pendiente\n2. En Proceso\n3. Entregado\n4. Cancelado\n";
                std::cout << "Opción: ";
                std::cin >> entradaEstado;
                try {
                    opcionEstado = std::stoi(entradaEstado);
                    if (opcionEstado >= 1 && opcionEstado <= 4) {
                        estadoValido = true;
                    } else {
                        std::cout << "Opción fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Entrada inválida: " << e.what() << ". Intente nuevamente.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } while (!estadoValido);
            switch (opcionEstado) {
                case 1: ventaAModificar->estadoEnvio = "Pendiente"; break;
                case 2: ventaAModificar->estadoEnvio = "En Proceso"; break;
                case 3: ventaAModificar->estadoEnvio = "Entregado"; break;
                case 4: ventaAModificar->estadoEnvio = "Cancelado"; break;
            }
            std::cout << "Estado de envío modificado correctamente.\n";
            break;
        }
        default:
            std::cout << "Opción de campo inválida.\n";
            break;
    }

    // 6. REESCRITURA COMPLETA DEL ARCHIVO CSV.
    // Abre el archivo en modo de truncado (BORRA el contenido existente).
    std::ofstream archivo(BASE_DE_DATOS, ios::out | ios::trunc);
    if (!archivo.is_open()) {
        std::cout << "ERROR CRÍTICO: No se pudo abrir el archivo para escritura y actualización. Los cambios NO se guardaron permanentemente.\n";
        return;
    }

    archivo << csvHeader << endl; // Escribe el encabezado ANTES de los datos.

    for (const auto& v : bdVector) {
        archivo << v.idVenta << ",";
        archivo << v.fecha << ",";
        archivo << v.pais << ",";
        archivo << v.ciudad << ",";
        archivo << v.cliente << ",";
        archivo << v.producto << ",";
        archivo << v.categoria << ",";
        archivo << v.cantidad << ",";
        archivo << std::fixed << std::setprecision(2) << v.precioUnitario << ",";
        archivo << std::fixed << std::setprecision(2) << v.montoTotal << ",";
        archivo << v.medioEnvio << ",";
        archivo << v.estadoEnvio << std::endl;
    }
    archivo.close();
    std::cout << "Venta modificada y archivo CSV actualizado correctamente.\n";
}

void eliminarVenta(vector<Venta>& ventas) {
    cout << "Ingrese el ID de la venta a eliminar: ";
    string idEliminarStr;
    cin >> idEliminarStr;

    int idEliminar;
    try {
        idEliminar = stoi(idEliminarStr);
    } catch (const exception& e) {
        cout << "ID inválido. Intente nuevamente." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    // Encuentra la venta por ID
    auto it = remove_if(ventas.begin(), ventas.end(), [idEliminar](const Venta& v) {
        return v.idVenta == idEliminar;
    });

    if (it == ventas.end()) {
        cout << "Venta con ID " << idEliminar << " no encontrada." << endl;
    } else {
        ventas.erase(it, ventas.end());
        cout << "Venta con ID " << idEliminar << " eliminada de la memoria." << endl;

        // Reescribir todo el archivo CSV para reflejar la eliminación
        ofstream archivo(BASE_DE_DATOS, ios::out | ios::trunc);
        if (!archivo.is_open()) {
            cout << "ERROR CRÍTICO: No se pudo abrir el archivo para reescritura después de eliminar. Los cambios NO se guardaron permanentemente." << endl;
            return;
        }

        archivo << csvHeader << endl; // Escribe el encabezado

        for (const auto& v : ventas) {
            archivo << v.idVenta << ","
                    << v.fecha << ","
                    << v.pais << ","
                    << v.ciudad << ","
                    << v.cliente << ","
                    << v.producto << ","
                    << v.categoria << ","
                    << v.cantidad << ","
                    << fixed << setprecision(2) << v.precioUnitario << ","
                    << fixed << setprecision(2) << v.montoTotal << ","
                    << v.medioEnvio << ","
                    << v.estadoEnvio << endl;
        }
        archivo.close();
        cout << "Archivo CSV actualizado: venta eliminada correctamente." << endl;
    }
}


// ======= FUNCIONES DE CONSULTAS (asumidas existentes) =======
// Estas funciones no las modifico, solo las dejo como declaración si no están en tu original
// o si están, mantendrán su implementación.
void calcularTopCiudades(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Top Ciudades." << endl; }
void montoTotalPorProductoPorPais(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Monto Total por Producto por Pais." << endl; }
void promedioVentasPorCategoriaPorPais(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Promedio de ventas por categoria por pais." << endl; }
void medioEnvioMasUtilizadoPorPais(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Medio de envio mas utilizado por pais." << endl; }
void medioEnvioMasUtilizadoPorCategoria(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Medio de envio mas utilizado por categoria." << endl; }
void diaConMasVentas(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Dia con mayor cantidad de ventas." << endl; }
void estadoEnvioMasFrecuentePorPais(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Estado de envio mas frecuente por pais." << endl; }
void productoMasVendido(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Producto mas vendido en cantidad total." << endl; }
void productoMenosVendido(const vector<Venta>& ventas) { /* ... tu implementación ... */ cout << "Implementacion de Producto menos vendido en cantidad total." << endl; }

void consultasDinamicas(const vector<Venta>& ventas) {
    cout << "Función de consultas dinámicas en desarrollo..." << endl;
}

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

        // Limpiar el buffer después de leer un entero
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

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

// ======= FUNCIÓN MAIN CORREGIDA =======
int main() {
    vector<Venta> ventas;
    Venta v; // Esta 'v' se usa para la nueva venta en agregarVenta, no es la base de datos principal

    cout << "Iniciando sistema de gestión de ventas..." << endl;

    // *** Cargar TODAS las ventas solo UNA VEZ al inicio del programa ***
    // Esto asegura que 'ventas' siempre tenga el estado completo del CSV.
    cargarVentas(ventas);

    int opcion;

    do {
        cout << "\n====== MENU PRINCIPAL ======" << endl;
        // La opción 1 ya no es "Cargar datos" en el sentido de iniciar, sino para recargar manualmente.
        cout << "1. Recargar datos (solo si el archivo externo ha cambiado sin reiniciar el programa)" << endl;
        cout << "2. Modificar una venta" << endl;
        cout << "3. Agregar una venta" << endl;
        cout << "4. Eliminar una venta" << endl;
        cout << "5. Consultas dinámicas" << endl;
        cout << "6. Mostrar resumen de ventas" << endl;
        cout << "7. Salir" << endl;
        cout << "Ingrese una opcion: ";

        // Leer la opción de forma segura
        string opcionStr;
        cin >> opcionStr;
        try {
            opcion = stoi(opcionStr);
        } catch (const exception& e) {
            cout << "Entrada inválida. Por favor, ingrese un número." << endl;
            opcion = 0; // Para que el bucle continúe y pida la opción de nuevo
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue; // Saltar al siguiente ciclo del bucle
        }

        // Limpiar el buffer después de leer un entero con cin >>
        cin.ignore(numeric_limits<streamsize>::max(), '\n');


        switch (opcion) {
            case 1:
                cout << "Recargando datos desde el archivo..." << endl;
                cargarVentas(ventas); // Permite recargar el vector desde el CSV si el archivo fue modificado externamente
                break;
            case 2:
                if (ventas.empty()) {
                    cout << "No hay ventas para modificar. Agregue una venta primero." << endl;
                } else {
                    modificarVenta(ventas); // `ventas` contiene la base de datos completa
                }
                break;
            case 3:
                // `agregarVenta` ahora manejará la reescritura completa del archivo
                agregarVenta(ventas, v);
                break;
            case 4:
                if (ventas.empty()) {
                    cout << "No hay ventas para eliminar." << endl;
                } else {
                    eliminarVenta(ventas); // Se añade la implementación de eliminarVenta
                }
                break;
            case 5:
                consultasDinamicas(ventas);
                break;
            case 6:
                if (ventas.empty()) {
                    cout << "No hay ventas para mostrar. Cargue o agregue datos primero." << endl;
                } else {
                    menuResumenVentas(ventas);
                }
                break;
            case 7:
                cout << "Saliendo del programa..." << endl;
                break;
            default:
                cout << "Opción incorrecta, intente nuevamente." << endl;
                break;
        }
    } while (opcion != 7);

    return 0;
}