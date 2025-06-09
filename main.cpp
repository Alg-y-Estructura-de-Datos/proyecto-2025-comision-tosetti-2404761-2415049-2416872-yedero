#include <vector>
#include <unordered_map> // Tabla hash estandar de c++
#include <algorithm> // La usamos para ordenar
#include <iostream>
#include <string>
#include <fstream> // Para leer nuestro archivo CSV
#include <limits> // Lo usamos para limpiar el buffer de entrada
#include <sstream> // Para leer nuestro archivo CSV
#include <iomanip>
#include <chrono> // Para medir tiempos de ejecucion
#include <ctime> // En nuestra funcion fechaAEntero para la logica de conversion.
#include "Arbol/ArbolBinarioAVL.h"
#include "Estructuras.h"
#include "Consultas.h"

// Asegurate de que esta ruta sea correcta para tu entorno.
#define BASE_DE_DATOS "ventas_sudamerica.csv"

using namespace std;
using namespace chrono;


// Variable global para almacenar el encabezado del CSV.
// Es una solucion simple para este caso; en una aplicacion mas grande,
// se podria pasar como parametro o ser parte de una clase de gestion de datos.
string csvHeader = "idVenta,fecha,pais,ciudad,cliente,producto,categoria,cantidad,precioUnitario,montoTotal,medioEnvio,estadoEnvio";


// ======= FUNCIONES AUXILIARES =======
string trim(const string& str) {
    // Trim
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";

    size_t last = str.find_last_not_of(" \t\n\r");
    string trimmed = str.substr(first, last - first + 1);

    // Convertir a minusculas
    transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::tolower);

    return trimmed;
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
        // En lugar de salir, podrias crear el archivo si no existe
        ofstream crearArchivo(BASE_DE_DATOS);
        if (crearArchivo.is_open()) {
            cout << "El archivo de base de datos no existia y ha sido creado." << endl;
            // Escribe el encabezado en el archivo recien creado
            crearArchivo << csvHeader << endl;
            crearArchivo.close();
            ventas.clear(); // El vector esta vacio, ya que el archivo estaba vacio
            return;
        } else {
            cout << "Error CRiTICO: No se pudo abrir ni crear el archivo de base de datos." << endl;
            return; // No se puede continuar sin el archivo
        }
    }

    string linea;
    // Leer la primera linea (encabezado) y guardarla, pero no procesarla como datos.
    // Si el archivo esta vacio, getline devolvera falso, y el encabezado global se mantiene.
    if (getline(archivo, linea)) {
        // Opcional: podrias verificar si 'linea' coincide con tu 'csvHeader' esperado.
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
        // Si una linea esta mal formateada (menos campos), esto evitara un crash.
        try {
            if (!getline(ss, campo, ',')) continue; v.idVenta = stoi(campo);
            if (!getline(ss, v.fecha, ',')) continue;
            if (!getline(ss, v.pais, ',')) continue; v.pais = trim(v.pais);
            if (!getline(ss, v.ciudad, ',')) continue; v.ciudad = trim(v.ciudad);
            if (!getline(ss, v.cliente, ',')) continue;v.cliente = trim(v.cliente);
            if (!getline(ss, v.producto, ',')) continue; v.producto = trim(v.producto);
            if (!getline(ss, v.categoria, ',')) continue; v.categoria = trim(v.categoria);
            if (!getline(ss, campo, ',')) continue; v.cantidad = stoi(campo);
            if (!getline(ss, campo, ',')) continue; v.precioUnitario = stod(campo);
            if (!getline(ss, campo, ',')) continue; v.montoTotal = stod(campo);
            if (!getline(ss, v.medioEnvio, ',')) continue; v.medioEnvio = trim(v.medioEnvio);
            if (!getline(ss, v.estadoEnvio)) continue; v.estadoEnvio = trim(v.estadoEnvio);

            ventas.push_back(v);
        } catch (const exception& e) {
            cout << "Advertencia: Salto una linea mal formateada en el CSV: " << linea << " (" << e.what() << ")" << endl;
            continue; // Saltar a la siguiente linea si hay un error de parseo
        }
    }
    archivo.close();
    cout << "Ventas cargadas correctamente. Total: " << ventas.size() << " registros." << endl;
}

// Esta funcion toma el vector de ventas crudas y crea el mapa agregado
void procesarDatosAgregados(const vector<Venta>& todasLasVentas, MapaPaises& datosAgregados) {
    // Limpiamos el mapa por si tenia datos anteriores
    datosAgregados.clear();

    for (const auto& v : todasLasVentas) {
        // La magia del operador[] de unordered_map hace esto increiblemente simple.
        // Si la clave no existe, la crea; si existe, devuelve una referencia al valor.
        EstadisticasAgregadas& stats = datosAgregados[v.pais][v.producto];

        // Actualizamos las estadisticas directamente.
        stats.montoTotalVendido += v.montoTotal;
        stats.cantidadTotalVendida += v.cantidad;
        stats.conteoMediosEnvio[v.medioEnvio]++;
    }
    cout << "Datos procesados para consultas analiticas." << endl;
}

