/*************************************************************************************************
 * Implementation of Depot for C++
 *                                                      Copyright (C) 2000-2005 Mikio Hirabayashi
 * This file is part of QDBM, Quick Database Manager.
 * QDBM is free software; you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation; either version
 * 2.1 of the License or any later version.  QDBM is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * You should have received a copy of the GNU Lesser General Public License along with QDBM; if
 * not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 *************************************************************************************************/


#include "xdepot.h"
#include <new>
#include <cstdlib>
#include <ctime>

extern "C" {
#include <depot.h>
#include <pthread.h>
}

using namespace qdbm;



/*************************************************************************************************
 * Depot_error
 *************************************************************************************************/


Depot_error::Depot_error() throw()
  : DBM_error(){
  ecode = DP_EMISC;
  return;
}


Depot_error::Depot_error(int ecode) throw()
  : DBM_error(){
  this->ecode = ecode;
  return;
}


Depot_error::Depot_error(const Depot_error& de) throw()
  : DBM_error(de){
  ecode = de.ecode;
  return;
}


Depot_error::~Depot_error() throw(){
  return;
}


Depot_error& Depot_error::operator =(const Depot_error& de) throw(){
  this->ecode = de.ecode;
  return *this;
}


Depot_error& Depot_error::operator =(int ecode) throw(){
  this->ecode = ecode;
  return *this;
}


bool Depot_error::operator ==(const Depot_error& de) const throw(){
  return ecode == de.ecode;
}


bool Depot_error::operator !=(const Depot_error& de) const throw(){
  return ecode != de.ecode;
}


bool Depot_error::operator ==(int ecode) const throw(){
  return this->ecode == ecode;
}


bool Depot_error::operator !=(int ecode) const throw(){
  return this->ecode != ecode;
}


Depot_error::operator const char*() const throw(){
  return dperrmsg(ecode);
}


int Depot_error::code() const throw(){
  return ecode;
}


const char* Depot_error::message() const throw(){
  return dperrmsg(ecode);
}



/*************************************************************************************************
 * Depot
 *************************************************************************************************/


const int Depot::ENOERR = DP_ENOERR;
const int Depot::EFATAL = DP_EFATAL;
const int Depot::EMODE = DP_EMODE;
const int Depot::EBROKEN = DP_EBROKEN;
const int Depot::EKEEP = DP_EKEEP;
const int Depot::ENOITEM = DP_ENOITEM;
const int Depot::EALLOC = DP_EALLOC;
const int Depot::EMAP = DP_EMAP;
const int Depot::EOPEN = DP_EOPEN;
const int Depot::ECLOSE = DP_ECLOSE;
const int Depot::ETRUNC = DP_ETRUNC;
const int Depot::ESYNC = DP_ESYNC;
const int Depot::ESTAT = DP_ESTAT;
const int Depot::ESEEK = DP_ESEEK;
const int Depot::EREAD = DP_EREAD;
const int Depot::EWRITE = DP_EWRITE;
const int Depot::ELOCK = DP_ELOCK;
const int Depot::EUNLINK = DP_EUNLINK;
const int Depot::EMKDIR = DP_EMKDIR;
const int Depot::ERMDIR = DP_ERMDIR;
const int Depot::EMISC = DP_EMISC;
const int Depot::OREADER = DP_OREADER;
const int Depot::OWRITER = DP_OWRITER;
const int Depot::OCREAT = DP_OCREAT;
const int Depot::OTRUNC = DP_OTRUNC;
const int Depot::ONOLCK = DP_ONOLCK;
const int Depot::OLCKNB = DP_OLCKNB;
const int Depot::OSPARSE = DP_OSPARSE;
const int Depot::DOVER = DP_DOVER;
const int Depot::DKEEP = DP_DKEEP;
const int Depot::DCAT = DP_DCAT;


const char* Depot::version() throw(){
  return dpversion;
}


