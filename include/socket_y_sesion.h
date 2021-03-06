#ifndef SOCKET_Y_SESION_H
#define SOCKET_Y_SESION_H

#include "asio.hpp"
#include "estructuras.h"
#include "autenticacion.h"

using asio::ip::tcp;

class sesion
  : public std::enable_shared_from_this<sesion>
{
public:
  sesion(tcp::socket socket)
    : socket_(std::move(socket)),
      usuario_(nullptr)
  {
  }

  void iniciar();

private:
  void iniciar_sesion_usuario();
  void hacer_lectura();
  void hacer_escritura(std::string str);
  void hacer_escritura_terminante(std::string str);
  void procesar_lectura();
  void enviar_archivo(std::string archivo);
  void re_routear_a_clientes();
  void re_routear_a_proveedores();

  /**esta sesi�n desea suscribirse a un servicio*/
  void subscribirse(std::string a_cual);

  /**la sesi�n conectante ofrecer un servicio al que otros puedan suscribirse*/
  void ofrecer(std::string ofrecer_que);

  /***/
  void retransmitir(std::shared_ptr<std::vector<char>> copia);

  /**retorna false si el mensaje no entr� en ninguna categoria. Puede ser llamado recursivamente*/
  bool parsear_mensaje_entrante(std::string lectura);

  tcp::socket socket_;
  enum {longitud_maxima = 8192}; /*tal vez usar un vector para ajustar el tama�o de buffer segun la aplicacion*/
  char data_[longitud_maxima]; //rx
  std::size_t sz_rx_ult_{0};
  std::string str_;
  std::unique_ptr<usuario> usuario_;
  bool proveedor_{false};
  bool consumidor_{false};
  std::string nombre_servicio_{}; //que yo proveo, y no necesariamente debo proveerlo
  std::vector<std::string> suscripciones_; //a qu� estoy suscrito / qu� consumo?
  bool muting_{false};
};

class servidor
{
public:
  servidor(asio::io_service& io_service, short puerto)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), puerto)),
      socket_(io_service)
  {
    hacer_aceptacion();
  }

private:
  void hacer_aceptacion();
  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

#endif // SOCKET_Y_SESION_H
