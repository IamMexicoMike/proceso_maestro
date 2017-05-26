#include "nube.h"

using namespace std;

/*e.g. sesion0 provee el servicio bascula1*/
map<string, weak_ptr<sesion> > nube::servicios;

/*e.g. los suscritos a bascula1 son sesion1, sesion2, sesion3*/
map<string, set<weak_ptr<sesion> > > nube::suscritos;
