#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <future>
#include <thread>
#include <iterator>
#include "socket_y_sesion.h"
#include "nube.h"

using namespace std;

extern string cargar_valor(string);

void sesion::iniciar()
{
  cout << socket_.remote_endpoint().address().to_string() << ":" << socket_.remote_endpoint().port() << '\n';
  iniciar_sesion_usuario();
}

void sesion::iniciar_sesion_usuario()
{
  auto tu_mismo(shared_from_this());
  socket_.async_read_some(asio::buffer(data_, longitud_maxima),
    [this, tu_mismo](std::error_code ec, std::size_t longitud)
  {
    if (!ec)
    {
      string lectura = data_;
      usuario_ = usuario::autenticar_usuario(lectura); //que pasa si usuario;contrase�a; llega dividido en dos paquetes?
      if(usuario_!=nullptr)
        hacer_lectura();                               //el estatus correcto
      else
      {
        cout << "usuario invalido: " << lectura << endl;
        socket_.close();
      }
    }
    else
    {
      /*y no volvemos a leer, si no, se cicla*/
      cout << "Error en inicio_sesion; error=" << ec.value() << ":" << ec.message() << endl;
      /*ec.value()==2 (End of file) t�picamente significa que el otro lado cerr� la conexi�n*/
    }
  });
}

/**A�ade aqu� entradas para procesar las lecturas entrantes
C�mo se paquetizan? Recuerda que TCP es un stream y no paquetes como UDP*/
void sesion::procesar_lectura()
{
  string lectura = data_;
  if(lectura.back() == 0xA) //LF, por si la solicitud viene de la terminal. Este error ocurria con ncat
    lectura.pop_back();
  cout << lectura << endl;
  if(lectura.substr(0,3) == "ftp") //esta solicitud *s�lo* deben hac�rsela al puerto 1339 (puerto ftp)
  {
    string archivo = lectura.substr(4);
    //cout << "ftp:" << archivo << endl;
    enviar_archivo(archivo);
    memset(data_, '\0', longitud_maxima);
    return; //para evitar volver a leer. De nuevo, a este bloque l�gico s�lo entra la instancia que escucha en 1339
  }
  else if(lectura.substr(0,7) == "version")
  {
    //string version_cliente = lectura.substr(8);
    string version_serv = cargar_valor("version");
    //cout << "Cliente con version " << version_cliente << " solicita actualizacion" << version_serv << endl;
    hacer_escritura("version " + version_serv);
  }

  /*Problema: c�mo suscribes a un ente y luego le transfieres el stream? despues de suscribirte las operaciones de control terminan*/
  else if(lectura.substr(0,7) == "ofrecer") //este socket proveer� un servicio de ahora en adelante
  {
    /* A�adimos un apuntador a este socket al mapa global */
    string nombre_servicio = lectura.substr(8);
    nombre_servicio_ = nombre_servicio;
    nube::servicios.emplace(make_pair(nombre_servicio_, shared_from_this())); //ser� correcto?
    procesar_ = false; //las proximas lecturas pasaran directo a la otra branch, la de forwardeo
  }

  //varios entes pueden subscribirse al mismo servicio
  else if(lectura.substr(0,9) == "suscribir") //este socket consumir� un servicio de ahora en adelante
  {
    string a_cual = lectura.substr(10);
    try
    {
      //obtenemos una referencia al grupo de sockets suscritos a este nombre de servicio
      vector<weak_ptr<sesion>>& grupo = nube::suscritos.at(a_cual);
      grupo.emplace_back(shared_from_this() );
    }
    catch(out_of_range& e) //probablemente el grupo no existia (nadie se habia suscrito a ese servico)
    {
      vector<weak_ptr<sesion>> nvo_grupo;
      nvo_grupo.emplace_back(shared_from_this());
      nube::suscritos.emplace(make_pair(a_cual, nvo_grupo));
    }
    catch(...)
    {
      cout << "excepcion extraordinaria durante la suscripcion\n";
    }
    //procesar_=false; //podriamos entrar a un escenario de transmision unidireccional
    /* Un socket que consume un servicio tambi�n re rutea?*/
  }

  memset(data_, '\0', longitud_maxima);
  hacer_lectura(); //siempre volvemos a "escuchar"
}

