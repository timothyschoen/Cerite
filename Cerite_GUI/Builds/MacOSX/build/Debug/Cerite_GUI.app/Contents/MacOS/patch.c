#include "libcerite.h" 



typedef struct {
double dt;
void(*send[2])(void* , Data);
void* objects[2];
} data_obj;
data_obj data;

void data_reset() {
data.dt = 0.000022;
data.objects[0] = 0;
data.objects[1] = 0;
}



typedef struct {
int argc;
double ms;
double time;
double delta;
int enabled;
Data argv[0];
int data_nodes[3];
} metro_obj;
metro_obj metro_obj_1;

void metro_obj_1_reset() {
metro_obj_1.argc = 200;
metro_obj_1.ms = 1000;
metro_obj_1.time = 0;
metro_obj_1.delta = 0;
metro_obj_1.enabled = 1;
metro_obj_1.data_nodes[0] = 0;
metro_obj_1.data_nodes[1] = 0;
metro_obj_1.data_nodes[2] = 1;
}

void metro_obj_set(metro_obj* obj, Data f_arg0){

  if(isNumber()) {
    obj->delta = getNumber() / 1000.;
  }
  else {
    print("wrong type!");
  }
}

void metro_obj_enable(metro_obj* obj, Data f_arg0){

  if(isNumber()) {
    obj->time = obj->delta;
    obj->enabled = getNumber();
  }
  if(isBang()) {
    obj->time = obj->delta;
    obj->enabled = 1;
  }

}

void metro_obj_data_prepare(metro_obj* obj){
  
  obj->delta = obj->ms / 1000.;
  registerData(0, metro_obj_enable);
  registerData(1, metro_obj_set);

}

void metro_obj_data_calc(metro_obj* obj){

  if(obj->enabled && obj->time >= obj->delta) {
    obj->time -= obj->delta;
    callData(2, Bang());
  }

  obj->time += data.dt;

}



typedef struct {
int argc;
Data argv[0];
int data_nodes[1];
} print_obj;
print_obj print_obj_1;

void print_obj_1_reset() {
print_obj_1.argc = 0;
print_obj_1.data_nodes[0] = 1;
}

void print_obj_onbang(print_obj* obj, Data f_arg0){

  if(isString()) {
    print(getString());
  }
  else if(isNumber()) {
    char numbuffer[50];
    snprintf(numbuffer, 50, "%f", getNumber());
    print(numbuffer);
  }
  else if(isBang()) {
    print("bang!\n");
  }




}

void print_obj_data_prepare(print_obj* obj){
  
  registerData(0, print_obj_onbang);

}

void data_obj_prepare(){
metro_obj_data_prepare(&metro_obj_1);
print_obj_data_prepare(&print_obj_1);

}

void data_obj_calc(){
metro_obj_data_calc(&metro_obj_1);

}

void reset() {
metro_obj_1_reset();
print_obj_1_reset();
data_reset();
}

void calc() {
data_obj_calc();
}

void prepare() {
data_obj_prepare();
}

