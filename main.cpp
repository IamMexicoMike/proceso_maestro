#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <fstream>

#include "asio.hpp"
#include "estructuras.h"
#include "socket_y_sesion.h"

#define PUERTO_CONTROL 1337
#define PUERTO_FTP 1339
#define PUERTO_CHAT 1341

using asio::ip::tcp;
using namespace std;

/*PROCESO MAESTRO*/
int main(int argc, char* argv[])
{

  /*Espacio de recuperaci�n de errores. En teor�a este programa rara vez inicia.
    La l�gica que aqu� se dise�e repercutir� en que la recuperaci�n sea adecuada*/
  /*if(mapa.empty())
      rellenar_mapa(algun_archivo)*/

  try
  {
    asio::io_service io_service;

    servidor servidor_de_control(io_service, PUERTO_CONTROL);
    servidor servidor_ftp(io_service, PUERTO_FTP); //aunque sean id�nticos, no queremos solicitudes de control a media transferencia
    cout << "pm escuchando en puertos "<< PUERTO_CONTROL << "(ctrl) y " << PUERTO_FTP << "(ftp)" << endl;

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Excepcion en main proceso maestro: " << e.what() << "\n";
  }

  return 0;
}

