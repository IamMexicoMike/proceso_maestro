#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <map>

//extern std::map<int, ditto> mapa;

struct punto
{
  int x, y;
};

class ditto
{
public:
  ditto(punto p1, punto p2) :
    p1_(p1),
    p2_(p2)
    {}

private:
  punto p1_;
  punto p2_;
};

/**Necesitas definir el conjunto de variables que definen el estado del sistema.
Se debe definir din�micamente, pues partimos de la suposici�n que es muy costoso que
un solo usuario conozca un sistema demasiado complejo

Un sistema puede definir nuevos procesos, crear cuentas, borrar cuentas, a�adir m�quinas, a�adir tipos nuevos, etc*/

/**Clase base para transacciones
Una transacci�n es cualquier acci�n que cambia el estado del sistema.
Suena intuitivo que sea subclaseable*/
class trsc
{
  virtual void deshacer();

  template<typename T>
  void transferir(int id1, int id2, T obj); //transacci�n de subobjeto id1 a subobjeto id2, de un objeto T
};

class diag_trsc
{

};


#endif // ESTRUCTURAS_H