void Depot::remove(const char* name) throw(Depot_error){
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpremove(name)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


Depot::Depot(const char* name, int omode, int bnum) throw(Depot_error)
  : ADBM(){
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!(depot = dpopen(name, omode, bnum))){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


Depot::~Depot() throw(){
  if(!depot) return;
  pthread_mutex_lock(&mutex);
  dpclose(depot);
  depot = 0;
  pthread_mutex_unlock(&mutex);
}


void Depot::close() throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpclose(depot)){
    depot = 0;
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  depot = 0;
  pthread_mutex_unlock(&mutex);
}


void Depot::put(const char* kbuf, int ksiz, const char* vbuf, int vsiz, int dmode)
  throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpput(depot, kbuf, ksiz, vbuf, vsiz, dmode)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


void Depot::out(const char* kbuf, int ksiz) throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpout(depot, kbuf, ksiz)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


char* Depot::get(const char* kbuf, int ksiz, int start, int max, int* sp) throw(Depot_error){
  char* vbuf;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!(vbuf = dpget(depot, kbuf, ksiz, start, max, sp))){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return vbuf;
}


int Depot::getwb(const char *kbuf, int ksiz, int start, int max, char *vbuf) throw(Depot_error){
  int vsiz;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if((vsiz = dpgetwb(depot, kbuf, ksiz, start, max, vbuf)) == -1){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return vsiz;
}


int Depot::vsiz(const char* kbuf, int ksiz) throw(Depot_error){
  int vsiz;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if((vsiz = dpvsiz(depot, kbuf, ksiz)) == -1){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return vsiz;
}


void Depot::iterinit() throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpiterinit(depot)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


char* Depot::iternext(int* sp) throw(Depot_error){
  char* vbuf;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!(vbuf = dpiternext(depot, sp))){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return vbuf;
}


void Depot::setalign(int align) throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpsetalign(depot, align)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


void Depot::setfbpsiz(int size) throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpsetfbpsiz(depot, size)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


void Depot::sync() throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpsync(depot)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


void Depot::optimize(int bnum) throw(Depot_error){
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!dpoptimize(depot, bnum)){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
}


char* Depot::name() throw(Depot_error){
  char* buf;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if(!(buf = dpname(depot))){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return buf;
}


int Depot::fsiz() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if((rv = dpfsiz(depot)) == -1){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return rv;
}


int Depot::bnum() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if((rv = dpbnum(depot)) == -1){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return rv;
}


int Depot::busenum() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if((rv = dpbusenum(depot)) == -1){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return rv;
}


int Depot::rnum() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  if((rv = dprnum(depot)) == -1){
    pthread_mutex_unlock(&mutex);
    throw Depot_error(dpecode);
  }
  pthread_mutex_unlock(&mutex);
  return rv;
}


bool Depot::writable() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  rv = dpwritable(depot);
  pthread_mutex_unlock(&mutex);
  return rv ? true : false;
}


bool Depot::fatalerror() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  rv = dpfatalerror(depot);
  pthread_mutex_unlock(&mutex);
  return rv ? true : false;
}


int Depot::inode() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  rv = dpinode(depot);
  pthread_mutex_unlock(&mutex);
  return rv;
}


time_t Depot::mtime() throw(Depot_error){
  time_t rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  rv = dpmtime(depot);
  pthread_mutex_unlock(&mutex);
  return rv;
}


int Depot::fdesc() throw(Depot_error){
  int rv;
  if(!depot) throw Depot_error();
  if(pthread_mutex_lock(&mutex) != 0) throw Depot_error();
  rv = dpfdesc(depot);
  pthread_mutex_unlock(&mutex);
  return rv;
}


void Depot::storerec(const Datum& key, const Datum& val, bool replace) throw(Depot_error){
  put(key.ptr(), key.size(), val.ptr(), val.size(), replace ? DP_DOVER : DP_DKEEP);
}


void Depot::deleterec(const Datum& key) throw(Depot_error){
  out(key.ptr(), key.size());
}


Datum Depot::fetchrec(const Datum& key) throw(Depot_error){
  char* vbuf;
  int vsiz;
  vbuf = get(key.ptr(), key.size(), 0, -1, &vsiz);
  return Datum(vbuf, vsiz, true);
}


Datum Depot::firstkey() throw(Depot_error){
  iterinit();
  return nextkey();
}


Datum Depot::nextkey() throw(Depot_error){
  char* kbuf;
  int ksiz;
  kbuf = iternext(&ksiz);
  return Datum(kbuf, ksiz, true);
}


bool Depot::error() throw(Depot_error){
  return fatalerror();
}


Depot::Depot(const Depot& depot) throw(Depot_error){
  throw Depot_error();
}


Depot& Depot::operator =(const Depot& depot) throw(Depot_error){
  throw Depot_error();
}



/* END OF FILE */