void sesion::hacer_lectura()
{
  auto si_mismo(shared_from_this());
  socket_.async_read_some(asio::buffer(data_, longitud_maxima),
    [this, si_mismo](std::error_code ec, std::size_t longitud)
  {
    if (!ec)
    {
      if(procesar_)
        procesar_lectura(); /* soy un socket normal, de control. Proceso la lectura y vuelvo a leer (procesar lectura lee al final)*/
      else
      {
        /*soy un socket puente, re ruteo a los interesados*/
        re_routear();
      }
    }
    else
    {
      /*y no volvemos a leer, si no, se cicla*/
      if(ec.value()==10054 or ec.value()==2)
        cout << usuario_->nombre() << " cerro sesion con ec " << ec.value() <<'\n';
      else
        cout << "lectura ec=" << ec.value() << ":" << ec.message() << endl;
      /*ec.value()==2 (End of file) t�picamente significa que el otro lado cerr� la conexi�n*/
    }
  });
}

/**Para todos los suscritos a mi servicio, les re-transmito el mensaje*/
void sesion::re_routear()
{
cout << nombre_servicio_ << " re-routeando " << data_ << '\n';
try{
  vector<weak_ptr<sesion>> mis_suscritos = nube::suscritos.at(nombre_servicio_);
  for(weak_ptr<sesion> wp : mis_suscritos)
  {
    shared_ptr<sesion> sp = wp.lock();
    sp->hacer_escritura(data_);
  }
}
catch(out_of_range& e) {}
  memset(data_, '\0', longitud_maxima);
  hacer_lectura(); //siempre volvemos a "escuchar"
}

void sesion::hacer_escritura(std::string str)
{
  str_ = str;
  auto si_mismo(shared_from_this());
  asio::async_write(socket_, asio::buffer(str_.data(), str_.size()),
    [this, si_mismo](std::error_code ec, std::size_t /*length*/)
  {
    if (!ec)
    {
      /*procesar escritura exitosa*/
    }
    else
    {
      cout << "Error escritura:" << ec.value() << ":" << ec.message() << '\n';
    }
  });
}

void sesion::hacer_escritura_terminante(std::string str)
{
  /*Hacemos que la variable str_ de la instancia de sesion contenga una copia de los datos a recibir.
    Esto es acertado por que un lambda no debe contener referencias a datos temporales*/
  str_ = str;
  auto si_mismo(shared_from_this());
  asio::async_write(socket_, asio::buffer(str_.data(), str_.size()),
    [this, si_mismo](std::error_code ec, std::size_t /*length*/)
  {
    if (!ec)
    {
      cout << "Escritura terminante, cerrando socket "  << socket_.remote_endpoint().address().to_string()
           << ":" << socket_.remote_endpoint().port() <<'\n';
      socket_.close();
    }
    else
    {
      cout << "Error escritura:" << ec.value() << ":" << ec.message() << '\n';
    }
  });
}

void sesion::enviar_archivo(string archivo)
{
  string buf;
  std::ifstream ifs(archivo/*, std::ios::binary*/);
  if(!ifs.is_open())
  {
    cout << "longitud de string: " << archivo.size() << '\t';
    for(auto c : archivo)
      cout << c << ':' << (int)c << '\n';
    cout << "Error abriendo archivo " << archivo << strerror(errno) << '\n';
    return;
  }

  ifs.seekg(0, std::ios_base::end);
  buf.resize(ifs.tellg()); /*Reservamos la memoria*/
  ifs.seekg(0); //regresamos el iterador de ifs a su inicio
  ifs.read(&buf[0], buf.size()); /*Leemos el archivo de golpe*/
  hacer_escritura_terminante(buf);
}

//ssssssssssssssssssssssssssssssssssssssssssssssssssss

/**Acepta conexiones entrantes y las mueve a un objeto sesi�n. Se llama a s� misma al terminar*/
void servidor::hacer_aceptacion()
{
  acceptor_.async_accept(socket_,
    [this](std::error_code ec)
  {
    if (!ec)
    {
      auto sp = std::make_shared<sesion>(std::move(socket_));
      sp->iniciar();
      //causa error cout << "conexion establecida por " << socket_.remote_endpoint() << endl;
    }
    else
    {
      cout << "Error aceptacion:" << ec.value() << ":" << ec.message() << '\n';
    }

    hacer_aceptacion();
  });
}
