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

    servidor s(io_service, 1337);
    cout << "pm escuchando en 127.0.0.1::1337" << endl;
    //servidor_ftp sftp(io_service, 1338);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