// ======= FUNCIONES DE MODIFICACIoN =======
void agregarVenta(vector<Venta>& ventas, Venta v) {
    // Para asegurar que el archivo tenga el encabezado al agregar la primera venta
    // o si el archivo fue borrado/corrupto.
    ofstream archivo(BASE_DE_DATOS, ios::out | ios::trunc); // Abrir en modo truncado para reescribir si esta vacio
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

    // --- Logica de entrada de datos para la nueva venta (pais, ciudad, cliente, producto, etc.) ---
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
            cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

    } while (!entradaValida);

    switch (opcion) {
        case 1: v.pais = "argentina"; break;
        case 2: v.pais = "brasil"; break;
        case 3: v.pais = "chile"; break;
        case 4: v.pais = "colombia"; break;
        case 5: v.pais = "ecuador"; break;
        case 6: v.pais = "peru"; break;
        case 7: v.pais = "uruguay"; break;
        case 8: v.pais = "venezuela"; break;
        default: ; // Nunca deberiamos llegar debido a la validacion.
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
        cout<<"3. Camara"<<endl;
        cout<<"4. Escritorio"<<endl;
        cout<<"5. Impresora"<<endl;
        cout<<"6. Laptop"<<endl;
        cout<<"7. Monitor"<<endl;
        cout<<"8. Silla ergonomica"<<endl;
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
        cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (!entrada2Valida);

    switch (opcion2) {
        case 1: v.producto="auriculares"; v.categoria="accesorios"; break;
        case 2: v.producto="celular"; v.categoria="electronica"; break;
        case 3: v.producto="camara"; v.categoria="electronica"; break;
        case 4: v.producto="escritorio"; v.categoria="muebles"; break;
        case 5: v.producto="impresora"; v.categoria="oficina"; break;
        case 6: v.producto="laptop"; v.categoria="electronica"; break;
        case 7: v.producto="monitor"; v.categoria="electronica"; break;
        case 8: v.producto="silla ergonomica"; v.categoria="muebles"; break;
        case 9: v.producto="tablet"; v.categoria="electronica"; break;
        case 10: v.producto="teclado"; v.categoria="accesorios"; break;
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
            cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
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
            cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
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
        case 1: v.medioEnvio = "terrestre"; break;
        case 2: v.medioEnvio = "maritimo"; break;
        case 3: v.medioEnvio = "aereo"; break;
        default: ; // Inalcanzable
    }

    v.estadoEnvio="pendiente";

    // Escribe la nueva venta al final del archivo CSV.
    // NOTA: Con la correccion de reescribir todo el archivo, esta parte ya no es
    // estrictamente "al final" del archivo despues de abrir con ios::app.
    // Ahora, agregamos 'v' al vector y luego reescribimos todo el vector.
    // El archivo se abre en truncado al principio de la funcion, se escribe el encabezado,
    // se escriben las ventas anteriores, y luego se agregara esta nueva venta al vector.

    // ¡IMPORTANTE! Agrega la nueva venta tambien al vector en memoria.
    ventas.push_back(v);

    // Ahora reescribe todo el archivo con la nueva venta incluida
    // Esto es crucial para que los cambios persistan y el encabezado se mantenga.
    ofstream reescrituraArchivo(BASE_DE_DATOS, ios::out | ios::trunc); // Abrir en modo truncado
    if (!reescrituraArchivo.is_open()) {
        cout << "Error CRiTICO: No se pudo reabrir el archivo para reescritura. Los cambios no se guardaron permanentemente." << endl;
        return;
    }
    reescrituraArchivo << csvHeader << endl; // Vuelve a escribir el encabezado
    for (const auto& ventaActual : ventas) {
        reescrituraArchivo << ventaActual.idVenta << ","
                           << ventaActual.fecha << ","
                           << ventaActual.pais << ","
                           << trim(ventaActual.ciudad) << ","
                           << trim(ventaActual.cliente) << ","
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


void modificarVenta(vector<Venta>& bdVector) {
    if (bdVector.empty()) {
        cout << "La base de datos (vector) esta vacia. No hay ventas para modificar.\n";
        return;
    }

    // 1. Crear un arbol AVL para acceso rapido y validacion de existencia.
    ArbolBinarioAVL<Venta> arbolDeVentas;
    for (const auto& ventaActual : bdVector) {
        arbolDeVentas.put(ventaActual); // introducimos todas las ventas en un arbol Balanceado
    }

    // 2. Solicitar el ID de la venta a modificar.
    int idModificarNum;
    string entradaID;
    bool idValido = false;
    Venta ventaParaBuscar; // Venta temporal para usar en la busqueda del arbol

    do {
        cout << "Ingrese el ID de la venta que desea modificar: ";
        cin >> entradaID;
        try {
            idModificarNum = stoi(entradaID);
            ventaParaBuscar.idVenta = idModificarNum; // Asignar el ID a la venta de busqueda

            // Intentar buscar en el arbol AVL para validar la existencia del ID
            // Asumimos que arbol.search() lanza una excepcion si no encuentra el elemento.
            // El valor devuelto por search() no se usa directamente aqui, solo se verifica la existencia.
            arbolDeVentas.search(ventaParaBuscar);
            idValido = true; // Si search() no lanzo excepcion, el ID existe

        } catch (const invalid_argument& e_stoi) {
            cout << "Entrada invalida para ID (no es un numero): " << e_stoi.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } catch (const out_of_range& e_stoi_range) {
            cout << "Entrada invalida para ID (numero fuera de rango): " << e_stoi_range.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } catch (const exception& e_avl) { // Captura la excepcion esperada de arbol.search si no encuentra
            cout << "ID de venta no encontrado en el arbol: " << e_avl.what() << ". Intente nuevamente.\n";
            // idValido permanece false
             cin.clear(); // Limpiar flags de error de cin
             cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Descartar entrada incorrecta
        }
    } while (!idValido);

    // El ID es valido y existe en el arbol. Ahora obtenemos una referencia a la venta en bdVector.
    Venta* ventaAModificarEnVector = nullptr;
    for (size_t i = 0; i < bdVector.size(); ++i) {
        if (bdVector[i].idVenta == idModificarNum) {
            ventaAModificarEnVector = &bdVector[i];
            break;
        }
    }

    if (!ventaAModificarEnVector) {
        // Esto no deberia ocurrir si el arbol se construyo correctamente desde bdVector y el ID se valido.
        cout << "Error critico: El ID fue validado pero la venta no se encontro en el vector fuente. Abortando modificacion.\n";
        return;
    }

    // 3. Mostrar los detalles de la venta seleccionada (usando el puntero al objeto en bdVector).
    cout << "\n--- Venta seleccionada para modificar ---\n";
    cout << "ID: " << ventaAModificarEnVector->idVenta << endl;
    cout << "Fecha: " << ventaAModificarEnVector->fecha << endl;
    cout << "Pais: " << ventaAModificarEnVector->pais << endl;
    cout << "Ciudad: " << ventaAModificarEnVector->ciudad << endl;
    cout << "Cliente: " << ventaAModificarEnVector->cliente << endl;
    cout << "Producto: " << ventaAModificarEnVector->producto << endl;
    cout << "Categoria: " << ventaAModificarEnVector->categoria << endl;
    cout << "Cantidad: " << ventaAModificarEnVector->cantidad << endl;
    cout << "Precio unitario: " << fixed << setprecision(2) << ventaAModificarEnVector->precioUnitario << endl;
    cout << "Monto total: " << fixed << setprecision(2) << ventaAModificarEnVector->montoTotal << endl;
    cout << "Medio de envio: " << ventaAModificarEnVector->medioEnvio << endl;
    cout << "Estado de envio: " << ventaAModificarEnVector->estadoEnvio << endl;
    cout << "--------------------------------------\n";

    // 4. Solicitar el campo a modificar.
    int opcionCampo;
    string entradaCampo;
    bool campoValido = false;

    do {
        cout << "\n¿Que campo desea modificar?\n";
        cout << "1. Pais\n";
        cout << "2. Ciudad\n";
        cout << "3. Cliente\n";
        cout << "4. Producto\n"; // Modifica Producto y Categoria
        cout << "5. Cantidad\n";
        cout << "6. Precio Unitario\n";
        cout << "7. Medio de Envio\n";
        cout << "8. Estado de Envio\n";
        cout << "Opcion: ";
        cin >> entradaCampo;

        try {
            opcionCampo = stoi(entradaCampo);
            if (opcionCampo >= 1 && opcionCampo <= 8) {
                campoValido = true;
            } else {
                cout << "Opcion fuera de rango. Intente nuevamente.\n";
            }
        } catch (const exception& e) {
            cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (!campoValido);

    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Limpiar buffer para getline

    // 5. Aplicar la modificacion segun el campo seleccionado.
    // Las modificaciones se hacen directamente en ventaAModificarEnVector.
    switch (opcionCampo) {
        case 1: { // Modificar Pais
            int opcionPais = 0;
            string entradaPais;
            bool paisValido = false;
            do {
                cout << "Seleccione el nuevo pais:\n";
                cout << "1. Argentina\n2. Brasil\n3. Chile\n4. Colombia\n5. Ecuador\n6. Peru\n7. Uruguay\n8. Venezuela\n";
                cout << "Opcion: ";
                cin >> entradaPais;
                try {
                    opcionPais = stoi(entradaPais);
                    if (opcionPais >= 1 && opcionPais <= 8) {
                        paisValido = true;
                    } else {
                        cout << "Opcion fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const exception& e) {
                    cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (!paisValido);
            switch (opcionPais) {
                case 1: ventaAModificarEnVector->pais = "Argentina"; break;
                case 2: ventaAModificarEnVector->pais = "Brasil"; break;
                case 3: ventaAModificarEnVector->pais = "Chile"; break;
                case 4: ventaAModificarEnVector->pais = "Colombia"; break;
                case 5: ventaAModificarEnVector->pais = "Ecuador"; break;
                case 6: ventaAModificarEnVector->pais = "Peru"; break;
                case 7: ventaAModificarEnVector->pais = "Uruguay"; break;
                case 8: ventaAModificarEnVector->pais = "Venezuela"; break;
            }
            cout << "Pais modificado correctamente por: "<<ventaAModificarEnVector->pais<<"\n";
            break;
        }
        case 2: { // Modificar Ciudad
            cout << "Ingrese la nueva ciudad: ";
            getline(cin, ventaAModificarEnVector->ciudad);
            cout << "Ciudad reemplazada correctamente por: "<<ventaAModificarEnVector->ciudad<<"\n";
            break;
        }
        case 3: { // Modificar Cliente
            cout << "Ingrese el nuevo nombre y apellido del cliente: ";
            getline(cin, ventaAModificarEnVector->cliente);
            cout << "Nombre del cliente reemplazado correctamente por: "<<ventaAModificarEnVector->cliente<<"\n";
            break;
        }
        case 4: { // Modificar Producto y Categoria
            int opcionProducto = 0;
            string entradaProducto;
            bool productoValido = false;
            do {
                cout << "Seleccione el nuevo producto:\n";
                cout << "1. Auriculares\n2. Celular\n3. Camara\n4. Escritorio\n5. Impresora\n6. Laptop\n7. Monitor\n8. Silla ergonomica\n9. Tablet\n10. Teclado\n";
                cout << "Opcion: ";
                cin >> entradaProducto;
                try {
                    opcionProducto = stoi(entradaProducto);
                    if (opcionProducto >= 1 && opcionProducto <= 10) {
                        productoValido = true;
                    } else {
                        cout << "Opcion fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const exception& e) {
                    cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (!productoValido);
            // No es necesario cin.ignore() aqui porque la lectura fue de un string que se convirtio
            // y la siguiente lectura sera probablemente un cout. Si fuera un getline, si.

            switch (opcionProducto) {
                case 1: ventaAModificarEnVector->producto = "Auriculares"; ventaAModificarEnVector->categoria = "Accesorios"; break;
                case 2: ventaAModificarEnVector->producto = "Celular"; ventaAModificarEnVector->categoria = "Electronica"; break;
                case 3: ventaAModificarEnVector->producto = "Camara"; ventaAModificarEnVector->categoria = "Electronica"; break;
                case 4: ventaAModificarEnVector->producto = "Escritorio"; ventaAModificarEnVector->categoria = "Muebles"; break;
                case 5: ventaAModificarEnVector->producto = "Impresora"; ventaAModificarEnVector->categoria = "Oficina"; break;
                case 6: ventaAModificarEnVector->producto = "Laptop"; ventaAModificarEnVector->categoria = "Electronica"; break;
                case 7: ventaAModificarEnVector->producto = "Monitor"; ventaAModificarEnVector->categoria = "Electronica"; break;
                case 8: ventaAModificarEnVector->producto = "Silla ergonomica"; ventaAModificarEnVector->categoria = "Muebles"; break;
                case 9: ventaAModificarEnVector->producto = "Tablet"; ventaAModificarEnVector->categoria = "Electronica"; break;
                case 10: ventaAModificarEnVector->producto = "Teclado"; ventaAModificarEnVector->categoria = "Accesorios"; break;
            }
            cout << "Producto y categoria modificados correctamente, su nuevo producot es:"<<ventaAModificarEnVector->producto<<"\n";
            // Recalcular monto total si es necesario (aunque producto no afecta precio directamente aqui)
            // ventaAModificarEnVector->montoTotal = ventaAModificarEnVector->cantidad * ventaAModificarEnVector->precioUnitario;
            // cout << "Monto total recalculado:" << fixed << setprecision(2) << ventaAModificarEnVector->montoTotal << endl;
            break;
        }
        case 5: { // Modificar Cantidad
            string nuevaCantidadStr;
            bool cantidadValida = false;
            int nuevaCantidadNum;
            do {
                cout << "Ingrese la nueva cantidad: ";
                cin >> nuevaCantidadStr;
                try {
                    nuevaCantidadNum = stoi(nuevaCantidadStr);
                    if (nuevaCantidadNum > 0) {
                        ventaAModificarEnVector->cantidad = nuevaCantidadNum;
                        cantidadValida = true;
                    } else {
                        cout << "La cantidad debe ser mayor que cero.\n";
                    }
                } catch (const exception& e) {
                    cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (!cantidadValida);
            ventaAModificarEnVector->montoTotal = ventaAModificarEnVector->cantidad * ventaAModificarEnVector->precioUnitario;
            cout << "Cantidad modificada correctamente. Nuevo monto total: " << fixed << setprecision(2) << ventaAModificarEnVector->montoTotal << endl;
            break;
        }
        case 6: { // Modificar Precio Unitario
            string nuevoPrecioStr;
            bool precioValido = false;
            double nuevoPrecioNum;
            do {
                cout << "Ingrese el nuevo precio unitario: ";
                cin >> nuevoPrecioStr;
                try {
                    nuevoPrecioNum = stod(nuevoPrecioStr); // Usar stod para double
                    if (nuevoPrecioNum > 0) {
                        ventaAModificarEnVector->precioUnitario = nuevoPrecioNum;
                        precioValido = true;
                    } else {
                        cout << "El precio debe ser mayor que cero.\n";
                    }
                } catch (const exception& e) {
                    cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (!precioValido);
            ventaAModificarEnVector->montoTotal = ventaAModificarEnVector->cantidad * ventaAModificarEnVector->precioUnitario;
            cout << "Precio unitario modificado correctamente. Nuevo monto total: " << fixed << setprecision(2) << ventaAModificarEnVector->montoTotal << endl;
            break;
        }
        case 7: { // Modificar Medio de Envio
            int opcionMedio = 0;
            string entradaMedio;
            bool medioValido = false;
            do {
                cout << "Seleccione el nuevo medio de envio:\n";
                cout << "1. Terrestre\n2. Maritimo\n3. Aereo\n";
                cout << "Opcion: ";
                cin >> entradaMedio;
                try {
                    opcionMedio = stoi(entradaMedio);
                    if (opcionMedio >= 1 && opcionMedio <= 3) {
                        medioValido = true;
                    } else {
                        cout << "Opcion fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const exception& e) {
                    cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (!medioValido);
            switch (opcionMedio) {
                case 1: ventaAModificarEnVector->medioEnvio = "Terrestre"; break;
                case 2: ventaAModificarEnVector->medioEnvio = "Maritimo"; break;
                case 3: ventaAModificarEnVector->medioEnvio = "Aereo"; break;
            }
            cout << "Medio de envio modificado correctamente, el nuevo medio de envio es:"<<ventaAModificarEnVector->medioEnvio<<"\n";
            break;
        }
        case 8: { // Modificar Estado de Envio
            int opcionEstado = 0;
            string entradaEstado;
            bool estadoValido = false;
            do {
                cout << "Seleccione el nuevo estado de envio:\n";
                cout << "1. Pendiente\n2. En Proceso\n3. Entregado\n4. Cancelado\n";
                cout << "Opcion: ";
                cin >> entradaEstado;
                try {
                    opcionEstado = stoi(entradaEstado);
                    if (opcionEstado >= 1 && opcionEstado <= 4) {
                        estadoValido = true;
                    } else {
                        cout << "Opcion fuera de rango. Intente nuevamente.\n";
                    }
                } catch (const exception& e) {
                    cout << "Entrada invalida: " << e.what() << ". Intente nuevamente.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } while (!estadoValido);
            switch (opcionEstado) {
                case 1: ventaAModificarEnVector->estadoEnvio = "Pendiente"; break;
                case 2: ventaAModificarEnVector->estadoEnvio = "En Proceso"; break;
                case 3: ventaAModificarEnVector->estadoEnvio = "Entregado"; break;
                case 4: ventaAModificarEnVector->estadoEnvio = "Cancelado"; break;
            }
            cout << "Estado de envio modificado correctamente, estado actual"<<ventaAModificarEnVector->estadoEnvio<<"\n";
            break;
        }
        default:
            cout << "Opcion de campo invalida.\n"; // No deberia llegar aqui por la validacion previa
            break;
    }

    // 6. REESCRITURA COMPLETA DEL ARCHIVO CSV.
    // El vector bdVector ya tiene la venta modificada.
    // Abre el archivo en modo de truncado (BORRA el contenido existente).
    ofstream archivo(BASE_DE_DATOS, ios::out | ios::trunc);
    if (!archivo.is_open()) {
        cout << "ERROR CRiTICO: No se pudo abrir el archivo '" << BASE_DE_DATOS << "' para escritura y actualizacion. Los cambios NO se guardaron permanentemente.\n";
        return;
    }

    archivo << csvHeader << endl; // Escribe el encabezado ANTES de los datos.

    for (const auto& v : bdVector) { // Itera sobre el bdVector actualizado
        archivo << v.idVenta << ",";
        archivo << v.fecha << ",";
        archivo << v.pais << ",";
        archivo << v.ciudad << ",";
        archivo << "\"" << v.cliente << "\","; // Considerar comillas si cliente puede tener comas
        archivo << v.producto << ",";
        archivo << v.categoria << ",";
        archivo << v.cantidad << ",";
        archivo << fixed << setprecision(2) << v.precioUnitario << ",";
        archivo << fixed << setprecision(2) << v.montoTotal << ",";
        archivo << v.medioEnvio << ",";
        archivo << v.estadoEnvio << endl; // Usar endl para asegurar flush y nueva linea
    }
    archivo.close();
    cout << "Venta modificada y archivo CSV actualizado correctamente.\n";
}

void eliminarVenta(vector<Venta>& ventas) {
    cout << "Ingrese el ID de la venta a eliminar: ";
    string idEliminarStr;
    cin >> idEliminarStr;

    int idEliminar;
    try {
        idEliminar = stoi(idEliminarStr);
    } catch (const exception& e) {
        cout << "ID invalido. Intente nuevamente." << endl;
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

        // Reescribir todo el archivo CSV para reflejar la eliminacion
        ofstream archivo(BASE_DE_DATOS, ios::out | ios::trunc);
        if (!archivo.is_open()) {
            cout << "ERROR CRiTICO: No se pudo abrir el archivo para reescritura despues de eliminar. Los cambios NO se guardaron permanentemente." << endl;
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
// Estas funciones no las modifico, solo las dejo como declaracion si no estan en tu original
// ======= FUNCIONES DE PROCESAMIENTO =======
void calcularTopCiudades(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, unordered_map<string, double>> ventasAcumuladas;
    for (size_t i = 0; i < ventas.size(); ++i) {
        ventasAcumuladas[ventas[i].pais][ventas[i].ciudad] += ventas[i].montoTotal;
        ifCount++;
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

void montoTotalPorProductoPorPais(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, unordered_map<string, double>> montoPorProductoPorPais;

    // Acumular ventas por pais y producto
    for (size_t i = 0; i < ventas.size(); ++i) {
        montoPorProductoPorPais[ventas[i].pais][ventas[i].producto] += ventas[i].montoTotal;
        ifCount++;
    }

    cout << fixed << setprecision(2);
    for (unordered_map<string, unordered_map<string, double>>::const_iterator it = montoPorProductoPorPais.begin();
        it != montoPorProductoPorPais.end(); ++it) {

        const string& pais = it->first;
        const unordered_map<string, double>& productos = it->second;

        cout << "===============================\n";
        cout << "PAiS: " << pais << endl;

        for (unordered_map<string, double>::const_iterator it2 = productos.begin(); it2 != productos.end(); ++it2) {
            cout << "- " << it2->first << ": $" << it2->second << endl;
        }
        cout << endl;
    }
}

void promedioVentasPorCategoriaPorPais(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, unordered_map<string, pair<double, int>>> datosPorCategoriaPorPais;

    // Acumular ventas y contar por pais y categoria
    for (size_t i = 0; i < ventas.size(); ++i) {
        datosPorCategoriaPorPais[ventas[i].pais][ventas[i].categoria].first += ventas[i].montoTotal;
        datosPorCategoriaPorPais[ventas[i].pais][ventas[i].categoria].second++;
        ifCount++;
    }

    cout << fixed << setprecision(2);
    for (unordered_map<string, unordered_map<string, pair<double, int>>>::const_iterator it = datosPorCategoriaPorPais.begin();
         it != datosPorCategoriaPorPais.end(); ++it) {

        const string& pais = it->first;
        const unordered_map<string, pair<double, int>>& categorias = it->second;

        cout << "===============================\n";
        cout << "PAiS: " << pais << endl;

        for (auto it2 = categorias.begin();
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

void medioEnvioMasUtilizadoPorPais(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, unordered_map<string, int>> conteoEnviosPorPais;

    // Contar medios de envio por pais
    for (size_t i = 0; i < ventas.size(); ++i) {
        conteoEnviosPorPais[ventas[i].pais][ventas[i].medioEnvio]++;
        ifCount++;
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

        cout << "PAiS: " << pais << " Medio mas usado: " << medioMasUsado << " (" << maxCantidad << " veces)" << endl;
         }
}

void medioEnvioMasUtilizadoPorCategoria(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, unordered_map<string, int>> conteoEnviosPorCategoria;

    // Contar medios de envio por categoria
    for (size_t i = 0; i < ventas.size(); ++i) {
        conteoEnviosPorCategoria[ventas[i].categoria][ventas[i].medioEnvio]++;
        ifCount++;
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

        cout << "CATEGORiA: " << categoria << " Medio mas usado: " << medioMasUsado << " (" << maxCantidad << " veces)" << endl;
         }
}

void diaConMasVentas(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, int> conteoPorFecha;

    // Contar ventas por fecha
    for (size_t i = 0; i < ventas.size(); ++i) {
        conteoPorFecha[ventas[i].fecha]++;
        ifCount++;
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
        cout << "Dia con mas ventas: " << fechaMax << " (" << maxVentas << " ventas)" << endl;
    } else {
        cout << "No hay ventas registradas." << endl;
    }
}

void estadoEnvioMasFrecuentePorPais(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, unordered_map<string, int>> estadosPorPais;

    // Contar estados de envio por pais
    for (size_t i = 0; i < ventas.size(); ++i) {
        estadosPorPais[ventas[i].pais][ventas[i].estadoEnvio]++;
        ifCount++;
    }

    cout << "Estado de envio mas frecuente por pais:\n";

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

void productoMasVendido(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, int> cantidadPorProducto;

    // Acumular cantidades por producto
    for (size_t i = 0; i < ventas.size(); ++i) {
        cantidadPorProducto[ventas[i].producto] += ventas[i].cantidad;
        ifCount++;
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

    cout << "Producto mas vendido en cantidad total:\n";
    cout << "- " << productoTop << " (" << maxCantidad << " unidades vendidas)" << endl;
}

void productoMenosVendido(const vector<Venta>& ventas, int &ifCount) {
    unordered_map<string, int> cantidadPorProducto;

    // Acumular cantidades por producto
    for (size_t i = 0; i < ventas.size(); ++i) {
        cantidadPorProducto[ventas[i].producto] += ventas[i].cantidad;
        ifCount++;
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

void menuResumenVentas(const vector<Venta>& ventas) {
    int opcionResumen;
    do {
        cout << "--- Resumen de Ventas ---\n";
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

        // Limpiar el buffer despues de leer un entero
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (opcionResumen) {
            case 1: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                calcularTopCiudades(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) para agrupar, vector + sort() para ordenar." << endl;
                break;
            }
            case 2: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                montoTotalPorProductoPorPais(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) anidada para agrupar montos por pais y producto." << endl;
                break;
            }
            case 3: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                promedioVentasPorCategoriaPorPais(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "Tiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) anidada con pair para calcular promedios por pais y categoria." << endl;
                break;
            }
            case 4: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                medioEnvioMasUtilizadoPorPais(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) anidada para conteo y busqueda del maximo." << endl;
                break;
            }
            case 5: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                medioEnvioMasUtilizadoPorCategoria(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) anidada para conteo y busqueda del maximo." << endl;
                break;
            }
            case 6: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                diaConMasVentas(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) para conteo y busqueda del maximo." << endl;
                break;
            }
            case 7: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                estadoEnvioMasFrecuentePorPais(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) anidada para conteo y busqueda del maximo." << endl;
                break;
            }
            case 8: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                productoMasVendido(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) para acumular cantidades y busqueda del maximo." << endl;
                break;
            }
            case 9: {
                int ifCount = 0;
                auto inicio = steady_clock::now();
                productoMenosVendido(ventas, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin - inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << " ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: tabla hash (unordered_map) para acumular cantidades y busqueda del minimo." << endl;
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


void mostrarMenu() {
    cout << "\n========== MENU DE CONSULTAS DINAMICAS ==========" << endl;
    cout << "1. Listar ventas por ciudad" << endl;
    cout << "2. Listar ventas por rango de fechas y pais" << endl;
    cout << "3. Comparar metricas de dos paises" << endl;
    cout << "4. Comparar dos productos en todos los paises" << endl;
    cout << "5. Buscar productos por monto promedio (umbral)" << endl;
    cout << "0. Salir" << endl;
    cout << "=================================================" << endl;
    cout << "Seleccione una opcion: ";
}

void menuConsultas() {
    // Estructuras de datos principales
    vector<Venta> todasLasVentas;
    MapaPaises datosAgregados;

    // Carga inicial de datos
    cargarVentas(todasLasVentas);
    procesarDatosAgregados(todasLasVentas, datosAgregados);

    int opcion;
    do {
        mostrarMenu();
        cin >> opcion;

        // Limpiar el buffer de entrada para que getline funcione bien despues de cin >>
        if (cin.peek() == '\n') {
            cin.ignore();
        }

        switch (opcion) {
            case 1: {
                int ifCount = 0;
                string ciudad;
                cout << "-> Ingrese el nombre de la ciudad: ";
                getline(cin, ciudad);

                auto inicio = steady_clock::now();
                consultaPorCiudad(todasLasVentas, trim(ciudad), ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin-inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << "ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: recorrido lineal (vector) con busqueda condicional." << endl;
                break;
            }
            case 2: {
                int ifCount = 0;
                string pais, fechaInicio, fechaFin;
                cout << "-> Ingrese el pais: ";
                getline(cin, pais);
                cout << "-> Ingrese la fecha de inicio (DD/MM/YYYY): ";
                getline(cin, fechaInicio);
                cout << "-> Ingrese la fecha de fin (DD/MM/YYYY): ";
                getline(cin, fechaFin);

                auto inicio = steady_clock::now();
                consultaPorFechaYPais(todasLasVentas, trim(pais), fechaInicio, fechaFin, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin-inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << "ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: recorrido lineal con filtros condicionales y conversion de fechas para comparacion." << endl;
                break;
            }
            case 3: {
                int ifCount = 0;
                string p1, p2;
                cout << "-> Ingrese el primer pais: ";
                getline(cin, p1);
                cout << "-> Ingrese el segundo pais: ";
                getline(cin, p2);

                auto inicio = steady_clock::now();
                compararDosPaises(datosAgregados, trim(p1), trim(p2), ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin-inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << "ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: recorrido lineal (unordered_map) con acumulacion y busqueda de maximo." << endl;
                break;
            }
            case 4: {
                int ifCount = 0;
                string prod1_str, prod2_str;
                cout << "-> Ingrese el primer producto: ";
                getline(cin, prod1_str);
                cout << "-> Ingrese el segundo producto: ";
                getline(cin, prod2_str);

                auto inicio = steady_clock::now();
                compararDosProductos(datosAgregados, trim(prod1_str), trim(prod2_str), ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin-inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << "ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: recorrido lineal (unordered_map) para consulta y comparacion condicional." << endl;
                break;
            }
            case 5: {
                int ifCount = 0;
                string pais, modo;
                double umbral;
                cout << "-> Ingrese el pais: ";
                getline(cin, pais);
                cout << "-> Ingrese el monto umbral (ej: 500): ";
                cin >> umbral;
                cout << "-> Buscar productos con promedio (mayor/menor) al umbral?: ";
                cin >> modo;
                // Limpiamos el buffer de entrada despues de leer un numero o palabra.
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                auto inicio = steady_clock::now();
                buscarPorPromedio(datosAgregados, trim(pais), trim(modo), umbral, ifCount);
                auto fin = steady_clock::now();
                auto duracion = duration_cast<milliseconds>(fin-inicio).count();
                cout << "\nTiempo de ejecucion: " << duracion << "ms\n";
                cout << "Condicionales utilizados: " << ifCount << " \n" << endl;
                cout << "Algoritmo/estructura utilizada: recorrido lineal (unordered_map) con filtro condicional." << endl;
                break;
            }
            case 0: cout << "Saliendo del modulo de consultas." << endl; break;
            default:
                cout << "Opcion no valida. Intente de nuevo." << endl;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            break;
        }
    } while (opcion != 0);
}

// ======= FUNCIoN MAIN CORREGIDA =======
int main() {
    vector<Venta> ventas;
    Venta v; // Esta 'v' se usa para la nueva venta en agregarVenta, no es la base de datos principal

    cout << "Iniciando sistema de gestion de ventas..." << endl;

    int opcion;

    do {
        cargarVentas(ventas);

        cout << "\n====== MENU PRINCIPAL ======" << endl;
        // La opcion 1 ya no es "Cargar datos" en el sentido de iniciar, sino para recargar manualmente.
        cout << "1. Recargar datos (solo si el archivo externo ha cambiado sin reiniciar el programa)" << endl;
        cout << "2. Modificar una venta" << endl;
        cout << "3. Agregar una venta" << endl;
        cout << "4. Eliminar una venta" << endl;
        cout << "5. Consultas dinamicas" << endl;
        cout << "6. Mostrar resumen de ventas" << endl;
        cout << "7. Salir" << endl;
        cout << "Ingrese una opcion: ";

        // Leer la opcion de forma segura
        string opcionStr;
        cin >> opcionStr;
        try {
            opcion = stoi(opcionStr);
        } catch (const exception& e) {
            cout << "Entrada invalida. Por favor, ingrese un numero." << endl;
            opcion = 0; // Para que el bucle continue y pida la opcion de nuevo
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue; // Saltar al siguiente ciclo del bucle
        }

        // Limpiar el buffer despues de leer un entero con cin >>
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
                // `agregarVenta` ahora manejara la reescritura completa del archivo
                agregarVenta(ventas, v);
                break;
            case 4:
                if (ventas.empty()) {
                    cout << "No hay ventas para eliminar." << endl;
                } else {
                    eliminarVenta(ventas); // Se añade la implementacion de eliminarVenta
                }
                break;
            case 5:
                menuConsultas();
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
                cout << "Opcion incorrecta, intente nuevamente." << endl;
                break;
        }
    } while (opcion != 7);

    return 0;

}